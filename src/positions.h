/**
 * positions.h - Layout position calculations for display elements.
 *
 * Calculates where text layers and icons should be placed on screen
 * based on font type, screen dimensions, alignment, and display state.
 */

#ifndef __TIMEBOXED_POSITIONS_
#define __TIMEBOXED_POSITIONS_

#include <pebble.h>

// =============================================================================
// Text Position Structure
// Stores the starting positions (GPoint) for main display elements.
// =============================================================================

struct TextPositions {
    GPoint hours;      // Position for the hours display
    GPoint date;       // Position for the date display
    GPoint bluetooth;  // Position for the Bluetooth status icon
    GPoint updates;    // Position for the update notification icon
};

// =============================================================================
// Position Calculation Functions
// =============================================================================

/**
 * Create a GPoint with the given x and y coordinates.
 * @param x  Horizontal position
 * @param y  Vertical position
 * @return     A GPoint struct
 */
GPoint create_point(int x, int y);

/**
 * Get the screen position for a specific display item in a given slot.
 * @param slot        Which slot the item is in (SLOT_A through SLOT_F)
 * @param item        Which display item (WEATHER_ITEM, STEPS_ITEM, etc.)
 * @param mode        Display mode (MODE_NORMAL or MODE_SIMPLE)
 * @param font_type   The currently selected font type
 * @param width       Available display width
 * @param height      Available display height
 * @return             GPoint with the calculated position
 */
GPoint get_pos_for_item(int slot, int item, int mode, int font_type, int width, int height);

/**
 * Get the starting positions for the main display elements (hours, date, etc.).
 * @param font_type   The currently selected font type
 * @param text_align  Text alignment (ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT)
 * @param positions   Output struct to store the calculated positions
 * @param width       Available display width
 * @param height      Available display height
 */
void get_text_positions(int font_type, int text_align,
                        struct TextPositions *positions, int width, int height);

#endif // __TIMEBOXED_POSITIONS_
