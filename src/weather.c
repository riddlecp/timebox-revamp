/**
 * weather.c - Weather data fetching and display implementation.
 *
 * This file handles:
 *   - Requesting weather data from the companion app
 *   - Mapping weather condition codes to FontAwesome weather icons
 *   - Converting wind direction from degrees to cardinal directions
 *   - Managing the weather update interval (default: 30 minutes)
 */

#include <pebble.h>
#include "configs.h"
#include "keys.h"
#include "health.h"
#include "text.h"
#include "weather.h"

// =============================================================================
// Internal State
// =============================================================================

static bool weather_enabled;   // Whether weather tracking is enabled
static bool use_celsius;       // Whether to display Celsius or Fahrenheit
static int last_update = 0;    // Unix timestamp of last weather update
static int weather_interval = 30; // Update interval in minutes

// =============================================================================
// Weather Condition Icons
// Maps weather condition codes (from Yahoo Weather API) to FontAwesome icon characters.
// These icons are rendered using the weather icon font.
// =============================================================================

static char* weather_conditions[] = {
    "\xF07B", // unknown
    "\xF00D", // clear (day)
    "\xF00D", // sunny (day)
    "\xF002", // partly cloudy (day)
    "\xF002", // mostly cloudy (day)
    "\xF00C", // mostly sunny (day)
    "\xF002", // partly sunny (day)
    "\xF013", // cloudy
    "\xF019", // rain
    "\xF01B", // snow
    "\xF01D", // thunderstorms
    "\xF0B5", // sleet
    "\xF00A", // flurries
    "\xF0B6", // hazy
    "\xF01D", // chance of thunderstorms
    "\xF01B", // chance of snow
    "\xF0B5", // chance of sleet
    "\xF008", // chance of rain
    "\xF01B", // chance of flurries
    "\xF07B", // unknown (night)
    "\xF02E", // clear (night)
    "\xF02E", // sunny (night)
    "\xF086", // partly cloudy (night)
    "\xF086", // mostly cloudy (night)
    "\xF081", // mostly sunny (night)
    "\xF086", // partly sunny (night)
    "\xF013", // cloudy (night)
    "\xF019", // rain (night)
    "\xF01B", // snow (night)
    "\xF01D", // thunderstorms (night)
    "\xF0B5", // sleet (night)
    "\xF038", // flurries (night)
    "\xF04A", // hazy (night)
    "\xF01D", // chance of thunderstorms (night)
    "\xF038", // chance of snow (night)
    "\xF0B3", // chance of sleet (night)
    "\xF036", // chance of rain (night)
    "\xF038", // chance of flurries (night)
    "\xF003", // fog
    "\xF04A", // fog (night)
    "\xF04E", // drizzle
    "\xF015", // hail
    "\xF076", // cold
    "\xF072", // hot
    "\xF050", // windy
    "\xF056", // tornado
    "\xF073", // hurricane
};

// =============================================================================
// Weather Data Fetching
// =============================================================================

void update_weather(bool force) {
    // Only fetch if we should, or if force is true
    int current_time = (int)time(NULL);
    if (force || last_update == 0 || (current_time - last_update) >= weather_interval * 60) {
        // Request weather data from the companion app via AppMessage
        DictionaryIterator *iter;
        AppMessageResult result = app_message_outbox_begin(&iter);
        if (result == APP_MSG_OK) {
            dict_write_uint8(iter, KEY_REQUESTWEATHER, 1);
            result = app_message_outbox_send();
            if (result == APP_MSG_OK) {
                last_update = current_time;
            }
        }
    }
}

// =============================================================================
// Wind Direction Conversion
// Converts degrees (0-360) to a 16-point compass direction.
// =============================================================================

char* get_wind_direction_text(int degrees) {
    // Map degrees to one of 16 compass points
    // Each sector covers 22.5 degrees (360/16 = 22.5)
    if (degrees > 349 || degrees <= 11)    return "S";  // North
    if (degrees > 11 && degrees <= 34)      return "SSW"; // North-northwest
    if (degrees > 34 && degrees <= 56)      return "SW"; // West-southwest
    if (degrees > 56 && degrees <= 79)      return "WSW"; // West-southwest
    if (degrees > 79 && degrees <= 101)     return "W";  // West
    if (degrees > 101 && degrees <= 124)    return "WNW"; // West-northwest
    if (degrees > 124 && degrees <= 146)    return "NW"; // Northwest
    if (degrees > 146 && degrees <= 169)    return "NNW"; // North-northwest
    if (degrees > 169 && degrees <= 191)    return "N";  // South
    if (degrees > 191 && degrees <= 214)    return "NNE"; // North-northeast
    if (degrees > 214 && degrees <= 236)    return "NE"; // Northeast
    if (degrees > 236 && degrees <= 259)    return "ENE"; // East-northeast
    if (degrees > 259 && degrees <= 281)    return "E";  // East
    if (degrees > 281 && degrees <= 304)    return "ESE"; // East-southeast
    if (degrees > 304 && degrees <= 326)    return "SE"; // Southeast
    if (degrees > 326 && degrees <= 349)    return "SSE"; // South-southeast
    return "NONE";
}

