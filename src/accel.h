/**
 * accel.h - Accelerometer-based gesture detection interface.
 *
 * Provides functions to:
 *   - Detect taps on the watch (tap-to-show extra information)
 *   - Detect wrist raises (wrist-to-show extra information)
 *   - Manage the timeout for these gestures
 */

#ifndef __TIMEBOXED_ACCEL_
#define __TIMEBOXED_ACCEL_

#include <pebble.h>

#if !defined PBL_PLATFORM_APLITE

/**
 * Accelerometer data handler - processes raw accelerometer samples.
 * Detects taps based on Z-axis acceleration patterns.
 * @param data       Array of accelerometer samples
 * @param num_samples  Number of samples in the array
 */
void accel_data_handler(AccelData *data, uint32_t num_samples);

/** Reset the tap-to-show state (called when timeout expires) */
void reset_tap_handler(void);

/** Initialize the accelerometer service (subscribe to accelerometer data) */
void init_accel_service(struct Window *watchface);

/** Reset the wrist-to-show state (called when timeout expires) */
void reset_wrist_handler(void);

/**
 * Shake/tilt data handler - detects wrist raises.
 * @param axis   The accelerometer axis that detected the motion
 * @param direction  The direction of motion (1 or -1)
 */
void shake_data_handler(AccelAxisType axis, int32_t direction);
#endif

/** Check if tap-to-show is currently active */
bool tap_mode_visible(void);

/** Check if wrist-to-show is currently active */
bool wrist_mode_visible(void);

#endif // __TIMEBOXED_ACCEL_
