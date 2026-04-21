/**
 * screen.h - Screen service handlers for the Timeboxed watchface.
 *
 * Provides callbacks for:
 *   - Battery status changes (updates battery percentage display)
 *   - Bluetooth connection changes (shows/hides Bluetooth icon)
 *   - Time ticks (updates displayed time)
 *   - Update checks (checks for watchface updates)
 *   - Notifications (handles update available notifications)
 *   - Screen unobstructed area changes (round screens when keyboard is shown)
 */

#ifndef __TIMEBOXED_SCREEN_
#define __TIMEBOXED_SCREEN_

#include <pebble.h>

/**
 * Load the screen with current configuration.
 * @param reload_origin  Why we're reloading (RELOAD_DEFAULT, RELOAD_CONFIGS, etc.)
 * @param watchface      The watchface window
 */
void load_screen(uint8_t reload_origin, struct Window *watchface);

/**
 * Redraw the screen with current data.
 * @param watchface  The watchface window
 */
void redraw_screen(struct Window *watchface);

/**
 * Reload fonts from the user's selected font preference.
 */
void reload_fonts(void);

/**
 * Recreate all text layers with new positions (called when screen layout changes).
 * @param watchface  The watchface window
 */
void recreate_text_layers(struct Window *watchface);

/**
 * Bluetooth connection state callback.
 * @param connected  true if Bluetooth is connected, false if disconnected
 */
void bt_handler(bool connected);

/**
 * Battery charge state callback.
 * @param battery_state  Current battery charge state
 */
void battery_handler(struct BatteryChargeState battery_state);

/**
 * Update the displayed time (called every minute).
 */
void update_time(void);

/**
 * Check for watchface updates (called daily at 4:00 AM).
 */
void check_for_updates(void);

/**
 * Notify about an available update.
 * @param update_available  true if an update is available
 */
void notify_update(int update_available);

/**
 * Load timezone data from persistent storage.
 */
void load_timezone(void);

#endif // __TIMEBOXED_SCREEN_