char* get_wind_direction(int degrees) {
    // Convert degrees to a single character for the icon font
    // Each sector covers 22.5 degrees (360/16 = 22.5)
    if (degrees > 349 || degrees <= 11)    return "0";
    if (degrees > 11 && degrees <= 34)      return "1";
    if (degrees > 34 && degrees <= 56)      return "2";
    if (degrees > 56 && degrees <= 79)      return "3";
    if (degrees > 79 && degrees <= 101)     return "4";
    if (degrees > 101 && degrees <= 124)    return "5";
    if (degrees > 124 && degrees <= 146)    return "6";
    if (degrees > 146 && degrees <= 169)    return "7";
    if (degrees > 169 && degrees <= 191)    return "S";
    if (degrees > 191 && degrees <= 214)    return "T";
    if (degrees > 214 && degrees <= 236)    return "U";
    if (degrees > 236 && degrees <= 259)    return "V";
    if (degrees > 259 && degrees <= 281)    return "W";
    if (degrees > 281 && degrees <= 304)    return "X";
    if (degrees > 304 && degrees <= 326)    return "Y";
    if (degrees > 326 && degrees <= 349)    return "Z";
    return "o";
}

// =============================================================================
// Display Updates
// =============================================================================

void update_weather_values(int temp_val, int weather_val) {
    if (is_module_enabled(MODULE_WEATHER)) {
        char temp_pattern[4];
        char temp_text[8];
        char weather_text[4];

        // Format temperature with degree symbol and unit (C or F)
        // Blocko fonts use lowercase 'c'/'f', others use uppercase
        if (get_loaded_font() == BLOCKO_BIG_FONT || get_loaded_font() == BLOCKO_FONT) {
            strcpy(temp_pattern, use_celsius ? "%dc" : "%df");
        } else {
            strcpy(temp_pattern, use_celsius ? "%dC" : "%dF");
        }

        snprintf(temp_text, sizeof(temp_text), temp_pattern, temp_val);
        snprintf(weather_text, sizeof(weather_text), "%s", weather_conditions[weather_val]);

        set_temp_cur_layer_text(temp_text);
        set_weather_layer_text(weather_text);
    } else {
        set_temp_cur_layer_text("");
        set_weather_layer_text("");
    }
}

void update_forecast_values(int max_val, int min_val) {
    if (is_module_enabled(MODULE_FORECAST)) {
        char max_text[6];
        char min_text[6];

        snprintf(max_text, sizeof(max_text), "%d", max_val);
        snprintf(min_text, sizeof(min_text), "%d", min_val);

        set_temp_max_layer_text(max_text);
        set_temp_min_layer_text(min_text);
        set_max_icon_layer_text("y"); // FontAwesome "temperature high" icon
        set_min_icon_layer_text("z"); // FontAwesome "temperature low" icon
    } else {
        set_temp_max_layer_text("");
        set_temp_min_layer_text("");
        set_max_icon_layer_text("");
        set_min_icon_layer_text("");
    }
}

void update_wind_values(int speed, int direction) {
    if (is_module_enabled(MODULE_WIND)) {
        char wind_speed[4];
        char wind_dir[2];
        char *wind_unit;

        // Convert wind direction to icon character
        strcpy(wind_dir, get_wind_direction(direction));

        // Convert wind speed to user's preferred unit
        if (get_wind_speed_unit() == UNIT_KPH) {
            speed = (speed * 1.60934) / 1;
            wind_unit = ")"; // Icon font character for "km/h"
        } else if (get_wind_speed_unit() == UNIT_KNOTS) {
            speed = (speed * 0.868976) / 1;
            wind_unit = "*"; // Icon font character for "knots"
        } else {
            wind_unit = "("; // Icon font character for "mph"
        }

        snprintf(wind_speed, sizeof(wind_speed), "%d", speed);
        set_wind_direction_layer_text(wind_dir);
        set_wind_speed_layer_text(wind_speed);
        set_wind_unit_layer_text(wind_unit);
    } else {
        set_wind_direction_layer_text("");
        set_wind_speed_layer_text("");
        set_wind_unit_layer_text("");
    }
}

