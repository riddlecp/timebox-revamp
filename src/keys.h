/**
 * keys.h - Message keys and constants for the Timeboxed watchface.
 *
 * This file defines:
 *   - AppMessage key IDs used for communication with the companion app
 *   - Configuration toggle flags (bitmask values)
 *   - Font type constants
 *   - Text alignment constants
 *   - Locale (language) constants
 *   - Date format constants
 *   - Module slot constants (A-F)
 *   - State constants (normal, sleep, tap, wrist)
 *   - Module type constants (weather, health, crypto, etc.)
 *   - Display item constants
 *   - Unit constants
 *   - Reload origin constants
 */

#ifndef __TIMEBOXED_KEYS_
#define __TIMEBOXED_KEYS_

// =============================================================================
// AppMessage Key IDs
// These are used to identify data exchanged between the watchface and companion app.
// Each key maps to a specific piece of data (temperature, slot config, color, etc.)
// =============================================================================

// Weather data keys
#define KEY_TEMP          0   // Current temperature
#define KEY_MAX           1   // High temperature
#define KEY_MIN           2   // Low temperature
#define KEY_WEATHER       3   // Weather condition code
#define KEY_WEATHERKEY    8   // Weather API key
#define KEY_WEATHERPROVIDER 43 // Weather data provider
#define KEY_YAHOOKEY      44  // Yahoo weather key
#define KEY_SUNRISE       72  // Sunrise time (Unix timestamp)
#define KEY_SUNSET        73  // Sunset time (Unix timestamp)
#define KEY_SUNRISECOLOR  74  // Sunrise text color
#define KEY_SUNSETCOLOR   75  // Sunset text color
#define KEY_WEATHERTIME   88  // Weather update interval (minutes)

// Health data keys
#define KEY_ENABLEHEALTH  4   // Enable health tracking
#define KEY_STEPS         27  // Step count
#define KEY_DIST          28  // Distance traveled
#define KEY_CAL           57  // Calories burned
#define KEY_SLEEP         41  // Sleep duration
#define KEY_DEEP          42  // Deep sleep duration
#define KEY_ACTIVE        78  // Active time
#define KEY_HEART         85  // Heart rate
#define KEY_HEARTCOLOR    86  // Heart rate display color
#define KEY_HEARTCOLOROFF 87  // Heart rate color when out of range
#define KEY_HEARTLOW      89  // Lower heart rate threshold
#define KEY_HEARTHIGH     90  // Upper heart rate threshold

// Wind data keys
#define KEY_SPEED         59  // Wind speed
#define KEY_DIRECTION     60  // Wind direction
#define KEY_WINDDIRCOLOR  61  // Wind direction color
#define KEY_WINDSPEEDCOLOR 62 // Wind speed color
#define KEY_SPEEDUNIT     65  // Wind speed unit preference

// Crypto data keys
#define KEY_CRYPTOFROM        110 // Source cryptocurrency
#define KEY_CRYPTOTO          111 // Target cryptocurrency
#define KEY_CRYPTOFROMB       112 // Second crypto pair source
#define KEY_CRYPTOTOB         113 // Second crypto pair target
#define KEY_CRYPTOPRICE       114 // Current crypto price
#define KEY_CRYPTOPRICEB      115 // Second crypto price
#define KEY_CRYPTOPRICEC      125 // Third crypto price
#define KEY_CRYPTOPRICED      126 // Fourth crypto price
#define KEY_CRYPTOMARKET      121 // Market cap
#define KEY_CRYPTOMARKETB     122 // Second market cap
#define KEY_CRYPTOMARKETC     127 // Third market cap
#define KEY_CRYPTOMARKETD     128 // Fourth market cap
#define KEY_CRYPTOTIME        129 // Crypto update interval (minutes)
#define KEY_CRYPTOCOLOR       117 // Crypto price color
#define KEY_CRYPTOBCOLOR      118 // Second crypto color
#define KEY_CRYPTOCCOLOR      123 // Third crypto color
#define KEY_CRYPTODCOLOR      124 // Fourth crypto color

// Timezone keys
#define KEY_TIMEZONES       10  // Timezone hour offset (A)
#define KEY_TIMEZONESCODE   24  // Timezone code string (A)
#define KEY_TIMEZONESMINUTES 25 // Timezone minute offset (A)
#define KEY_TIMEZONESB      107 // Timezone hour offset (B)
#define KEY_TIMEZONESBCODE  108 // Timezone code string (B)
#define KEY_TIMEZONESBMINUTES 109 // Timezone minute offset (B)

