/**
 * accel.c - Accelerometer-based gesture detection implementation.
 *
 * This file handles two gesture-based features:
 *
 * 1. Tap-to-show: When the user taps the watch face, extra information
 *    (like health data, weather, etc.) is displayed for a set timeout period.
 *    This is detected by analyzing Z-axis accelerometer data for a specific
 *    pattern: rest -> upward tap -> rest -> downward tap -> rest.
 *
 * 2. Wrist-to-show: When the user raises their wrist, extra information
 *    is displayed. This is detected by the accelerometer tap service
 *    detecting motion on the Y-axis (wrist raise).
 *
 * These features are disabled on APlite (Pebble original) due to limited
 * processing power.
 */

#include <pebble.h>
#include "screen.h"
#include "configs.h"
#include "keys.h"
#include "text.h"

#if !defined PBL_PLATFORM_APLITE

// =============================================================================
// Internal State
// =============================================================================

static bool initialized;       // Whether accelerometer has been initialized
static int lastPassX;          // Last filtered X acceleration
static int lastPassY;          // Last filtered Y acceleration
static int lastPassZ;          // Last filtered Z acceleration
static int lastX;              // Last raw X acceleration
static int lastY;              // Last raw Y acceleration
static int lastZ;              // Last raw Z acceleration

// Tap detection state machine
static bool begin_tap;         // First phase of tap detected (upward motion)
static bool mid_tap;           // Second phase (at peak of tap)
static bool end_tap;           // Third phase (return to rest)
static int mid_tap_count;      // Samples at peak (must reach threshold)
static int end_tap_count;      // Samples returning to rest (must reach threshold)
static bool neg;               // Whether the tap was downward (negative Z)

// Display state
static bool show_tap_mode;     // Whether tap-to-show mode is active
static bool show_wrist_mode;   // Whether wrist-to-show mode is active
static int timeout_sec = 0;    // How long to show extra info (seconds)

// Window reference for redrawing
static struct Window *watchface_ref;

// =============================================================================
// Tap Detection Helpers
// =============================================================================

/** Reset the tap detection state machine */
static void reset_tap(void) {
    begin_tap = false;
    mid_tap = false;
    end_tap = false;
    end_tap_count = 0;
    mid_tap_count = 0;
}

void reset_tap_handler(void) {
    show_tap_mode = false;
    redraw_screen(watchface_ref);
}

// =============================================================================
// Accelerometer Data Handler
// Analyzes raw accelerometer samples to detect tap gestures.
//
// Tap detection works by looking for a specific pattern in the Z-axis:
//   1. Rest: Z is near 0 (watch lying flat)
//   2. Upward motion: Z goes negative (watch lifted)
//   3. Peak: Z reaches a threshold (watch at highest point)
//   4. Downward motion: Z returns toward 0
//   5. Rest: Z is near 0 again (watch placed back down)
//
// The pattern must complete within a short time window to be considered a tap.
// =============================================================================

