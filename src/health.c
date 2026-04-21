/**
 * health.c - Health data tracking implementation.
 *
 * This file handles:
 *   - Fetching steps, distance, calories, sleep, deep sleep, active time,
 *     and heart rate from the Pebble Health Service
 *   - Comparing current values against weekly averages to show "on track" or
 *     "falling behind" indicators
 *   - Detecting when the user is sleeping
 *   - Showing sleep data for 30 minutes after the user wakes up
 *   - Storing health data in persistent memory for restoration after reload
 *
 * Note: The Health Service is only available on Pebble Time, Time 2, Time Round,
 * and Time Steel. It is NOT available on the Pebble (original) or Pebble Lite.
 */

#include <pebble.h>
#include "keys.h"
#include "health.h"
#include "text.h"
#include "configs.h"
#include "screen.h"

#if defined(PBL_HEALTH)

// =============================================================================
// Internal State
// =============================================================================

static bool health_enabled;   // Whether health tracking is enabled
static int woke_up_at;        // Timestamp when user last woke up (0 = not applicable)
static bool was_asleep;       // Whether user was previously asleep
static bool sleep_data_visible; // Whether sleep data should be shown
static bool sleep_data_enabled; // Whether sleep data display is enabled
static bool useKm;            // Whether to use kilometers (true) or miles (false)
static bool useCalories;      // Whether to use calorie display format
static bool update_queued;    // Whether a health update is pending
static bool is_sleeping;      // Whether user is currently sleeping
static bool sleep_status_updated; // Whether sleep status has been checked this cycle

// Buffers for health data text
static char steps_text[8];
static char cal_text[10];
static char dist_text[10];
static char sleep_text[8];
static char deep_text[8];
static char active_text[8];
static char heart_text[10];

// =============================================================================
// Helper Functions
// =============================================================================

/** Clear all health data displays */
static void clear_health_fields(void) {
    set_steps_layer_text("");
    set_dist_layer_text("");
    set_cal_layer_text("");
    set_sleep_layer_text("");
    set_deep_layer_text("");
    set_active_layer_text("");
    set_heart_layer_text("");
    set_heart_icon_layer_text("");
}

/** Check if the user has granted health data permissions */
static bool health_permission_granted(void) {
    if (!health_enabled) {
        return false;
    }
    HealthMetric metric_steps = HealthMetricStepCount;
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    HealthServiceAccessibilityMask mask_steps =
        health_service_metric_accessible(metric_steps, start, end);
    return !(mask_steps & HealthServiceAccessibilityMaskNoPermission);
}

// =============================================================================
// Health Data Fetching
// Each data type has its own function that:
//   1. Checks if the module is enabled
//   2. Queries the Health Service for the current value
//   3. Compares against the weekly average
//   4. Formats and displays the result
// =============================================================================

static void get_steps_data(void) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    int one_day = 24 * SECONDS_PER_HOUR;
    int current_steps = 0;
    int steps_last_week = 0;

    HealthMetric metric_steps = HealthMetricStepCount;
    HealthServiceAccessibilityMask mask_steps =
        health_service_metric_accessible(metric_steps, start, end);

    if (mask_steps & HealthServiceAccessibilityMaskAvailable) {
        // Get today's step count
        current_steps = (int)health_service_sum_today(metric_steps);

        // Get weekly average (use averaged data if available, otherwise calculate from past weeks)
        HealthServiceAccessibilityMask mask_steps_average =
            health_service_metric_averaged_accessible(metric_steps, start, end,
                                                        HealthServiceTimeScopeDailyWeekdayOrWeekend);

        if (mask_steps_average & HealthServiceAccessibilityMaskAvailable) {
            steps_last_week = (int)health_service_sum_averaged(
                metric_steps, start, end, HealthServiceTimeScopeDailyWeekdayOrWeekend);
        } else {
            // Fall back to averaging the past 4 weeks
            for (int i = 7; i <= 28; i = i + 7) {
                steps_last_week += (int)health_service_sum(
                    metric_steps, start - i * one_day, end - i * one_day);
            }
            steps_last_week /= 4;
        }

        // Format and display step count
        snprintf(steps_text, sizeof(steps_text), "%d", current_steps);
        set_steps_layer_text(steps_text);

        // Color changes if falling behind pace (compare to weekly average)
        set_progress_color_steps(current_steps < steps_last_week);
    }
}