// Display configuration keys
#define KEY_BGCOLOR         11  // Background color
#define KEY_HOURSCOLOR      12  // Hours text color
#define KEY_DATECOLOR       14  // Date text color
#define KEY_ALTHOURSCOLOR   15  // Alternative time color
#define KEY_ALTHOURSBCOLOR  116 // Alternative time B color
#define KEY_DATECOLOR       14  // Date text color
#define KEY_BLUETOOTHCOLOR  30  // Bluetooth icon color
#define KEY_UPDATECOLOR     34  // Update notification color
#define KEY_BATTERYCOLOR    16  // Battery percentage color
#define KEY_BATTERYLOWCOLOR 17  // Battery low color
#define KEY_TEMPCOLOR       19  // Temperature text color
#define KEY_MINCOLOR        20  // Min temperature color
#define KEY_MAXCOLOR        21  // Max temperature color
#define KEY_STEPSCOLOR      22  // Steps color
#define KEY_DISTCOLOR       23  // Distance color
#define KEY_SECONDSCOLOR    97  // Seconds color
#define KEY_COMPASSCOLOR    91  // Compass color
#define KEY_CALCOLOR        63  // Calories color
#define KEY_SLEEPCOLOR      66  // Sleep color
#define KEY_DEEPCOLOR       68  // Deep sleep color

// Health behind-color keys (for "falling behind" indicator)
#define KEY_STEPSBEHINDCOLOR 39
#define KEY_DISTBEHINDCOLOR  40
#define KEY_CALBEHINDCOLOR   64
#define KEY_SLEEPBEHINDCOLOR 67
#define KEY_DEEPBEHINDCOLOR  69
#define KEY_ACTIVEBEHINDCOLOR 77

// Configuration toggle keys
#define KEY_ENABLEADVANCED  13  // Enable advanced colors
#define KEY_SHOWSLEEP       6   // Show sleep data
#define KEY_USECELSIUS      9   // Use Celsius temperature
#define KEY_USEKM           5   // Use kilometers
#define KEY_BLUETOOTHDISCONNECT 29 // Vibrate on BT disconnect
#define KEY_LEADINGZERO     46  // Show leading zero for hours
#define KEY_SIMPLEMODE      48  // Simple display mode
#define KEY_QUICKVIEW       70  // Enable quick view
#define KEY_SHOWTAP         84  // Enable tap-to-show
#define KEY_SHOWWRIST       96  // Enable wrist-to-show
#define KEY_MUTEONQUIET     106 // Mute during do not disturb
#define KEY_UPDATE          33  // Disable update notifications
#define KEY_TIMEZONES       10  // Enable timezone display
#define KEY_FONTTYPE        26  // Selected font type
#define KEY_LOCALE          36  // Display language
#define KEY_DATEFORMAT      37  // Date format style
#define KEY_TEXTALIGN       38  // Text alignment
#define KEY_DATESEPARATOR   71  // Date separator character
#define KEY_TAPTIME         83  // Tap-to-show timeout (seconds)
#define KEY_OVERIDELOCATION 31  // Override location string

// Slot configuration keys (6 slots: A-F)
#define KEY_SLOTA       49  // Slot A module assignment
#define KEY_SLOTB       50  // Slot B module assignment
#define KEY_SLOTC       51  // Slot C module assignment
#define KEY_SLOTD       52  // Slot D module assignment
#define KEY_SLOTE       98  // Slot E module assignment
#define KEY_SLOTF       99  // Slot F module assignment

// Sleep mode slot keys (non-aplite)
#define KEY_SLEEPSLOTA  53
#define KEY_SLEEPSLOTB  54
#define KEY_SLEEPSLOTC  55
#define KEY_SLEEPSLOTD  56
#define KEY_SLEEPSLOTE  100
#define KEY_SLEEPSLOTF  101

// Tap mode slot keys (non-aplite)
#define KEY_TAPSLOTA    79
#define KEY_TAPSLOTB    80
#define KEY_TAPSLOTC    81
#define KEY_TAPSLOTD    82
#define KEY_TAPSLOTE    102
#define KEY_TAPSLOTF    103

// Wrist mode slot keys (non-aplite)
#define KEY_WRISTSLOTA  92
#define KEY_WRISTSLOTB  93
#define KEY_WRISTSLOTC  94
#define KEY_WRISTSLOTD  95
#define KEY_WRISTSLOTE  104
#define KEY_WRISTSLOTF  105

// Request/trigger keys
#define KEY_REQUESTWEATHER  119 // Request weather update
#define KEY_REQUESTCRYPTO   120 // Request crypto update
#define KEY_CONFIGS         45  // Configuration data
#define KEY_HASUPDATE       35  // Update available flag
#define KEY_ERROR           32  // Error message
#define KEY_UPDATE          33  // Update data

