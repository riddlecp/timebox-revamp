/**
 * locales.h - Date formatting and localization.
 *
 * Provides functions to format dates in different locales (languages)
 * and date formats, with support for localization of weekday names,
 * month names, and date separators.
 */

#ifndef __TIMEBOXED_LOCALE_
#define __TIMEBOXED_LOCALE_

#include <pebble.h>

/**
 * Get the current date formatted as a string in the selected locale.
 * @param tick_time  Current time struct
 * @param buffer     Output buffer for the formatted date
 * @param buffer_size Size of the output buffer
 */
void get_current_date(struct tm *tick_time, char *buffer, uint16_t buffer_size);

/**
 * Load and return the current locale (language) setting from persistent storage.
 * @return Locale ID (LC_ENGLISH, LC_FRENCH, etc.)
 */
int load_locale(void);

#endif // __TIMEBOXED_LOCALE_