static void get_dist_data(void) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    int one_day = 24 * SECONDS_PER_HOUR;

    int current_dist = 0;
    int dist_last_week = 0;
    int current_dist_int = 0;
    int current_dist_dec = 0;

    HealthMetric metric_dist = HealthMetricWalkedDistanceMeters;
    HealthServiceAccessibilityMask mask_dist =
        health_service_metric_accessible(metric_dist, start, end);

    if (mask_dist & HealthServiceAccessibilityMaskAvailable) {
        current_dist = (int)health_service_sum_today(metric_dist);

        HealthServiceAccessibilityMask mask_dist_average =
            health_service_metric_averaged_accessible(metric_dist, start, end,
                                                        HealthServiceTimeScopeDailyWeekdayOrWeekend);

        if (mask_dist_average & HealthServiceAccessibilityMaskAvailable) {
            dist_last_week = (int)health_service_sum_averaged(
                metric_dist, start, end, HealthServiceTimeScopeDailyWeekdayOrWeekend);
        } else {
            for (int i = 7; i <= 28; i = i + 7) {
                dist_last_week += (int)health_service_sum(
                    metric_dist, start - i * one_day, end - i * one_day);
            }
            dist_last_week /= 4;
        }

        // Convert meters to miles or kilometers based on user preference
        if (!useKm) {
            current_dist /= 1.6; // Convert meters to miles (approximate)
        }
        current_dist_int = current_dist / 1000;
        current_dist_dec = (current_dist % 1000) / 100;

        // Format: "X.Xmi" or "X.XXkm"
        snprintf(dist_text, sizeof(dist_text), (useKm ? "%d.%dkm" : "%d.%dmi"),
                 current_dist_int, current_dist_dec);

        set_dist_layer_text(dist_text);
        set_progress_color_dist(current_dist < dist_last_week);
    }
}

static void get_cal_data(void) {
    HealthMetric metric_cal_rest = HealthMetricRestingKCalories;
    HealthMetric metric_cal_act = HealthMetricActiveKCalories;

    time_t start = time_start_of_today();
    time_t end = time(NULL);
    int one_day = 24 * SECONDS_PER_HOUR;

    int current_cal = 0;
    int cal_last_week = 0;

    HealthServiceAccessibilityMask mask_cal_rest =
        health_service_metric_accessible(metric_cal_rest, start, end);
    HealthServiceAccessibilityMask mask_cal_act =
        health_service_metric_accessible(metric_cal_act, start, end);

    bool has_cal_metric = (mask_cal_rest & HealthServiceAccessibilityMaskAvailable) ||
                          (mask_cal_act & HealthServiceAccessibilityMaskAvailable);

    if (has_cal_metric) {
        current_cal = (int)health_service_sum_today(metric_cal_rest) +
                      (int)health_service_sum_today(metric_cal_act);

        HealthServiceAccessibilityMask mask_cal_rest_average =
            health_service_metric_averaged_accessible(metric_cal_rest, start, end,
                                                        HealthServiceTimeScopeDailyWeekdayOrWeekend);
        HealthServiceAccessibilityMask mask_cal_act_average =
            health_service_metric_averaged_accessible(metric_cal_act, start, end,
                                                        HealthServiceTimeScopeDailyWeekdayOrWeekend);

        if ((mask_cal_rest_average & HealthServiceAccessibilityMaskAvailable) ||
            (mask_cal_act_average & HealthServiceAccessibilityMaskAvailable)) {
            cal_last_week += (int)health_service_sum_averaged(
                metric_cal_rest, start, end, HealthServiceTimeScopeDailyWeekdayOrWeekend);
            cal_last_week += (int)health_service_sum_averaged(
                metric_cal_act, start, end, HealthServiceTimeScopeDailyWeekdayOrWeekend);
        } else {
            for (int i = 7; i <= 28; i = i + 7) {
                cal_last_week += (int)health_service_sum(
                    metric_cal_rest, start - i * one_day, end - i * one_day);
                cal_last_week += (int)health_service_sum(
                    metric_cal_act, start - i * one_day, end - i * one_day);
            }
            cal_last_week /= 4;
        }

        // Format calorie display
        snprintf(cal_text, sizeof(cal_text),
                 (get_loaded_font() == KONSTRUCT_FONT && current_cal >= 2000) ?
                     "%dcal" : "%d cal", current_cal);

        set_cal_layer_text(cal_text);
        set_progress_color_cal(current_cal < cal_last_week);
    }
}

