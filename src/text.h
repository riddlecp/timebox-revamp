/**
 * text.h - Text layer management interface.
 *
 * Provides functions to:
 *   - Create/destroy all text layers on the watchface
 *   - Load/unload custom fonts
 *   - Set fonts and colors on all text layers
 *   - Update text content for individual layers
 */

#ifndef __TIMEBOXED_TEXT_
#define __TIMEBOXED_TEXT_

#include <pebble.h>

// =============================================================================
// Text Layer Creation/ Destruction
// =============================================================================

/** Create all text layers and add them to the window */
void create_text_layers(struct Window *window);

/** Destroy all text layers and free resources */
void destroy_text_layers(void);

// =============================================================================
// Font Management
// =============================================================================

/** Load custom fonts from resources based on user's font preference */
void load_face_fonts(void);

/** Unload all custom fonts */
void unload_face_fonts(void);

/** Apply fonts to all text layers */
void set_face_fonts(void);

/** Get the currently loaded font type */
uint8_t get_loaded_font(void);

// =============================================================================
// Color Management
// =============================================================================

/** Set base colors for all text layers based on user preferences */
void set_colors(struct Window *window);

/** Set Bluetooth layer color based on connection state */
void set_bluetooth_color(void);

/** Set update notification color */
void set_update_color(void);

/** Set battery layer color based on charge level */
void set_battery_color(int percentage);

// =============================================================================
// Health Progress Colors
// Set colors for health modules showing "on target" or "behind pace"
// =============================================================================

void set_progress_color_steps(bool falling_behind);
void set_progress_color_dist(bool falling_behind);
void set_progress_color_cal(bool falling_behind);
void set_progress_color_sleep(bool falling_behind);
void set_progress_color_deep(bool falling_behind);
void set_progress_color_active(bool falling_behind);
void set_progress_color_heart(int heart_value);

// =============================================================================
// Text Content Setters
// Each function updates the text for one specific text layer.
// Uses static buffers to avoid heap allocation.
// =============================================================================

// Time display
void set_hours_layer_text(char* text);
void set_date_layer_text(char* text);
void set_alt_time_layer_text(char* text);
void set_alt_time_b_layer_text(char* text);
void set_seconds_layer_text(char* text);

// Battery and Bluetooth
void set_battery_layer_text(char* text);
void set_bluetooth_layer_text(char* text);

// Weather
void set_temp_cur_layer_text(char* text);
void set_temp_max_layer_text(char* text);
void set_temp_min_layer_text(char* text);
void set_weather_layer_text(char* text);
void set_max_icon_layer_text(char* text);
void set_min_icon_layer_text(char* text);
void set_wind_speed_layer_text(char* text);
void set_wind_direction_layer_text(char* text);
void set_wind_unit_layer_text(char* text);
void set_sunrise_layer_text(char* text);
void set_sunrise_icon_layer_text(char* text);
void set_sunset_layer_text(char* text);
void set_sunset_icon_layer_text(char* text);

// Health data
void set_steps_layer_text(char* text);
void set_dist_layer_text(char* text);
void set_cal_layer_text(char* text);
void set_sleep_layer_text(char* text);
void set_deep_layer_text(char* text);
void set_active_layer_text(char* text);
void set_heart_layer_text(char* text);
void set_heart_icon_layer_text(char* text);

// Compass
void set_degrees_layer_text(char* text);
void set_compass_layer_text(char* text);

// Crypto
void set_crypto_layer_text(char* text);
void set_crypto_b_layer_text(char* text);
void set_crypto_c_layer_text(char* text);
void set_crypto_d_layer_text(char* text);

// Update notification
void set_update_layer_text(char* text);

#endif // __TIMEBOXED_TEXT_
