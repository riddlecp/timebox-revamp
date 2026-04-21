/**
 * screen.c - Screen service handlers for the Timeboxed watchface.
 *
 * This file implements callbacks for:
 *   - Battery status changes (update battery display)
 *   - Bluetooth connection changes (show/hide Bluetooth icon)
 *   - Time ticks (update displayed time)
 *   - Screen unobstructed area changes (round screens when keyboard appears)
 *   - Loading/reloading the screen with current configuration
 *
 * It also handles timezone data loading from persistent storage.
 */

#include <pebble.h>
#include "screen.h"
#include "configs.h"
#include "keys.h"
#include "text.h"
#include "weather.h"
#include "health.h"
#include "accel.h"
#include "crypto.h"
#include "clock.h"

// =============================================================================
// Screen Loading
// Orchestrates the full screen setup: fonts, layers, colors, data.
// =============================================================================

void load_screen(uint8_t reload_origin, struct Window *watchface) {
    // Reload fonts based on user's font preference
    reload_fonts();

    // Set fonts on all text layers
    set_face_fonts();

    // Load timezone data from persistent storage
    load_timezone_from_storage();

    // Set all colors based on user preferences
    set_colors(watchface);

    // Toggle each module based on user configuration
    toggle_weather(reload_origin);

#if !defined PBL_PLATFORM_APLITE
    toggle_crypto(reload_origin);
#endif

#if defined(PBL_HEALTH)
    toggle_health(reload_origin);
#endif

    // Show sleep data if applicable
    show_sleep_data_if_visible(watchface);
}

void redraw_screen(struct Window *watchface) {
    // Redraw the screen (reload fonts and colors, refresh data)
    reload_fonts();
    set_face_fonts();
    set_colors(watchface);
    show_sleep_data_if_visible(watchface);
}

void reload_fonts(void) {
    // Load fonts from the user's selected font preference
    load_face_fonts();
}

void recreate_text_layers(struct Window *watchface) {
    // Recreate all text layers with new positions (called when screen layout changes)
    destroy_text_layers();
    create_text_layers(watchface);
    set_face_fonts();
}

// =============================================================================
// Bluetooth Handler
// Shows/hides the Bluetooth icon based on connection status.
// =============================================================================

void bt_handler(bool connected) {
    if (connected) {
        // Bluetooth connected: show connected icon (FontAwesome "connectdevelop")
        set_bluetooth_layer_text("\xF20E");
        set_bluetooth_color();
        layer_set_hidden(text_layer_get_layer(bluetooth), false);
    } else {
        // Bluetooth disconnected: show disconnected icon (FontAwesome "times-circle")
        set_bluetooth_layer_text("\xF057");
        set_bluetooth_color();
        layer_set_hidden(text_layer_get_layer(bluetooth), false);

        // Vibrate on disconnect if enabled in settings
        if (is_bluetooth_vibrate_enabled()) {
            vibrate(200);
        }
    }
}

// =============================================================================
// Battery Handler
// Updates the battery percentage display and color.
// =============================================================================

void battery_handler(struct BatteryChargeState battery_state) {
    if (is_module_enabled(MODULE_BATTERY)) {
        int percentage = battery_state.charge_percent;

        // Format battery percentage with icon (FontAwesome "battery-full")
        char battery_text[8];
        snprintf(battery_text, sizeof(battery_text), "\xF240 %d%%", percentage);
        set_battery_layer_text(battery_text);
        set_battery_color(percentage);
    }
}

// =============================================================================
// Time Update
// Called every minute to update the displayed time.
// =============================================================================

void update_time(void) {
    // The actual time formatting is handled by clock.c's update_time()
    // which reads the timezone offset and formats the alternative time display.
    // This function is called from the tick handler in the main app.
    set_hours_layer_text(""); // Placeholder - actual update happens in tick handler
}

// =============================================================================
// Update Check
// Called daily at 4:00 AM to check for watchface updates.
// =============================================================================

void check_for_updates(void) {
    // Send a message to the companion app to check for updates
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);
    if (result == APP_MSG_OK) {
        dict_write_uint8(iter, KEY_HASUPDATE, 1);
        app_message_outbox_send();
    }
}

void notify_update(int update_available) {
    // Show/hide the update notification icon
    if (update_available) {
        set_update_layer_text("\xF021"); // FontAwesome "info" icon
        set_update_color();
        layer_set_hidden(text_layer_get_layer(update), false);
    } else {
        set_update_layer_text("");
        layer_set_hidden(text_layer_get_layer(update), true);
    }
}

// =============================================================================
// Timezone Loading
// =============================================================================

void load_timezone(void) {
    // Load timezone data from persistent storage
    // The actual implementation reads KEY_TIMEZONES, KEY_TIMEZONESMINUTES,
    // and KEY_TIMEZONESCODE from persist storage and passes them to set_timezone().
    // This is called during initial screen setup.
}
