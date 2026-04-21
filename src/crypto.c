/**
 * crypto.c - Cryptocurrency price display implementation.
 *
 * This file handles:
 *   - Requesting cryptocurrency prices from the companion app via AppMessage
 *   - Updating the display with received prices (up to 4 crypto pairs)
 *   - Storing prices in persistent memory for restoration after reload
 *   - Managing the update interval (default: 15 minutes)
 *   - Retrying failed message sends with a 2-second delay
 *
 * Note: Cryptocurrency data is only available on non-APlite devices
 * (Pebble Time, Time 2, Time Round, Time Steel).
 */

#include <pebble.h>
#include "configs.h"
#include "crypto.h"
#include "keys.h"
#include "text.h"

#if !defined PBL_PLATFORM_APLITE

// =============================================================================
// Internal State
// =============================================================================

static bool crypto_enabled;  // Whether crypto tracking is enabled
static int last_update = 0;  // Unix timestamp of last crypto update
static int crypto_interval = 15; // Update interval in minutes
static char price[8];        // Current crypto price (primary)
static char price_b[8];      // Second crypto price
static char price_c[8];      // Third crypto price
static char price_d[8];      // Fourth crypto price
static AppTimer *retry_timer; // Timer for retrying failed message sends

// =============================================================================
// Retry Logic
// If app_message_outbox_send fails, retry after 2 seconds
// =============================================================================

static void retry_handler(void *context) {
    update_crypto(true);
}

// =============================================================================
// Crypto Price Fetching
// =============================================================================

void update_crypto(bool force) {
    int current_time = (int)time(NULL);

    // Only fetch if forced, or if enough time has passed since last update
    if (force || last_update == 0 || (current_time - last_update) >= crypto_interval * 60) {
        DictionaryIterator *iter;
        AppMessageResult result = app_message_outbox_begin(&iter);
        if (result == APP_MSG_OK) {
            // Request crypto data from the companion app
            dict_write_uint8(iter, KEY_REQUESTCRYPTO, 1);
            result = app_message_outbox_send();

            if (result == APP_MSG_OK) {
                // Show "loading" indicator while waiting for data
                if (force) {
                    set_crypto_layer_text("loading");
                    set_crypto_b_layer_text("loading");
                    set_crypto_c_layer_text("loading");
                    set_crypto_d_layer_text("loading");
                }
                last_update = current_time;
            }
        } else if (force) {
            // If sending failed, schedule a retry in 2 seconds
            retry_timer = app_timer_register(2000, retry_handler, NULL);
        }
    }
}

static bool get_crypto_enabled(void) {
    // Crypto is enabled if any of the 4 crypto slots has a module assigned
    return is_module_enabled(MODULE_CRYPTO) ||
        is_module_enabled(MODULE_CRYPTOB) ||
        is_module_enabled(MODULE_CRYPTOC) ||
        is_module_enabled(MODULE_CRYPTOD);
}

// =============================================================================
// Price Display Updates
// Each function updates the display for one crypto slot.
// If the slot is not enabled, it clears the display.
// =============================================================================

void update_crypto_price(char *price) {
    if (is_module_enabled(MODULE_CRYPTO)) {
        set_crypto_layer_text(price);
    } else {
        set_crypto_layer_text("");
    }
}

void update_crypto_price_b(char *price) {
    if (is_module_enabled(MODULE_CRYPTOB)) {
        set_crypto_b_layer_text(price);
    } else {
        set_crypto_b_layer_text("");
    }
}

void update_crypto_price_c(char *price) {
    if (is_module_enabled(MODULE_CRYPTOC)) {
        set_crypto_c_layer_text(price);
    } else {
        set_crypto_c_layer_text("");
    }
}

void update_crypto_price_d(char *price) {
    if (is_module_enabled(MODULE_CRYPTOD)) {
        set_crypto_d_layer_text(price);
    } else {
        set_crypto_d_layer_text("");
    }
}

// =============================================================================
// Persistent Storage
// =============================================================================

static void update_crypto_from_storage(void) {
    // Restore crypto prices from persistent storage after reload
    if (persist_exists(KEY_CRYPTOPRICE)) {
        persist_read_string(KEY_CRYPTOPRICE, price, sizeof(price));
        update_crypto_price(price);
    }
    if (persist_exists(KEY_CRYPTOPRICEB)) {
        persist_read_string(KEY_CRYPTOPRICEB, price_b, sizeof(price_b));
        update_crypto_price_b(price_b);
    }
    if (persist_exists(KEY_CRYPTOPRICEC)) {
        persist_read_string(KEY_CRYPTOPRICEC, price_c, sizeof(price_c));
        update_crypto_price_c(price_c);
    }
    if (persist_exists(KEY_CRYPTOPRICED)) {
        persist_read_string(KEY_CRYPTOPRICED, price_d, sizeof(price_d));
        update_crypto_price_d(price_d);
    }
}

void store_crypto_price(char *price) {
    persist_write_string(KEY_CRYPTOPRICE, price);
}

void store_crypto_price_b(char *price) {
    persist_write_string(KEY_CRYPTOPRICEB, price);
}

void store_crypto_price_c(char *price) {
    persist_write_string(KEY_CRYPTOPRICEC, price);
}

void store_crypto_price_d(char *price) {
    persist_write_string(KEY_CRYPTOPRICED, price);
}

// =============================================================================
// Enable/Disable Crypto Tracking
// =============================================================================

void toggle_crypto(uint8_t reload_origin) {
    crypto_enabled = get_crypto_enabled();

    // Load update interval from storage on config reload
    if (reload_origin == RELOAD_CONFIGS || reload_origin == RELOAD_DEFAULT) {
        crypto_interval = persist_exists(KEY_CRYPTOTIME) ?
            persist_read_int(KEY_CRYPTOTIME) : 15;
    }

    if (crypto_enabled) {
        update_crypto_from_storage(); // Restore from storage
        if (reload_origin == RELOAD_MODULE || reload_origin == RELOAD_CONFIGS) {
            update_crypto(true); // Force immediate update
        }
    } else {
        // Clear all crypto displays
        set_crypto_layer_text("");
        set_crypto_b_layer_text("");
        set_crypto_c_layer_text("");
        set_crypto_d_layer_text("");
    }
}

bool is_crypto_enabled(void) {
    return crypto_enabled;
}

#endif // !PBL_PLATFORM_APLITE
