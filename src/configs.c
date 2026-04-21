/**
 * configs.c - Configuration management implementation.
 *
 * This file handles:
 *   - Loading user preferences from persistent storage (persist_read_int)
 *   - Managing which modules appear in which display slots
 *   - Supporting different slot configurations per display state
 *     (normal, sleep, tap-to-show, wrist-to-show)
 *   - Querying individual toggle states from a combined bitmask
 */

#include <pebble.h>
#include "configs.h"
#include "keys.h"
#include "text.h"
#include "screen.h"

// =============================================================================
// Internal State
// =============================================================================

// Whether the configuration bitmask has been loaded from storage
static bool configs_loaded = false;

// Bitmask of all enabled feature toggles (weather, health, etc.)
static int configs = 0;

// Module assignments for each display state.
// modules[state][slot] = which module to show in that slot for that state.
// For example: modules[SLOT_A][STATE_NORMAL] = MODULE_WEATHER
//             means "show weather in slot A during normal mode"
static uint8_t modules[6];                          // Normal state
#if !defined PBL_PLATFORM_APLITE
static uint8_t modules_sleep[6];  // Sleep state slot assignments
static uint8_t modules_tap[6];    // Tap-to-show state slot assignments
static uint8_t modules_wrist[6];  // Wrist-to-show state slot assignments
#endif

// Whether slots E and F (center slots on round screens) should be visible
static bool center_slots_enabled = true;

// =============================================================================
// Module Configuration
// =============================================================================

void set_module(int slot, int module, int state) {
    // Store which module goes in which slot for a given display state
    if (state == STATE_NORMAL) {
        modules[slot] = module;
    } else if (state == STATE_SLEEP) {
#if !defined PBL_PLATFORM_APLITE
        modules_sleep[slot] = module;
#endif
    } else if (state == STATE_TAP) {
#if !defined PBL_PLATFORM_APLITE
        modules_tap[slot] = module;
#endif
    } else if (state == STATE_WRIST) {
#if !defined PBL_PLATFORM_APLITE
        modules_wrist[slot] = module;
#endif
    }
}

int get_wind_speed_unit() {
    // Read stored wind speed unit preference, default to MPH
    return persist_exists(KEY_SPEEDUNIT) ? persist_read_int(KEY_SPEEDUNIT) : UNIT_MPH;
}

// Load all module assignments from persistent storage
static void load_modules(void) {
    // Read normal mode slot assignments
    modules[SLOT_A] = persist_read_int(KEY_SLOTA);
    modules[SLOT_B] = persist_read_int(KEY_SLOTB);
    modules[SLOT_C] = persist_read_int(KEY_SLOTC);
    modules[SLOT_D] = persist_read_int(KEY_SLOTD);
    modules[SLOT_E] = persist_read_int(KEY_SLOTE);
    modules[SLOT_F] = persist_read_int(KEY_SLOTF);

#if !defined PBL_PLATFORM_APLITE
    // Read sleep mode slot assignments
    modules_sleep[SLOT_A] = persist_read_int(KEY_SLEEPSLOTA);
    modules_sleep[SLOT_B] = persist_read_int(KEY_SLEEPSLOTB);
    modules_sleep[SLOT_C] = persist_read_int(KEY_SLEEPSLOTC);
    modules_sleep[SLOT_D] = persist_read_int(KEY_SLEEPSLOTD);
    modules_sleep[SLOT_E] = persist_read_int(KEY_SLEEPSLOTE);
    modules_sleep[SLOT_F] = persist_read_int(KEY_SLEEPSLOTF);

    // Read tap-to-show slot assignments
    modules_tap[SLOT_A] = persist_read_int(KEY_TAPSLOTA);
    modules_tap[SLOT_B] = persist_read_int(KEY_TAPSLOTB);
    modules_tap[SLOT_C] = persist_read_int(KEY_TAPSLOTC);
    modules_tap[SLOT_D] = persist_read_int(KEY_TAPSLOTD);
    modules_tap[SLOT_E] = persist_read_int(KEY_TAPSLOTE);
    modules_tap[SLOT_F] = persist_read_int(KEY_TAPSLOTF);

    // Read wrist-to-show slot assignments
    modules_wrist[SLOT_A] = persist_read_int(KEY_WRISTSLOTA);
    modules_wrist[SLOT_B] = persist_read_int(KEY_WRISTSLOTB);
    modules_wrist[SLOT_C] = persist_read_int(KEY_WRISTSLOTC);
    modules_wrist[SLOT_D] = persist_read_int(KEY_WRISTSLOTD);
    modules_wrist[SLOT_E] = persist_read_int(KEY_WRISTSLOTE);
    modules_wrist[SLOT_F] = persist_read_int(KEY_WRISTSLOTF);
#endif

    modules_loaded = true;
}

