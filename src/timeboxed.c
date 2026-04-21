/**
 * timeboxed.c - Main entry point for the Timeboxed watchface application.
 *
 * This file orchestrates the entire watchface:
 *   - App lifecycle (init/deinit)
 *   - Window setup and teardown
 *   - AppMessage communication with the companion phone app
 *   - Periodic tick handler (updates time, triggers data fetches)
 *   - Battery and Bluetooth service callbacks
 *   - Loading user configuration from persistent storage
 *
 * Message flow:
 *   Watch -> Companion: Request weather, crypto, timezone data
 *   Companion -> Watch: Send weather data, crypto prices, timezone offsets
 */

#include <pebble.h>
#include "keys.h"
#include "configs.h"
#include "text.h"
#include "weather.h"
#include "health.h"
#include "accel.h"
#include "crypto.h"
#include "clock.h"
#include "screen.h"

// =============================================================================
// Persistent Storage Keys for App State
// Track which modules were enabled so we can restore state after reload
// =============================================================================

#define KEY_ENABLED        0x10  // Bitmask of enabled modules (stored persistently)
#define KEY_CONFIG_LOADED  0x11  // Flag indicating configs were loaded from storage

// =============================================================================
// App State
// =============================================================================

static struct Window *s_window;
static struct s_AppContext {
    bool configs_loaded;       // Whether we've loaded user configs at least once
    bool bluetooth_connected;  // Current Bluetooth connection state
    uint32_t enabled_modules;  // Bitmask of currently enabled modules
    uint8_t reload_origin;     // Which event triggered the current reload
} s_app_context;

// =============================================================================
// AppMessage Callbacks
// Handle messages received from the companion phone app.
// The companion app fetches data (weather, crypto) and sends it to the watch.
// =============================================================================

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
    // ---- Weather Data ----
    // Companion sends temperature (int) and weather condition code (int)
    Tuple *t_weather = dict_find(iter, KEY_TEMP);
    Tuple *t_weather_code = dict_find(iter, KEY_WEATHER);
    if (t_weather && t_weather_code) {
        update_weather_values(tuple_int32(t_weather), tuple_int32(t_weather_code));
    }

    // ---- Forecast Data (max/min temp) ----
    Tuple *t_max = dict_find(iter, KEY_MAX);
    Tuple *t_min = dict_find(iter, KEY_MIN);
    if (t_max && t_min) {
        update_forecast_values(tuple_int32(t_max), tuple_int32(t_min));
    }

    // ---- Wind Data (speed in mph, direction in degrees) ----
    Tuple *t_speed = dict_find(iter, KEY_SPEED);
    Tuple *t_direction = dict_find(iter, KEY_DIRECTION);
    if (t_speed && t_direction) {
        update_wind_values(tuple_int32(t_speed), tuple_int32(t_direction));
    }

    // ---- Sunrise/Sunset (Unix timestamps) ----
    Tuple *t_sunrise = dict_find(iter, KEY_SUNRISE);
    if (t_sunrise) {
        update_sunrise(tuple_int32(t_sunrise));
    }
    Tuple *t_sunset = dict_find(iter, KEY_SUNSET);
    if (t_sunset) {
        update_sunset(tuple_int32(t_sunset));
    }

    // ---- Cryptocurrency Prices ----
    // Individual price strings for up to 4 crypto slots
    Tuple *t_cryptoprice = dict_find(iter, KEY_CRYPTOPRICE);
    if (t_cryptoprice) {
        const char *price = tuple_cstring(t_cryptoprice);
        if (price) {
            update_crypto_price(price);
            store_crypto_price(price);
        }
    }
    Tuple *t_cryptopriceb = dict_find(iter, KEY_CRYPTOPRICEB);
    if (t_cryptopriceb) {
        const char *price = tuple_cstring(t_cryptopriceb);
        if (price) {
            update_crypto_price_b(price);
            store_crypto_price_b(price);
        }
    }
    Tuple *t_cryptopricec = dict_find(iter, KEY_CRYPTOPRICEC);
    if (t_cryptopricec) {
        const char *price = tuple_cstring(t_cryptopricec);
        if (price) {
            update_crypto_price_c(price);
            store_crypto_price_c(price);
        }
    }
    Tuple *t_cryptopriced = dict_find(iter, KEY_CRYPTOPRICED);
    if (t_cryptopriced) {
        const char *price = tuple_cstring(t_cryptopriced);
        if (price) {
            update_crypto_price_d(price);
            store_crypto_price_d(price);
        }
    }

    // ---- Timezone Data ----
    // Companion sends timezone names, minute offsets, and timezone codes
    Tuple *t_timezones = dict_find(iter, KEY_TIMEZONES);
    Tuple *t_minutes = dict_find(iter, KEY_TIMEZONESMINUTES);
    Tuple *t_codes = dict_find(iter, KEY_TIMEZONESCODE);
    if (t_timezones && t_minutes && t_codes) {
        set_timezone(
            tuple_cstring(t_timezones),
            tuple_cstring(t_minutes),
            tuple_cstring(t_codes)
        );
    }

    // ---- Update Notification ----
    // Companion found a watchface update available
    Tuple *t_hasupdate = dict_find(iter, KEY_HASUPDATE);
    if (t_hasupdate) {
        notify_update(tuple_uint8(t_hasupdate));
    }

    // ---- Crypto Update Interval ----
    // User's preferred crypto refresh interval (in minutes)
    Tuple *t_cryptotime = dict_find(iter, KEY_CRYPTOTIME);
    if (t_cryptotime) {
        crypto_interval = tuple_uint8(t_cryptotime);
        persist_write_int(KEY_CRYPTOTIME, crypto_interval);
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    // Inbox message was dropped (usually due to full inbox).
    // Log the reason for debugging.
    LOG(APP_LOG_LEVEL_DEBUG, "Inbox dropped, reason=%d", reason);
}