void update_sunrise(int sunrise) {
    if (is_module_enabled(MODULE_SUNRISE)) {
        char sunrise_text[6];
        time_t temp = (time_t)sunrise;
        struct tm *tick_time = localtime(&temp);

        // Format sunrise time from Unix timestamp
        set_hours(tick_time, sunrise_text, sizeof(sunrise_text));
        set_sunrise_layer_text(sunrise_text);
        set_sunrise_icon_layer_text("\xF051"); // FontAwesome "sun" icon
    } else {
        set_sunrise_layer_text("");
        set_sunrise_icon_layer_text("");
    }
}

void update_sunset(int sunset) {
    if (is_module_enabled(MODULE_SUNSET)) {
        char sunset_text[6];
        time_t temp = (time_t)sunset;
        struct tm *tick_time = localtime(&temp);

        // Format sunset time from Unix timestamp
        set_hours(tick_time, sunset_text, sizeof(sunset_text));
        set_sunset_layer_text(sunset_text);
        set_sunset_icon_layer_text("\xF052"); // FontAwesome "moon" icon
    } else {
        set_sunset_layer_text("");
        set_sunset_icon_layer_text("");
    }
}

// =============================================================================
// Weather Enable/Disable
// =============================================================================

static bool get_weather_enabled(void) {
    // Weather is enabled if the weather toggle is on OR any weather-related module is assigned
    bool weather_module_available =
        is_module_enabled(MODULE_WEATHER) ||
        is_module_enabled(MODULE_FORECAST) ||
        is_module_enabled(MODULE_WIND) ||
        is_module_enabled(MODULE_SUNRISE) ||
        is_module_enabled(MODULE_SUNSET);
    return is_weather_toggle_enabled() || weather_module_available;
}

static void update_weather_from_storage(void) {
    // Restore weather data from persistent storage (after watchface reload)
    if (persist_exists(KEY_TEMP)) {
        update_weather_values(persist_read_int(KEY_TEMP), persist_read_int(KEY_WEATHER));
    }
    if (persist_exists(KEY_MIN)) {
        update_forecast_values(persist_read_int(KEY_MAX), persist_read_int(KEY_MIN));
    }
    if (persist_exists(KEY_SPEED)) {
        update_wind_values(persist_read_int(KEY_SPEED), persist_read_int(KEY_DIRECTION));
    }
    if (persist_exists(KEY_SUNRISE)) {
        update_sunrise(persist_read_int(KEY_SUNRISE));
    }
    if (persist_exists(KEY_SUNSET)) {
        update_sunset(persist_read_int(KEY_SUNSET));
    }
}

void toggle_weather(uint8_t reload_origin) {
    weather_enabled = get_weather_enabled();

    // Load update interval from storage on config reload
    if (reload_origin == RELOAD_CONFIGS || reload_origin == RELOAD_DEFAULT) {
        weather_interval = persist_exists(KEY_WEATHERTIME) ?
            persist_read_int(KEY_WEATHERTIME) : 30;
    }

    if (weather_enabled) {
        use_celsius = is_use_celsius_enabled();
        update_weather_from_storage();
        if (reload_origin == RELOAD_MODULE || reload_origin == RELOAD_CONFIGS) {
            update_weather(true); // Force immediate update
        }
    } else {
        // Clear all weather displays
        set_temp_cur_layer_text("");
        set_temp_max_layer_text("");
        set_temp_min_layer_text("");
        set_weather_layer_text("");
        set_max_icon_layer_text("");
        set_min_icon_layer_text("");
        set_wind_direction_layer_text("");
        set_wind_speed_layer_text("");
        set_wind_unit_layer_text("");
        set_sunrise_layer_text("");
        set_sunrise_icon_layer_text("");
        set_sunset_icon_layer_text("");
    }
}

void store_weather_values(int temp, int max, int min, int weather,
                          int speed, int direction, int sunrise, int sunset) {
    // Store weather data in persistent memory for restoration after reload
    persist_write_int(KEY_TEMP, temp);
    persist_write_int(KEY_MAX, max);
    persist_write_int(KEY_MIN, min);
    persist_write_int(KEY_WEATHER, weather);
    persist_write_int(KEY_SPEED, speed);
    persist_write_int(KEY_DIRECTION, direction);
    persist_write_int(KEY_SUNRISE, sunrise);
    persist_write_int(KEY_SUNSET, sunset);
}

bool is_weather_enabled(void) {
    return weather_enabled;
}
