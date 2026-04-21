/**
 * configs.h - Configuration management for the Timeboxed watchface.
 *
 * Handles loading and querying user preferences stored in persistent
 * memory. These preferences control which modules are enabled, unit
 * preferences, and display toggles.
 */

#ifndef __TIMEBOXED_CONFIGS_
#define __TIMEBOXED_CONFIGS_

#include <pebble.h>

// Forward declarations
struct Window;

// =============================================================================
// Configuration Loading
// =============================================================================

/**
 * Load configuration toggles from persistent storage.
 * Returns an integer bitmask combining all enabled flags.
 */
int get_config_toggles(void);

/**
 * Set configuration toggles in memory (called after receiving new config from companion).
 */
void set_config_toggles(int toggles);

// =============================================================================
// Module Configuration
// =============================================================================

/**
 * Assign a module to a specific display slot for a given state.
 * @param slot    Which slot to assign (SLOT_A through SLOT_F)
 * @param module  Which module to place there (MODULE_WEATHER, MODULE_STEPS, etc.)
 * @param state   Which display state this applies to (STATE_NORMAL, STATE_SLEEP, etc.)
 */
void set_module(int slot, int module, int state);

/**
 * Check if a specific module is currently enabled and visible.
 * Considers the current display state (normal, sleep, tap, wrist).
 * @param module  The module type to check
 * @return true if the module is assigned to a visible slot
 */
bool is_module_enabled(int module);

/**
 * Check if a module is enabled in ANY display state.
 * Used to determine if resources should be allocated.
 * @param module  The module type to check
 * @return true if the module is enabled in any state
 */
bool is_module_enabled_any(int module);

/**
 * Find which slot a module is assigned to.
 * @param module  The module type to look up
 * @return The slot number (0-5), or -1 if not found
 */
int get_slot_for_module(int module);

/**
 * Enable or disable center slots (E and F) on round screens.
 * @param enable  true to show center slots, false to hide them
 */
void toggle_center_slots(bool enable);

// =============================================================================
// Toggle Query Functions
// Each returns true if the corresponding feature is enabled.
// These read from the stored config bitmask.
// =============================================================================

/** Weather tracking enabled */
bool is_weather_toggle_enabled(void);

/** Health data tracking enabled */
bool is_health_toggle_enabled(void);

/** Use Celsius temperature scale */
bool is_use_celsius_enabled(void);

/** Use kilometers for distance */
bool is_use_km_enabled(void);

/** Vibrate on Bluetooth disconnect */
bool is_bluetooth_vibrate_enabled(void);

/** Update notifications disabled */
bool is_update_disabled(void);

/** Leading zeros disabled for hours (e.g., "9" instead of "09") */
bool is_leading_zero_disabled(void);

/** Advanced color customization enabled */
bool is_advanced_colors_enabled(void);

/** Sleep data display enabled */
bool is_sleep_data_enabled(void);

/** Calorie display enabled */
bool is_use_calories_enabled(void);

/** Simple display mode enabled */
bool is_simple_mode_enabled(void);

/** Timezone display enabled */
bool is_timezone_enabled(void);

/** Quick view disabled */
bool is_quickview_disabled(void);

/** Tap-to-show feature enabled */
bool is_tap_enabled(void);

/** Wrist-to-show feature enabled */
bool is_wrist_enabled(void);

/** Mute during quiet hours enabled */
bool is_mute_on_quiet_enabled(void);

// =============================================================================
// Unit Preferences
// =============================================================================

/**
 * Get the preferred wind speed unit.
 * @return UNIT_MPH, UNIT_KPH, or UNIT_KNOTS
 */
int get_wind_speed_unit(void);

#endif // __TIMEBOXED_CONFIGS_