static void get_sleep_data(void) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    int one_day = 24 * SECONDS_PER_HOUR;

    int current_sleep = 0;
    int sleep_last_week = 0;

    HealthMetric metric_sleep = HealthMetricSleepSeconds;
    HealthServiceAccessibilityMask mask_sleep =
        health_service_metric_accessible(metric_sleep, start, end);
    HealthServiceAccessibilityMask mask_sleep_average =
        health_service_metric_averaged_accessible(metric_sleep, start, end,
                                                    HealthServiceTimeScopeDailyWeekdayOrWeekend);

    if (mask_sleep & HealthServiceAccessibilityMaskAvailable) {
        current_sleep = (int)health_service_sum_today(metric_sleep);

        if (mask_sleep_average & HealthServiceAccessibilityMaskAvailable) {
            sleep_last_week = (int)health_service_sum_averaged(
                metric_sleep, start, start + 24 * SECONDS_PER_HOUR - 1,
                HealthServiceTimeScopeDailyWeekdayOrWeekend);
        } else {
            for (int i = 1; i <= 7; i++) {
                sleep_last_week += (int)health_service_sum(
                    metric_sleep, start - i * one_day, start - (i - 1) * one_day);
            }
            sleep_last_week /= 7;
        }

        // Format sleep duration as "XhYYm"
        int current_sleep_hours = current_sleep / SECONDS_PER_HOUR;
        int current_sleep_minutes = ((current_sleep - (current_sleep_hours * SECONDS_PER_HOUR)) /
                                      SECONDS_PER_MINUTE) % SECONDS_PER_MINUTE;

        snprintf(sleep_text, sizeof(sleep_text), "%dh%02dm",
                 current_sleep_hours, current_sleep_minutes);

        set_sleep_layer_text(sleep_text);
        set_progress_color_sleep(current_sleep < sleep_last_week);
    }
}

static void get_deep_data(void) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    int one_day = 24 * SECONDS_PER_HOUR;

    int current_deep = 0;
    int deep_last_week = 0;

    HealthMetric metric_deep = HealthMetricSleepRestfulSeconds;
    HealthServiceAccessibilityMask mask_deep =
        health_service_metric_accessible(metric_deep, start, end);
    HealthServiceAccessibilityMask mask_deep_average =
        health_service_metric_averaged_accessible(metric_deep, start, end,
                                                    HealthServiceTimeScopeDailyWeekdayOrWeekend);

    if (mask_deep & HealthServiceAccessibilityMaskAvailable) {
        current_deep = (int)health_service_sum_today(metric_deep);

        if (mask_deep_average & HealthServiceAccessibilityMaskAvailable) {
            deep_last_week = (int)health_service_sum_averaged(
                metric_deep, start, start + 24 * SECONDS_PER_HOUR - 1,
                HealthServiceTimeScopeDailyWeekdayOrWeekend);
        } else {
            for (int i = 1; i <= 7; i++) {
                deep_last_week += (int)health_service_sum(
                    metric_deep, start - i * one_day, start - (i - 1) * one_day);
            }
            deep_last_week /= 7;
        }

        int current_deep_hours = current_deep / SECONDS_PER_HOUR;
        int current_deep_minutes = ((current_deep - (current_deep_hours * SECONDS_PER_HOUR)) /
                                      SECONDS_PER_MINUTE) % SECONDS_PER_MINUTE;

        snprintf(deep_text, sizeof(deep_text), "%dh%02dm",
                 current_deep_hours, current_deep_minutes);

        set_deep_layer_text(deep_text);
        set_progress_color_deep(current_deep < deep_last_week);
    }
}

