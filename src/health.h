/**
 * health.h - Health data tracking interface.
 *
 * Provides functions to:
 *   - Enable/disable health data tracking
 *   - Fetch health data (steps, distance, calories, sleep, heart rate)
 *   - Queue health data updates
 *   - Detect if the user is sleeping
 *   - Show sleep data after the user wakes up
 */

#ifndef __TIMEBOXED_HEALTH_
#define __TIMEBOXED_HEALTH_

#include <pebble.h>

// Health service functions (only available on devices with health sensors)
#if defined PBL_HEALTH
/** Enable or disable health tracking */
void toggle_health(uint8_t);

/** Fetch current health data from the Health Service */
void get_health_data(void);

/** Queue a health data update (will be processed on next tick) */
void queue_health_update(void);

/** Show sleep data if the user recently woke up */
void show_sleep_data_if_visible(struct Window *watchface);

/** Initialize sleep tracking state */
void init_sleep_data(void);

/** Save health data to persistent storage */
void save_health_data_to_storage(void);
#endif

/** Check if the user is currently sleeping */
bool is_user_sleeping(void);

/** Check if sleep data should be shown (after user wakes up) */
bool should_show_sleep_data(void);

#endif // __TIMEBOXED_HEALTH_