bool is_module_enabled(int module) {
    // Ensure module assignments are loaded before checking
    if (!modules_loaded) {
        load_modules();
    }

    // Determine which slot config to use based on current display state
    // The watchface can show different content depending on context:
    //   - Normal: regular time display
    //   - Sleep: show sleep data when user is sleeping
    //   - Tap: show extra info when user taps the watch
    //   - Wrist: show extra info when user raises wrist
    uint8_t *current_modules;
#if !defined PBL_PLATFORM_APLITE
    if (tap_mode_visible()) {
        current_modules = modules_tap;
    } else if (wrist_mode_visible()) {
        current_modules = modules_wrist;
    } else if (should_show_sleep_data()) {
        current_modules = modules_sleep;
    } else {
        current_modules = modules;
    }
#else
    current_modules = modules;
#endif

    // Search all 6 slots for the requested module
    for (unsigned int i = 0; i < 6; ++i) {
        if (current_modules[i] == module) {
            // Slots A-D are always visible.
            // Slots E-F are only visible on round screens AND if center slots are enabled.
            return i <= 3 || (i > 3 && center_slots_enabled);
        }
    }
    return false;
}

bool is_module_enabled_any(int module) {
    // Check if a module is enabled in ANY state (normal, sleep, tap, wrist)
    if (!modules_loaded) {
        load_modules();
    }
    for (unsigned int i = 0; i < 6; ++i) {
        if (modules[i] == module ||
            modules_tap[i] == module ||
            modules_wrist[i] == module ||
            modules_sleep[i] == module) {
            return i <= 3 || (i > 3 && center_slots_enabled);
        }
    }
    return false;
}

int get_slot_for_module(int module) {
    // Find which slot a module is assigned to in the current display state
    if (!modules_loaded) {
        load_modules();
    }

    uint8_t *current_modules;
#if !defined PBL_PLATFORM_APLITE
    if (tap_mode_visible()) {
        current_modules = modules_tap;
    } else if (wrist_mode_visible()) {
        current_modules = modules_wrist;
    } else if (should_show_sleep_data()) {
        current_modules = modules_sleep;
    } else {
        current_modules = modules;
    }
#else
    current_modules = modules;
#endif

    for (unsigned int i = 0; i < 6; ++i) {
        if (current_modules[i] == module) {
            return i;
        }
    }
    return -1; // Module not found in any slot
}

void toggle_center_slots(bool enable) {
    center_slots_enabled = enable;
}

// =============================================================================
// Configuration Bitmask Query Functions
// =============================================================================

int get_config_toggles(void) {
    if (!configs_loaded) {
        // Load the config bitmask from persistent storage on first access
        configs = persist_exists(KEY_CONFIGS) ? persist_read_int(KEY_CONFIGS) : 0;
        configs_loaded = true;
    }
    return configs;
}

void set_config_toggles(int toggles) {
    configs = toggles;
    configs_loaded = true;
}

// Each toggle query function checks if a specific bit is set in the config bitmask.
// For example, if FLAG_WEATHER (0x0001) is set, weather tracking is enabled.

bool is_weather_toggle_enabled(void) {
    return get_config_toggles() & FLAG_WEATHER;
}

bool is_health_toggle_enabled(void) {
    return get_config_toggles() & FLAG_HEALTH;
}

bool is_use_celsius_enabled(void) {
    return get_config_toggles() & FLAG_CELSIUS;
}

bool is_use_km_enabled(void) {
    return get_config_toggles() & FLAG_KM;
}

bool is_bluetooth_vibrate_enabled(void) {
    return get_config_toggles() & FLAG_BLUETOOTH;
}

bool is_update_disabled(void) {
    return get_config_toggles() & FLAG_UPDATE;
}

bool is_leading_zero_disabled(void) {
    return get_config_toggles() & FLAG_LEADINGZERO;
}

bool is_advanced_colors_enabled(void) {
    return get_config_toggles() & FLAG_ADVANCED;
}

bool is_sleep_data_enabled(void) {
    return get_config_toggles() & FLAG_SLEEP;
}

bool is_use_calories_enabled(void) {
    return get_config_toggles() & FLAG_CALORIES;
}

bool is_simple_mode_enabled(void) {
    return false; // Simple mode is reserved for future use
}

bool is_timezone_enabled(void) {
    return true; // Timezone display is always enabled
}

bool is_quickview_disabled(void) {
    return get_config_toggles() & FLAG_QUICKVIEW;
}

bool is_tap_enabled(void) {
    return get_config_toggles() & FLAG_TAP;
}

bool is_wrist_enabled(void) {
    return get_config_toggles() & FLAG_WRIST;
}

bool is_mute_on_quiet_enabled(void) {
    return get_config_toggles() && FLAG_MUTEONQUIET;
}