// =============================================================================
// Configuration Toggle Flags
// These are bitmask values that combine into a single config integer.
// =============================================================================

#define FLAG_WEATHER      0x0001  // Weather tracking enabled
#define FLAG_HEALTH       0x0002  // Health tracking enabled
#define FLAG_KM           0x0004  // Use kilometers instead of miles
#define FLAG_SLEEP        0x0008  // Show sleep data
#define FLAG_CELSIUS      0x0010  // Use Celsius temperature
#define FLAG_ADVANCED     0x0020  // Enable advanced color customization
#define FLAG_BLUETOOTH    0x0040  // Vibrate on Bluetooth disconnect
#define FLAG_UPDATE       0x0080  // Disable update notifications
#define FLAG_LEADINGZERO  0x0100  // Show leading zero for single-digit hours
#define FLAG_CALORIES     0x0200  // Show calories
#define FLAG_SIMPLEMODE   0x0400  // Simple display mode
#define FLAG_TIMEZONES    0x0800  // Enable timezone display
#define FLAG_QUICKVIEW    0x1000  // Enable quick view
#define FLAG_TAP          0x2000  // Enable tap-to-show feature
#define FLAG_WRIST        0x4000  // Enable wrist-to-show feature
#define FLAG_MUTEONQUIET  0x8000  // Mute notifications during quiet hours

// =============================================================================
// Font Type Constants
// Each font has a unique ID used in persist storage.
// =============================================================================

#define BLOCKO_FONT       0   // Blocko font (default)
#define BLOCKO_BIG_FONT   1   // Blocko large variant
#define SYSTEM_FONT       2   // Pebble system font
#define ARCHIVO_FONT      3   // Archivo Narrow font
#define DIN_FONT          4   // OSP-DIN font
#define PROTOTYPE_FONT    5   // Prototype font
#define LECO_FONT         6   // Leco font
#define KONSTRUCT_FONT    7   // Konstruct font

// =============================================================================
// Text Alignment Constants
// =============================================================================

#define ALIGN_LEFT        0   // Left-aligned text
#define ALIGN_CENTER      1   // Center-aligned text
#define ALIGN_RIGHT       2   // Right-aligned text (default for square screens)

// =============================================================================
// Locale (Language) Constants
// Language codes for weekday/month name localization.
// =============================================================================

#define LC_ENGLISH        0   // English (default)
#define LC_PORTUGUESE     1   // Portuguese
#define LC_FRENCH         2   // French
#define LC_GERMAN         3   // German
#define LC_SPANISH        4   // Spanish
#define LC_ITALIAN        5   // Italian
#define LC_DUTCH          6   // Dutch
#define LC_DANISH         7   // Danish
#define LC_TURKISH        8   // Turkish
#define LC_CZECH          9   // Czech
#define LC_POLISH         10  // Polish
#define LC_SWEDISH        11  // Swedish
#define LC_FINNISH        12  // Finnish
#define LC_SLOVAK         13  // Slovak

// =============================================================================
// Date Format Constants
// Controls how the date is displayed (order of day, month, weekday).
// =============================================================================

#define FORMAT_WMD        0   // Weekday, Month, Day (e.g., "Mon, January 15")
#define FORMAT_WDM        1   // Weekday, Day, Month
#define FORMAT_WD         2   // Weekday, Day
#define FORMAT_DM         3   // Day, Month
#define FORMAT_MD         4   // Month, Day
#define FORMAT_DSM        5   // Day, Separator, Month
#define FORMAT_SMD        6   // Separator, Month, Day
#define FORMAT_WDSM       7   // Weekday, Day, Separator, Month
#define FORMAT_WSMD       8   // Weekday, Separator, Month, Day
#define FORMAT_ISO        9   // ISO format (YYYY-MM-DD)
#define FORMAT_WNDSM      10  // Weekday, Number, Day, Separator, Month
#define FORMAT_WNSMD      11  // Weekday, Number, Separator, Month, Day

// =============================================================================
// Module Slot Constants
// Six physical display slots where modules can be placed.
// Slots A-D are on the sides; Slots E-F are in the center (round screens).
// =============================================================================

#define SLOT_A            0   // Slot A (top-left or top area)
#define SLOT_B            1   // Slot B (second position)
#define SLOT_C            2   // Slot C (third position)
#define SLOT_D            3   // Slot D (fourth position)
#define SLOT_E            4   // Slot E (center, round only)
#define SLOT_F            5   // Slot F (center, round only)

