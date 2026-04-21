/**
 * compass.h - Compass heading display interface.
 *
 * Provides functions to initialize the compass service for displaying
 * the watch's magnetic heading. Only available on devices with a compass
 * (Pebble Time Round, Pebble Time Steel).
 */

#ifndef __TIMEBOXED_COMPASS
#define __TIMEBOXED_COMPASS

#if defined PBL_COMPASS
/** Register the compass data handler */
void compass_handler(struct CompassHeadingData heading);

/** Initialize the compass service (subscribe to compass data) */
void init_compass_service(struct Window *watchface);
#endif

#endif // __TIMEBOXED_COMPASS
