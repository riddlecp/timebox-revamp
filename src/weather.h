/**
 * weather.h - Weather data fetching and display.
 *
 * Handles:
 *   - Requesting weather data from the companion app via AppMessage
 *   - Converting weather condition codes to icon characters
 *   - Converting wind direction degrees to cardinal directions
 *   - Storing weather data in persistent memory
 *   - Managing update intervals
 */

#ifndef __TIMEBOXED_WEATHER_
#define __TIMEBOXED_WEATHER_

#include <pebble.h>

/**
 * Request a weather data update from the companion app.
 * @param force  If true, ignore the update interval and fetch immediately
 */
void update_weather(bool force);

/**
 * Update the current temperature and weather icon display.
 * @param temp_val    Temperature value (in user's preferred unit)
 * @param weather_val Weather condition code (maps to a weather icon)
 */
void update_weather_values(int temp_val, int weather_val);

/**
 * Update the high/low temperature forecast display.
 * @param max_val  High temperature
 * @param min_val  Low temperature
 */
void update_forecast_values(int max_val, int min_val);

/**
 * Update the wind speed and direction display.
 * @param speed     Wind speed value
 * @param direction Wind direction in degrees (0-360)
 */
void update_wind_values(int speed, int direction);

/**
 * Update the sunrise time display.
 * @param sunrise  Unix timestamp for sunrise
 */
void update_sunrise(int sunrise);

/**
 * Update the sunset time display.
 * @param sunset  Unix timestamp for sunset
 */
void update_sunset(int sunset);

/**
 * Store weather values in persistent memory.
 * Data is stored so it can be restored after a watchface reload.
 */
void store_weather_values(int temp_val, int max_val, int min_val,
                          int weather_val, int speed_val, int direction_val,
                          int sunrise_val, int sunset_val);

/**
 * Enable or disable weather tracking based on user configuration.
 * @param reload_origin  Why this is being called (RELOAD_CONFIGS, RELOAD_MODULE, etc.)
 */
void toggle_weather(uint8_t reload_origin);

/** Check if weather tracking is currently enabled */
bool is_weather_enabled(void);

/**
 * Convert wind direction degrees to a single-character icon code.
 * @param degrees  Wind direction in degrees (0-360)
 * @return          Single character representing the direction
 */
char* get_wind_direction(int degrees);

/**
 * Convert wind direction degrees to a cardinal direction string.
 * @param degrees  Wind direction in degrees (0-360)
 * @return          Cardinal direction string (N, NE, E, SE, etc.)
 */
char* get_wind_direction_text(int degrees);

#endif // __TIMEBOXED_WEATHER_
