/**
 * compass.c - Compass heading display implementation.
 *
 * Listens to the Pebble compass service and updates the display with
 * the current magnetic heading (direction the watch is pointing).
 *
 * The compass only works when:
 *   - The device has a compass sensor
 *   - The user has calibrated the compass
 *   - The compass module is enabled in the watchface configuration
 */

#include <pebble.h>
#include "screen.h"
#include "configs.h"
#include "keys.h"
#include "text.h"
#include "weather.h"

#if defined PBL_COMPASS

/**
 * Compass heading data handler - called whenever the compass reports new data.
 * @param data  The compass heading data (magnetic heading, status)
 */
void compass_handler(struct CompassHeadingData data) {
    CompassStatus status = data.compass_status;
    int heading;
    char degrees[6];
    char compass[2];

    switch (status) {
        case CompassStatusCalibrated:
        case CompassStatusCalibrating:
            // Convert the magnetic heading to degrees (0-360)
            // TRIGANGLE_TO_DEG converts Pebble's internal angle format to degrees
            // We subtract from TRIG_MAX_ANGLE because Pebble's angle increases clockwise
            // from North, but we want standard bearing (0 = North, 90 = East, etc.)
            heading = TRIGANGLE_TO_DEG(TRIG_MAX_ANGLE - (int)data.magnetic_heading);

            // Get the cardinal direction letter (N, NE, E, etc.)
            strcpy(compass, get_wind_direction(
                (TRIGANGLE_TO_DEG((int)data.magnetic_heading) + 180) % 360));

            // Get the degree string
            snprintf(degrees, sizeof(degrees), "%s",
                     get_wind_direction_text((heading + 180) % 360));

            // Update the display layers
            set_compass_layer_text(compass);
            set_degrees_layer_text(degrees);
            break;

        case CompassStatusUnavailable:
        case CompassStatusDataInvalid:
            // Compass is unavailable or data is invalid
            set_compass_layer_text("N");
            set_degrees_layer_text("NA");
            break;
    }
}

/**
 * Initialize the compass service.
 * Subscribes to compass data if the compass module is enabled.
 * @param watchface  The watchface window
 */
void init_compass_service(struct Window *watchface) {
    if (is_module_enabled(MODULE_COMPASS)) {
        // Subscribe to compass heading updates with a filter to reduce noise
        // TRIG_MAX_ANGLE/64 means only report changes of at least ~5.6 degrees
        compass_service_subscribe((CompassHeadingHandler)compass_handler);
        compass_service_set_heading_filter(TRIG_MAX_ANGLE / 64);
    } else {
        compass_service_unsubscribe();
    }
}

#endif // PBL_COMPASS