static void get_active_data(void) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    int one_day = 24 * SECONDS_PER_HOUR;

    int current_active = 0;
    int active_last_week = 0;

    HealthMetric metric_active = HealthMetricActiveSeconds;
    HealthServiceAccessibilityMask mask_active =
        health_service_metric_accessible(metric_active, start, end);
    HealthServiceAccessibilityMask mask_active_average =
        health_service_metric_averaged_accessible(metric_active, start, end,
                                                    HealthServiceTimeScopeDailyWeekdayOrWeekend);

    if (mask_active & HealthServiceAccessibilityMaskAvailable) {
        current_active = (int)health_service_sum_today(metric_active);

        if (mask_active_average & HealthServiceAccessibilityMaskAvailable) {
            active_last_week = (int)health_service_sum_averaged(
                metric_active, start, start + 24 * SECONDS_PER_HOUR - 1,
                HealthServiceTimeScopeDailyWeekdayOrWeekend);
        } else {
            for (int i = 1; i <= 7; i++) {
                active_last_week += (int)health_service_sum(
                    metric_active, start - i * one_day, start - (i - 1) * one_day);
            }
            active_last_week /= 7;
        }

        int current_active_hours = current_active / SECONDS_PER_HOUR;
        int current_active_minutes = ((current_active - (current_active_hours * SECONDS_PER_HOUR)) /
                                        SECONDS_PER_MINUTE) % SECONDS_PER_MINUTE;

        snprintf(active_text, sizeof(active_text), "%dh%02dm",
                 current_active_hours, current_active_minutes);

        set_active_layer_text(active_text);
        set_progress_color_active(current_active < active_last_week);
    }
}

static void get_heart_data(void) {
    int current_heart = 0;
    HealthMetric metric_heart = HealthMetricHeartRateBPM;

    // Heart rate is peeked from the latest reading (not summed)
    current_heart = (int)health_service_peek_current_value(metric_heart);

    snprintf(heart_text, sizeof(heart_text), "%d", current_heart);
    set_heart_layer_text(heart_text);
    set_heart_icon_layer_text("v"); // FontAwesome heart icon
    set_progress_color_heart(current_heart);
}

// =============================================================================
// Health Data Management
// =============================================================================

void queue_health_update(void) {
    update_queued = true;
}

void get_health_data(void) {
    if (health_enabled && update_queued) {
        update_queued = false;

        // Fetch each enabled module's data
        if (is_module_enabled(MODULE_STEPS)) {
            get_steps_data();
        }
        if (is_module_enabled(MODULE_DIST)) {
            get_dist_data();
        }
        if (is_module_enabled(MODULE_CAL)) {
            get_cal_data();
        }
        if (is_module_enabled(MODULE_SLEEP)) {
            get_sleep_data();
        }
        if (is_module_enabled(MODULE_DEEP)) {
            get_deep_data();
        }
        if (is_module_enabled(MODULE_ACTIVE)) {
            get_active_data();
        }
        if (is_module_enabled(MODULE_HEART)) {
            get_heart_data();
        }
    }
}

/** Health Service event handler - triggers data updates */
void health_handler(HealthEventType event, void *context) {
    switch (event) {
        case HealthEventSignificantUpdate:
        case HealthEventMovementUpdate:
        case HealthEventSleepUpdate:
        case HealthEventMetricAlert:
        case HealthEventHeartRateUpdate:
            queue_health_update();
            break;
    }
}

static void load_health_data_from_storage(void) {
    // Restore health data from persistent storage after reload
    if (is_module_enabled(MODULE_STEPS)) {
        persist_read_string(KEY_STEPS, steps_text, sizeof(steps_text));
        set_steps_layer_text(steps_text);
        set_progress_color_steps(false);
    }
    if (is_module_enabled(MODULE_DIST)) {
        persist_read_string(KEY_DIST, dist_text, sizeof(dist_text));
        set_dist_layer_text(dist_text);
        set_progress_color_dist(false);
    }
    if (is_module_enabled(MODULE_CAL)) {
        persist_read_string(KEY_CAL, cal_text, sizeof(cal_text));
        set_cal_layer_text(cal_text);
        set_progress_color_cal(false);
    }
    if (is_module_enabled(MODULE_SLEEP)) {
        persist_read_string(KEY_SLEEP, sleep_text, sizeof(sleep_text));
        set_sleep_layer_text(sleep_text);
        set_progress_color_sleep(false);
    }
    if (is_module_enabled(MODULE_DEEP)) {
        persist_read_string(KEY_DEEP, deep_text, sizeof(deep_text));
        set_deep_layer_text(deep_text);
        set_progress_color_deep(false);
    }
    if (is_module_enabled(MODULE_ACTIVE)) {
        persist_read_string(KEY_ACTIVE, active_text, sizeof(active_text));
        set_active_layer_text(active_text);
        set_progress_color_active(false);
    }
    if (is_module_enabled(MODULE_HEART)) {
        set_heart_layer_text("0");
        set_heart_icon_layer_text("v");
        set_progress_color_heart(0);
    }
}