void accel_data_handler(AccelData *data, uint32_t num_samples) {
    // If tap-to-show is already active, don't process more data
    if (show_tap_mode) {
        return;
    }

    // Filtered and raw acceleration values for two consecutive samples
    int passX[2];
    int passY[2];
    int passZ[2];
    int Z[2];
    int X[2];
    int Y[2];

    if (!initialized) {
        // First call: initialize with the first sample
        passX[0] = passX[1] = data[0].x;
        passY[0] = passY[1] = data[0].y;
        passZ[0] = passZ[1] = data[0].z;
        Z[0] = Z[1] = data[0].z;
        X[0] = X[1] = data[0].x;
        Y[0] = Y[1] = data[0].y;
        initialized = true;
        show_tap_mode = false;
        reset_tap();
    } else {
        // Use previous sample values
        passX[0] = passX[1] = lastPassX;
        passY[0] = passY[1] = lastPassY;
        passZ[0] = passZ[1] = lastPassZ;
        X[0] = X[1] = lastX;
        Y[0] = Y[1] = lastY;
        Z[0] = Z[1] = lastZ;
    }

    // Detection thresholds and parameters
    int factor = 4;              // Smoothing factor for filtered values
    int high_threshold = 40;     // Z acceleration threshold to detect "lift"
    int low_threshold = 10;      // Z threshold to detect "rest"
    int threshold_other_axis = 15; // Max X/Y movement allowed during rest
    int range = 2;               // Number of samples needed at each phase
    int x_threshold = 350;       // Max X acceleration for "still" detection
    int y_threshold = 150;       // Max Y acceleration for "still" detection

    for (int i = 0; i < (int)num_samples; ++i) {
        // Skip samples from the tap button vibration (not a gesture)
        if (data[i].did_vibrate) {
            reset_tap();
            continue;
        }

        Z[1] = data[i].z;
        X[1] = data[i].x;
        Y[1] = data[i].y;

        // Calculate filtered (smoothed) acceleration values
        passX[1] = (int)((passX[0] + X[1] - X[0]) / factor);
        passY[1] = (int)((passY[0] + Y[1] - Y[0]) / factor);
        passZ[1] = (int)((passZ[0] + Z[1] - Z[0]) / factor);

        // Phase 3: Check if tap is ending (returning to rest)
        if (!end_tap && mid_tap) {
            if (end_tap_count < 3 * range) {
                end_tap_count++;
                if (abs(passZ[1]) <= abs(low_threshold)) {
                    end_tap = true;
                }
            } else {
                reset_tap();
            }
        }

        // Phase 2: Check if tap is at peak
        if (!mid_tap && begin_tap) {
            if (mid_tap_count < range) {
                mid_tap_count++;
                if ((neg && passZ[1] >= high_threshold) ||
                    (!neg && passZ[1] <= -1 * high_threshold)) {
                    mid_tap = true;
                }
            } else {
                reset_tap();
            }
        }

        // Phase 1: Check if tap is beginning (watch lifted from rest)
        // Conditions:
        //   - Currently at rest (Z near 0, X and Y near 0)
        //   - Z acceleration exceeds threshold (watch being lifted)
        //   - Not already in a tap sequence
        if ((abs(passZ[0]) <= low_threshold) &&
            ((abs(X[0]) <= x_threshold && Y[0] <= y_threshold)) &&
            ((abs(passX[0]) <= threshold_other_axis &&
              abs(passY[0]) <= threshold_other_axis)) &&
            abs(passZ[1]) >= high_threshold &&
            !begin_tap) {
            begin_tap = true;
            neg = passZ[1] < 0; // Remember direction of tap
        }

        // Complete tap detected! Enable tap-to-show mode
        if (begin_tap && mid_tap && end_tap) {
            show_tap_mode = true;
            reset_tap();
            redraw_screen(watchface_ref);
        }

        // Update previous values for next iteration
        passX[0] = passX[1];
        passY[0] = passY[1];
        passZ[0] = passZ[1];
        Z[0] = Z[1];
        X[0] = X[1];
        Y[0] = Y[1];
    }

    // Store current values for next call
    lastPassX = passX[1];
    lastPassY = passY[1];
    lastPassZ = passZ[1];
    lastX = X[1];
    lastY = Y[1];
    lastZ = Z[1];
}

bool tap_mode_visible(void) {
    return show_tap_mode && !show_wrist_mode;
}

bool wrist_mode_visible(void) {
    return show_wrist_mode && !show_tap_mode;
}

void reset_wrist_handler(void) {
    show_wrist_mode = false;
    redraw_screen(watchface_ref);
}

/**
 * Wrist raise detection handler.
 * Called when the accelerometer tap service detects motion on the Y-axis.
 * This corresponds to the user raising their wrist to view the watch.
 */
void shake_data_handler(AccelAxisType axis, int32_t direction) {
    if (show_wrist_mode) {
        return;
    }

    if (axis == ACCEL_AXIS_Y) {
        show_wrist_mode = true;
        redraw_screen(watchface_ref);
    }
}

/**
 * Initialize the accelerometer service.
 * Subscribes to accelerometer data and tap detection if the features are enabled.
 * @param watchface  The watchface window
 */
void init_accel_service(struct Window *watchface) {
    timeout_sec = persist_exists(KEY_TAPTIME) ?
        persist_read_int(KEY_TAPTIME) : 7; // Default 7 second timeout
    watchface_ref = watchface;

#if !defined PBL_PLATFORM_APLITE
    // Unsubscribe from any previous subscription
    accel_data_service_unsubscribe();
    if (is_tap_enabled()) {
        // Subscribe to raw accelerometer data (25Hz sampling rate)
        accel_data_service_subscribe(25, accel_data_handler);
    }
#endif

    // Unsubscribe from previous tap service
    accel_tap_service_unsubscribe();
    if (is_wrist_enabled()) {
        // Subscribe to tap detection (for wrist raise)
        accel_tap_service_subscribe(shake_data_handler);
    }
}

#else // APlite: no accelerometer gesture detection

bool tap_mode_visible(void) {
    return false;
}

bool wrist_mode_visible(void) {
    return false;
}

#endif
