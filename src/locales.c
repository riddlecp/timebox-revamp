/**
 * locales.c - Date formatting and localization implementation.
 *
 * This file provides:
 *   - Localized weekday and month name arrays for multiple languages
 *   - Date formatting that respects user's locale and format preferences
 *   - Loading locale settings from persistent storage
 */

#include <pebble.h>
#include "keys.h"
#include "locales.h"

// =============================================================================
// Localized Weekday Names
// Each row corresponds to a locale (LC_ENGLISH, LC_FRENCH, etc.)
// =============================================================================

static char* WEEKDAYS[][7] = {
    // English
    {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
    // Portuguese
    {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"},
    // French
    {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},
    // German
    {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"},
    // Spanish
    {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"},
    // Italian
    {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"},
    // Dutch
    {"Zo", "Ma", "Di", "Wo", "Do", "Vr", "Za"},
    // Danish
    {"Søn", "Man", "Tir", "Ons", "Tor", "Fre", "Lør"},
    // Turkish
    {"Paz", "Pzt", "Sal", "Cars", "Per", "Cum", "Cts"},
    // Czech
    {"Ne", "Po", "Ut", "St", "St", "Pá", "So"},
    // Polish
    {"Ndz", "Pon", "Wt", "Sr", "Czw", "Pt", "Sob"},
    // Swedish
    {"Sön", "Mån", "Tis", "Ons", "Tor", "Fre", "Lör"},
    // Finnish
    {"S", "Ma", "Ti", "Ke", "To", "Pe", "La"},
    // Slovak
    {"Ne", "Po", "Ut", "St", "Št", "Pi", "So"},
};

// =============================================================================
// Localized Month Names
// =============================================================================

static char* MONTHS[][12] = {
    // English
    {"January", "February", "March", "April", "May", "June",
     "July", "August", "September", "October", "November", "December"},
    // Portuguese
    {"Janeiro", "Fevereiro", "Marco", "Abril", "Maio", "Junho",
     "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"},
    // French
    {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin",
     "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"},
    // German
    {"Januar", "Februar", "Marz", "April", "Mai", "Juni",
     "Juli", "August", "September", "Oktober", "November", "Dezember"},
    // Spanish
    {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio",
     "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"},
    // Italian
    {"Gennaio", "Febbraio", "Marzo", "Aprile", "Maggio", "Giugno",
     "Luglio", "Agosto", "Settembre", "Ottobre", "Novembre", "Dicembre"},
    // Dutch
    {"Januari", "Februari", "Maart", "April", "Mei", "Juni",
     "Juli", "Augustus", "September", "Oktober", "November", "December"},
    // Danish
    {"Januar", "Februar", "Marts", "April", "Maj", "Juni",
     "July", "August", "September", "Oktober", "November", "December"},
    // Turkish
    {"Ocak", "Subat", "Mart", "Nisan", "Mayis", "Haziran",
     "Temmuz", "Agustos", "Eylul", "Ekim", "Kasim", "Aralik"},
    // Czech
    {"Leden", "Unor", "Brezen", "Duben", "Kveten", "Cerven",
     "Cervenec", "Srpen", "Zari", "Rijen", "Listopad", "Prosinec"},
    // Polish
    {"Sty", "Lut", "Mar", "Kwi", "Maj", "Cze",
     "Lip", "Sie", "Wrz", "Paz", "Lis", "Gru"},
    // Swedish
    {"Januari", "Februari", "Mars", "April", "Maj", "Juni",
     "Juli", "Augusti", "September", "Oktober", "November", "December"},
    // Finnish
    {"Tammikuu", "Helmikuu", "Maaliskuu", "Huhtikuu", "Toukokuu", "Kesakuu",
     "Heinakuu", "Elokuu", "Syyskuu", "Lokakuu", "Marraskuu", "Joulukuu"},
    // Slovak
    {"Januar", "Februar", "Marec", "April", "Maj", "Jún",
     "Júl", "August", "September", "Október", "November", "December"},
};

// =============================================================================
// Date Separator Characters
// Different locales use different separators between day and month
// =============================================================================

static char* DATE_SEPARATORS[] = {
    "-",  // English (default)
    "-",
    "/",
    ".",
    "-",
    ".",
    "-",
    "-",
    "-",
    ".",
    ".",
    "-",
    ".",
    ".",
};

// =============================================================================
// Date Formatting
// =============================================================================

void get_current_date(struct tm *tick_time, char *buffer, uint16_t buffer_size) {
    // Get the user's locale and date format preferences
    int locale = load_locale();
    int date_format = persist_exists(KEY_DATEFORMAT) ? persist_read_int(KEY_DATEFORMAT) : FORMAT_WMD;
    char separator = DATE_SEPARATORS[locale][0];

    // Get localized weekday and month names
    char* weekday = WEEKDAYS[locale][tick_time->tm_wday];
    char* month = MONTHS[locale][tick_time->tm_mon];

    // Format the date based on the selected format style
    switch (date_format) {
        case FORMAT_WMD:
            // "Mon, January 15"
            snprintf(buffer, buffer_size, "%s, %s %d", weekday, month, tick_time->tm_mday);
            break;
        case FORMAT_WDM:
            // "Mon, 15 January"
            snprintf(buffer, buffer_size, "%s, %d %s", weekday, tick_time->tm_mday, month);
            break;
        case FORMAT_WD:
            // "Mon 15"
            snprintf(buffer, buffer_size, "%s %d", weekday, tick_time->tm_mday);
            break;
        case FORMAT_DM:
            // "15 January"
            snprintf(buffer, buffer_size, "%d %s", tick_time->tm_mday, month);
            break;
        case FORMAT_MD:
            // "January 15"
            snprintf(buffer, buffer_size, "%s %d", month, tick_time->tm_mday);
            break;
        case FORMAT_DSM:
            // "15-January" (day, separator, month)
            snprintf(buffer, buffer_size, "%d-%s", tick_time->tm_mday, month);
            break;
        case FORMAT_SMD:
            // "-January-15" (separator, month, day)
            snprintf(buffer, buffer_size, "-%s-%d", month, tick_time->tm_mday);
            break;
        case FORMAT_WDSM:
            // "Mon 15-January"
            snprintf(buffer, buffer_size, "%s %d-%s", weekday, tick_time->tm_mday, month);
            break;
        case FORMAT_WSMD:
            // "Mon -January- 15"
            snprintf(buffer, buffer_size, "%s -%s- %d", weekday, month, tick_time->tm_mday);
            break;
        case FORMAT_ISO:
            // "2024-01-15"
            snprintf(buffer, buffer_size, "%04d-%02d-%02d",
                     tick_time->tm_year + 1900, tick_time->tm_mon + 1, tick_time->tm_mday);
            break;
        case FORMAT_WNDSM:
            // "Mon 1/15-January"
            snprintf(buffer, buffer_size, "%s %d/%d-%s", weekday, tick_time->tm_wday + 1,
                     tick_time->tm_mday, month);
            break;
        case FORMAT_WNSMD:
            // "Mon 1/-January- 15"
            snprintf(buffer, buffer_size, "%s %d/-%s- %d", weekday, tick_time->tm_wday + 1,
                     month, tick_time->tm_mday);
            break;
        default:
            // Default format: "Mon, January 15"
            snprintf(buffer, buffer_size, "%s, %s %d", weekday, month, tick_time->tm_mday);
            break;
    }
}

int load_locale(void) {
    // Load the user's locale preference from persistent storage
    // Default to English (LC_ENGLISH = 0) if no preference is stored
    return persist_exists(KEY_LOCALE) ? persist_read_int(KEY_LOCALE) : LC_ENGLISH;
}