static bool get_health_enabled(void) {
    return is_health_toggle_enabled() ||
        is_module_enabled(MODULE_STEPS) ||
        is_module_enabled(MODULE_DIST) ||
        is_module_enabled(MODULE_CAL) ||
        is_module_enabled(MODULE_SLEEP) ||
        is_module_enabled(MODULE_DEEP) ||
        is_module_enabled(MODULE_ACTIVE) ||
        is_module_enabled(MODULE_HEART);
}

void toggle_health(uint8_t reload_origin) {
    is_sleeping = false;
    health_enabled = get_health_enabled();
    sleep_data_enabled = is_sleep_data_enabled();

    if (health_enabled) {
        // Determine distance unit system
        MeasurementSystem distMeasure = health_service_get_measurement_system_for_display(
            HealthMetricWalkedDistanceMeters);
        if (distMeasure != MeasurementSystemUnknown) {
            useKm = distMeasure == MeasurementSystemMetric;
        } else {
            useKm = is_use_km_enabled();
        }

        useCalories = is_use_calories_enabled();

        if (health_permission_granted()) {
            bool has_health = health_service_events_subscribe(health_handler, NULL);
            if (has_health) {
                clear_health_fields();
                queue_health_update();
                if (reload_origin != RELOAD_DEFAULT) {
                    get_health_data();
                } else {
                    load_health_data_from_storage();
                }
            } else {
                health_service_events_unsubscribe();
            }
        } else {
            health_enabled = false;
        }
    }

    if (!health_enabled) {
        clear_health_fields();
        health_service_events_unsubscribe();
    }
}

bool is_user_sleeping(void) {
    if (!health_enabled) {
        return false;
    }
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Only check sleep status every 10 minutes to save battery
    if (tick_time->tm_min % 10 == 0) {
        if (!sleep_status_updated) {
            sleep_status_updated = true;
            HealthActivityMask activities = health_service_peek_current_activities();
            is_sleeping = activities & HealthActivitySleep ||
                          activities & HealthActivityRestfulSleep;
        }
    } else {
        sleep_status_updated = false;
    }
    return is_sleeping;
}

void show_sleep_data_if_visible(struct Window *watchface) {
    if (health_enabled && sleep_data_enabled) {
        if (is_user_sleeping()) {
            sleep_data_visible = true;
            if (!was_asleep) {
                was_asleep = true;
                woke_up_at = 0;
                queue_health_update();
                redraw_screen(watchface);
            }
        }

        if (!is_user_sleeping() && was_asleep) {
            sleep_data_visible = true;
            woke_up_at = time(NULL) + SECONDS_PER_MINUTE * 30; // Show for 30 minutes
            was_asleep = false;
            queue_health_update();
        }

        // Hide sleep data 30 minutes after waking up
        if (sleep_data_visible && woke_up_at > 0 && time(NULL) > woke_up_at) {
            sleep_data_visible = false;
            redraw_screen(watchface);
            queue_health_update();
        }
    }
}

void init_sleep_data(void) {
    was_asleep = false;
    sleep_data_visible = false;
}

void save_health_data_to_storage(void) {
    persist_write_string(KEY_STEPS, steps_text);
    persist_write_string(KEY_DIST, dist_text);
    persist_write_string(KEY_CAL, cal_text);
    persist_write_string(KEY_SLEEP, sleep_text);
    persist_write_string(KEY_DEEP, deep_text);
    persist_write_string(KEY_ACTIVE, active_text);
}

bool should_show_sleep_data(void) {
    return sleep_data_visible;
}

#else // Health not available on this device

bool is_user_sleeping(void) {
    return false;
}

bool should_show_sleep_data(void) {
    return false;
}

#endif
