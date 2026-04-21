/**
 * clock.h - Time formatting and timezone management.
 *
 * Provides functions to format hours, minutes, and seconds for display,
 * as well as timezone conversion utilities.
 */

#ifndef __TIMEBOXED_CLOCK_
#define __TIMEBOXED_CLOCK_

#include <pebble.h>

/**
 * Format the hours portion of the time into the provided buffer.
 * @param tick_time   Current time struct from the tick handler
 * @param buffer      Output buffer for the formatted hours string
 * @param buffer_size Size of the output buffer
 */
void set_hours(struct tm *tick_time, char *buffer, uint16_t buffer_size);

/**
 * Format the minutes portion of the time into the provided buffer.
 * @param tick_time   Current time struct from the tick handler
 * @param buffer      Output buffer for the formatted minutes string
 * @param buffer_size Size of the output buffer
 */
void set_minutes(struct tm *tick_time, char *buffer, uint16_t buffer_size);

/**
 * Format the seconds portion of the time into the provided buffer.
 * @param tick_time   Current time struct from the tick handler
 * @param buffer      Output buffer for the formatted seconds string
 * @param buffer_size Size of the output buffer
 */
void set_seconds(struct tm *tick_time, char *buffer, uint16_t buffer_size);

/**
 * Set the timezone for alternative time display.
 * @param name   Timezone name/code string (e.g., "EST", "PST")
 * @param hour   Hour offset from UTC (e.g., -5 for EST)
 * @param minute Minute offset (usually 0, can be non-zero for places like Nepal +5:45)
 */
void set_timezone(char *name, int hour, uint8_t minute);

#if !defined PBL_PLATFORM_APLITE
/**
 * Set the second timezone for alternative time display.
 * @param name   Timezone name/code string
 * @param hour   Hour offset from UTC
 * @param minute Minute offset
 */
void set_timezone_b(char *name, int hour, uint8_t minute);
#endif

/** Update the displayed time (called every minute) */
void update_time(void);

/** Update the seconds display (called every second) */
void update_seconds(struct tm *tick_time);

#endif // __TIMEBOXED_CLOCK_
