/**
 * clock.c - Time formatting and timezone management implementation.
 *
 * This file handles:
 *   - Formatting hours, minutes, and seconds for display
 *   - Managing timezone offsets for alternative time display
 *   - Updating the displayed time when the timezone changes
 */

#include <pebble.h>
#include "keys.h"
#include "text.h"
#include "configs.h"
#include "clock.h"

// =============================================================================
// Internal State
// =============================================================================

// Stored timezone data for alternative time display
static char tz_name[TZ_LEN];     // Timezone code (e.g., "EST", "PST")
static int tz_hour = 0;          // Hour offset from UTC
static uint8_t tz_minute = 0;    // Minute offset from UTC

#if !defined PBL_PLATFORM_APLITE
static char tz_name_b[TZ_LEN];   // Second timezone code
static int tz_hour_b = 0;        // Second timezone hour offset
static uint8_t tz_minute_b = 0;  // Second timezone minute offset
#endif

// =============================================================================
// Time Formatting
// =============================================================================

void set_hours(struct tm *tick_time, char *buffer, uint16_t buffer_size) {
    // Convert 24-hour time to 12-hour format
    int hours = tick_time->tm_hour;
    int minutes = tick_time->tm_min;
    char ampm = 'A'; // 'A' for AM, 'P' for PM

    // Determine AM/PM
    if (hours >= 12) {
        ampm = 'P';
    }

    // Convert to 12-hour format
    if (hours > 12) {
        hours -= 12;
    } else if (hours == 0) {
        hours = 12; // Midnight displays as 12, not 0
    }

    // Check if leading zero should be shown (controlled by user preference)
    // When leading zero is enabled (FLAG_LEADINGZERO bit NOT set), show "09"
    // When disabled, show "9"
    bool has_leading_zero = !is_leading_zero_disabled();

    // Format the time string: "H:MM" or "HH:MM" depending on leading zero setting
    if (has_leading_zero) {
        snprintf(buffer, buffer_size, "%02d:%02d", hours, minutes);
    } else {
        snprintf(buffer, buffer_size, "%d:%02d", hours, minutes);
    }
}

void set_minutes(struct tm *tick_time, char *buffer, uint16_t buffer_size) {
    // Format just the minutes with leading zero (e.g., "05", "30")
    snprintf(buffer, buffer_size, "%02d", tick_time->tm_min);
}

void set_seconds(struct tm *tick_time, char *buffer, uint16_t buffer_size) {
    // Format just the seconds with leading zero (e.g., "05", "45")
    snprintf(buffer, buffer_size, "%02d", tick_time->tm_sec);
}

// =============================================================================
// Timezone Management
// =============================================================================

void set_timezone(char *name, int hour, uint8_t minute) {
    // Store timezone data received from the companion app
    strncpy(tz_name, name, TZ_LEN - 1);
    tz_name[TZ_LEN - 1] = '\0';
    tz_hour = hour;
    tz_minute = minute;
}

#if !defined PBL_PLATFORM_APLITE
void set_timezone_b(char *name, int hour, uint8_t minute) {
    // Store second timezone data
    strncpy(tz_name_b, name, TZ_LEN - 1);
    tz_name_b[TZ_LEN - 1] = '\0';
    tz_hour_b = hour;
    tz_minute_b = minute;
}
#endif

void update_time(void) {
    // Get current local time
    time_t temp_time = time(NULL);
    struct tm *local_time = localtime(&temp_time);

    // Calculate timezone offset in minutes
    int tz_offset_minutes = tz_hour * 60 + tz_minute;

    // Adjust the time by the timezone offset
    local_time->tm_hour += tz_hour;
    local_time->tm_min += tz_minute;

    // Format the alternative time display
    char alt_time_text[22];
    set_hours(local_time, alt_time_text, sizeof(alt_time_text));
    set_alt_time_layer_text(alt_time_text);

    // Format timezone name for display (e.g., "EST")
    char tz_display[5];
    strncpy(tz_display, tz_name, sizeof(tz_display) - 1);
    tz_display[sizeof(tz_display) - 1] = '\0';

#if !defined PBL_PLATFORM_APLITE
    // Update second timezone display
    int tz_offset_b = tz_hour_b * 60 + tz_minute_b;
    local_time->tm_hour = localtime(&temp_time)->tm_hour + tz_hour_b;
    local_time->tm_min = localtime(&temp_time)->tm_min + tz_minute_b;

    char alt_time_b_text[22];
    set_hours(local_time, alt_time_b_text, sizeof(alt_time_b_text));
    set_alt_time_b_layer_text(alt_time_b_text);
#endif
}

void update_seconds(struct tm *tick_time) {
    // Format and display the seconds indicator
    char seconds_text[4];
    snprintf(seconds_text, sizeof(seconds_text), "%02d", tick_time->tm_sec);
    set_seconds_layer_text(seconds_text);
}