static void outbox_failed_callback(DictIterator *iterator, AppMessageResult reason, void *context) {
    // Failed to send message to companion app.
    // Schedule a retry after 2 seconds.
    LOG(APP_LOG_LEVEL_DEBUG, "Outbox failed, reason=%d", reason);
    app_timer_register(2000, retry_handler, NULL);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    // Successfully sent message to companion app.
    // The companion will process the request and send data back via inbox_received_callback.
    LOG(APP_LOG_LEVEL_DEBUG, "Outbox sent");
}

// =============================================================================
// Tick Handler
// Called periodically (based on tick_timer_service_subscribe interval).
// Updates the time display and triggers data fetches as needed.
// =============================================================================

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    // Only update display elements when the minute changes.
    // This avoids unnecessary processing on every second tick.
    if (!clock_is_twenty_four_format()) {
        // 12-hour format: update when hour or minute changes
        if (units_changed & (hour | minute)) {
            update_time();
            set_hours_layer_text(""); // Placeholder for hours display
            return;
        }
    } else {
        // 24-hour format: update when hour or minute changes
        if (units_changed & (hour | minute)) {
            update_time();
            set_hours_layer_text("");
            return;
        }
    }

    // Every minute: update seconds display
    set_seconds_layer_text("");

    // Periodically request fresh data from companion app.
    // Weather: request every 10 minutes (companion handles rate limiting)
    // Crypto: request based on configured interval (default 15 min)
    // Health: request on first tick of each hour
    static int last_weather_request = 0;
    static int last_health_request = 0;
    int current_time = (int)time(NULL);

    // Request weather data periodically
    if (is_weather_enabled() && (current_time - last_weather_request) >= 600) {
        last_weather_request = current_time;
        update_weather(false); // Don't force, let companion decide
    }

    // Request crypto prices based on configured interval
    if (is_crypto_enabled()) {
        update_crypto(false);
    }

    // Request health data on the minute (companion caches and throttles)
    if (is_health_toggle_enabled()) {
        queue_health_update();
    }
}

// =============================================================================
// Window Handlers
// Manage screen lifecycle events.
// =============================================================================

static void window_load(struct Window *window) {
    // Called when the window is first created and its root layer is loaded.
    // Set up the window background and create all UI layers.
    layer_set_background_color(window_get_root_layer(window), GColorClear);

    // Load the screen with all modules configured by the user
    load_screen(RELOAD_DEFAULT, window);
}