// =============================================================================
// Module Type Constants
// Each module represents a piece of data displayed on the watchface.
// MODULE_NONE = -1 means no module in a slot.
// =============================================================================

#define MODULE_NONE       -1  // Empty slot
#define MODULE_WEATHER    1   // Current weather icon + temperature
#define MODULE_FORECAST   2   // High/low temperature forecast
#define MODULE_STEPS      3   // Step count
#define MODULE_DIST       4   // Distance traveled
#define MODULE_CAL        5   // Calories burned
#define MODULE_SLEEP      6   // Total sleep duration
#define MODULE_DEEP       7   // Deep sleep duration
#define MODULE_WIND       8   // Wind speed + direction
#define MODULE_FEELS      9   // "Feels like" temperature
#define MODULE_WEATHER_FEELS 10 // Weather + feels like
#define MODULE_SUNRISE    11  // Sunrise time
#define MODULE_SUNSET     12  // Sunset time
#define MODULE_ACTIVE     13  // Active time (standing/walking)
#define MODULE_HEART      14  // Heart rate
#define MODULE_COMPASS    15  // Compass heading
#define MODULE_SECONDS    16  // Seconds indicator
#define MODULE_BATTERY    17  // Battery percentage
#define MODULE_TIMEZONE   18  // Alternative timezone (A)
#define MODULE_TIMEZONEB  19  // Alternative timezone (B)
#define MODULE_CRYPTO     20  // Cryptocurrency price
#define MODULE_CRYPTOB    21  // Second cryptocurrency price
#define MODULE_CRYPTOC    22  // Third cryptocurrency price
#define MODULE_CRYPTOD    23  // Fourth cryptocurrency price

// =============================================================================
// Display State Constants
// The watchface can display different content based on user interaction.
// =============================================================================

#define MODE_NORMAL       0   // Normal display mode
#define MODE_SIMPLE       1   // Simplified display mode

#define STATE_NORMAL      0   // Normal state (default slot config)
#define STATE_SLEEP       1   // Sleep state (different slot config)
#define STATE_TAP         2   // Tap-to-show state (different slot config)
#define STATE_WRIST       3   // Wrist-to-show state (different slot config)

// =============================================================================
// Display Item Constants
// Specific items within a slot (for positioning calculations).
// =============================================================================

#define WEATHER_ITEM      0   // Weather icon
#define TEMP_ITEM         1   // Temperature value
#define TEMPMIN_ITEM      2   // Minimum temperature
#define TEMPMAX_ITEM      3   // Maximum temperature
#define STEPS_ITEM        4   // Step count
#define DIST_ITEM         5   // Distance
#define CAL_ITEM          6   // Calories
#define SLEEP_ITEM        7   // Sleep duration
#define DEEP_ITEM         8   // Deep sleep duration
#define SPEED_ITEM        9   // Wind speed
#define DIRECTION_ITEM    10  // Wind direction
#define WIND_UNIT_ITEM    11  // Wind unit symbol
#define TEMPMINICON_ITEM  12  // Min temperature icon
#define TEMPMAXICON_ITEM  13  // Max temperature icon
#define SUNRISE_ITEM      14  // Sunrise time text
#define SUNSET_ITEM       15  // Sunset time text
#define SUNRISEICON_ITEM  16  // Sunrise icon
#define SUNSETICON_ITEM   17  // Sunset icon
#define ACTIVE_ITEM       18  // Active time
#define HEART_ITEM        19  // Heart rate value
#define HEARTICON_ITEM    20  // Heart icon
#define DEGREES_ITEM      21  // Compass degrees
#define COMPASS_ITEM      22  // Compass direction letters
#define SECONDS_ITEM      23  // Seconds indicator
#define BATTERY_ITEM      24  // Battery percentage
#define TIMEZONE_ITEM     25  // Timezone time
#define TIMEZONEB_ITEM    26  // Timezone B time
#define CRYPTO_ITEM       27  // Crypto price

// =============================================================================
// Wind Speed Unit Constants
// =============================================================================

#define UNIT_MPH          0   // Miles per hour
#define UNIT_KPH          1   // Kilometers per hour
#define UNIT_KNOTS        2   // Knots

// =============================================================================
// Reload Origin Constants
// Indicates why the screen is being reloaded.
// =============================================================================

#define RELOAD_DEFAULT    0   // Initial load on app start
#define RELOAD_CONFIGS    1   // Configuration changed (from companion)
#define RELOAD_REDRAW     2   // Visual redraw (e.g., unobstructed area changed)
#define RELOAD_MODULE     3   // Module data changed (e.g., new weather data)

#endif // __TIMEBOXED_KEYS_