static void window_unload(struct Window *window) {
    // Called when the window is about to be destroyed.
    // Clean up all text layers and resources.
    destroy_text_layers();
}

static void window_screen_util_unobstructed_size_changed_callback(
    struct Window *window,
    struct GSize unobstructed_size
) {
    // Called when the unobstructed area changes (e.g., keyboard appears).
    // Recreate text layers with new positions to avoid overlap.
    recreate_text_layers(window);
}

// =============================================================================
// Battery Service Handler
// Updates battery display when charge level changes.
// =============================================================================

static void battery_state_handler(struct BatteryChargeState state) {
    battery_handler(state);
}

// =============================================================================
// Bluetooth Service Handler
// Updates Bluetooth icon when connection state changes.
// =============================================================================

static void bluetooth_connection_handler(bool connected) {
    s_app_context.bluetooth_connected = connected;
    bt_handler(connected);
}

// =============================================================================
// Configuration Loading
// Load user's saved preferences from persistent storage.
// Called once during init and whenever configs change.
// =============================================================================

static void load_configs(void) {
    // Load all user toggles (weather, crypto, health, etc.)
    // Load module assignments for each display state (normal, sleep, tap, wrist)
    // Load unit preferences (C/F, mph/kph, 12/24h)
    load_all_toggles();

    s_app_context.configs_loaded = true;
}

// =============================================================================
// App Init
// Entry point: called when the watchface is launched.
// Sets up all services, loads config, and starts the app.
// =============================================================================

int main(void) {
    // Initialize app context to zero/default values
    memset(&s_app_context, 0, sizeof(s_app_context));

    // Create the main window (fullscreen, no status bar for watchface)
    s_window = window_create(NULL);
    window_set_fullscreen(s_window, true);

    // Set up window lifecycle callbacks
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
        .screen_util_unobstructed_size_changed = window_screen_util_unobstructed_size_changed_callback,
    });

    // Load user configuration from persistent storage
    load_configs();

    // ---- Set up AppMessage ----
    // Configure send/receive buffers and register message callbacks.
    // AppMessage allows communication between the watchface and the companion iOS/Android app.
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    // Use maximum supported message sizes (companion apps can send large weather data)
    app_message_open(
        app_message_inbox_size_maximum(),  // Receive buffer: max size
        app_message_outbox_size_maximum()  // Send buffer: max size
    );

    // ---- Set up Battery Service ----
    // Subscribe to battery state changes for battery level display.
    battery_service_subscribe(battery_state_handler);

    // ---- Set up Bluetooth Service ----
    // Subscribe to connection state changes for Bluetooth icon display.
    bluetooth_connection_subscribe(bluetooth_connection_handler);
    bluetooth_connection_handler(bluetooth_service_get_last_connection_state().connected);

    // ---- Set up Tick Timer ----
    // Subscribe to minute-level tick events for time display updates.
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    // ---- Set up Accelerometer Service ----
    // Initialize tap-to-show and wrist-to-show gesture detection.
    init_accel_service(s_window);

    // ---- Show the window ----
    window_stack_push(s_window, true, true);

    // ---- Initialize sleep tracking ----
    init_sleep_data();

    // ---- Toggle modules based on user config ----
    // Must be called before update_weather/update_crypto to enable the modules
    toggle_weather(RELOAD_DEFAULT);
    toggle_crypto(RELOAD_DEFAULT);

    // ---- Initial data fetch ----
    // Request weather and crypto data immediately on launch
    update_weather(false);
    update_crypto(false);

    // Run the app event loop (blocks until app exits)
    app_event_loop();

    // ---- Cleanup on exit ----
    // Unsubscribe from all services
    tick_timer_service_unsubscribe();
    battery_service_unsubscribe();
    bluetooth_connection_unsubscribe();
    accel_data_service_unsubscribe();
    accel_tap_service_unsubscribe();

    // Save any remaining health data
    save_health_data_to_storage();

    // Destroy the window (calls window_unload, which destroys text layers)
    window_destroy(s_window);

    // Health data cleanup is handled by save_health_data_to_storage
}
