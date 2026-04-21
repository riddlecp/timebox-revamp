/**
 * positions.c - Layout position calculation implementation.
 *
 * This file contains all the logic for determining where each display
 * element should be positioned on screen. The positions depend on:
 *   - The selected font (different fonts have different sizes)
 *   - Screen type (square vs round)
 *   - Text alignment preference
 *   - Display mode (normal vs simple)
 *   - Which slot a module is assigned to
 */

#include <pebble.h>
#include "keys.h"
#include "configs.h"
#include "positions.h"

// =============================================================================
// GPoint Creation Helper
// =============================================================================

GPoint create_point(int x, int y) {
    return (GPoint){.x = x, .y = y};
}

// =============================================================================
// Main Layout Position Calculation
// Determines where the hours, date, and status icons start.
// =============================================================================

void get_text_positions(int font_type, int text_align,
                        struct TextPositions *positions, int width, int height) {
    // The hours are the largest element, centered vertically on the screen
    // Position depends on font type (different fonts have different sizes)
    switch (font_type) {
        case BLOCKO_FONT:
            // Blocko font: large hours display
            positions->hours = create_point(0, height / 2 - 30);
            positions->date = create_point(0, height / 2 + 20);
            break;
        case BLOCKO_BIG_FONT:
            // Blocko big: even larger hours
            positions->hours = create_point(0, height / 2 - 35);
            positions->date = create_point(0, height / 2 + 30);
            break;
        case SYSTEM_FONT:
            // System font: standard Pebble font
            positions->hours = create_point(0, height / 2 - 25);
            positions->date = create_point(0, height / 2 + 15);
            break;
        case ARCHIVO_FONT:
            // Archivo Narrow font
            positions->hours = create_point(0, height / 2 - 28);
            positions->date = create_point(0, height / 2 + 18);
            break;
        case DIN_FONT:
            // OSP-DIN font
            positions->hours = create_point(0, height / 2 - 30);
            positions->date = create_point(0, height / 2 + 20);
            break;
        case PROTOTYPE_FONT:
            // Prototype font
            positions->hours = create_point(0, height / 2 - 27);
            positions->date = create_point(0, height / 2 + 17);
            break;
        case LECO_FONT:
            // Leco font
            positions->hours = create_point(0, height / 2 - 28);
            positions->date = create_point(0, height / 2 + 18);
            break;
        case KONSTRUCT_FONT:
            // Konstruct font
            positions->hours = create_point(0, height / 2 - 22);
            positions->date = create_point(0, height / 2 + 14);
            break;
        default:
            // Fallback to Blocko positioning
            positions->hours = create_point(0, height / 2 - 30);
            positions->date = create_point(0, height / 2 + 20);
            break;
    }

    // Position Bluetooth icon at the top of the screen
    // On round screens, it's centered at the top; on square, it's at the top-right
    positions->bluetooth = create_point(
        PBL_IF_ROUND_ELSE(width / 2 - 10, text_align == ALIGN_LEFT ? 10 : width - 20),
        0
    );

    // Position update notification icon at the bottom
    positions->updates = create_point(
        PBL_IF_ROUND_ELSE(width / 2 - 10, text_align == ALIGN_LEFT ? 10 : width - 20),
        height - 20
    );
}

// =============================================================================
// Slot-Based Position Calculation
// Determines where a specific module's content should go based on its slot.
// =============================================================================

GPoint get_pos_for_item(int slot, int item, int mode, int font_type, int width, int height) {
    // Calculate positions for each slot (A-F).
    // Slots A-D are on the sides of the screen.
    // Slots E-F are in the center (round screens only).
    // The exact Y position depends on the font type (different fonts need different spacing).

    int font_offset = 0;
    switch (font_type) {
        case BLOCKO_FONT:      font_offset = 0; break;
        case BLOCKO_BIG_FONT:  font_offset = 10; break;
        case SYSTEM_FONT:      font_offset = -5; break;
        case ARCHIVO_FONT:     font_offset = -3; break;
        case DIN_FONT:         font_offset = 0; break;
        case PROTOTYPE_FONT:   font_offset = -2; break;
        case LECO_FONT:        font_offset = -3; break;
        case KONSTRUCT_FONT:   font_offset = -2; break;
        default:               font_offset = 0; break;
    }

    // Slot positions vary by font type. Here are the positions for each slot:
    // Slot A: Top-left area
    // Slot B: Below slot A
    // Slot C: Below slot B
    // Slot D: Below slot C
    // Slot E: Center (round screens)
    // Slot F: Below center (round screens)

    int y_positions[6];
    int x_offset = 0;

    switch (font_type) {
        case BLOCKO_FONT:
            y_positions[0] = height / 2 - 30 + font_offset;
            y_positions[1] = height / 2 + 5;
            y_positions[2] = height / 2 + 40;
            y_positions[3] = height / 2 + 75;
            y_positions[4] = height / 2 - 15;  // Slot E (center)
            y_positions[5] = height / 2 + 10;  // Slot F (center)
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        case BLOCKO_BIG_FONT:
            y_positions[0] = height / 2 - 35 + font_offset;
            y_positions[1] = height / 2;
            y_positions[2] = height / 2 + 35;
            y_positions[3] = height / 2 + 70;
            y_positions[4] = height / 2 - 20;
            y_positions[5] = height / 2 + 5;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        case SYSTEM_FONT:
            y_positions[0] = height / 2 - 25 + font_offset;
            y_positions[1] = height / 2 + 10;
            y_positions[2] = height / 2 + 45;
            y_positions[3] = height / 2 + 80;
            y_positions[4] = height / 2 - 10;
            y_positions[5] = height / 2 + 15;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        case ARCHIVO_FONT:
            y_positions[0] = height / 2 - 28 + font_offset;
            y_positions[1] = height / 2 + 7;
            y_positions[2] = height / 2 + 42;
            y_positions[3] = height / 2 + 77;
            y_positions[4] = height / 2 - 12;
            y_positions[5] = height / 2 + 13;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        case DIN_FONT:
            y_positions[0] = height / 2 - 30 + font_offset;
            y_positions[1] = height / 2 + 5;
            y_positions[2] = height / 2 + 40;
            y_positions[3] = height / 2 + 75;
            y_positions[4] = height / 2 - 15;
            y_positions[5] = height / 2 + 10;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        case PROTOTYPE_FONT:
            y_positions[0] = height / 2 - 27 + font_offset;
            y_positions[1] = height / 2 + 8;
            y_positions[2] = height / 2 + 43;
            y_positions[3] = height / 2 + 78;
            y_positions[4] = height / 2 - 12;
            y_positions[5] = height / 2 + 13;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        case LECO_FONT:
            y_positions[0] = height / 2 - 28 + font_offset;
            y_positions[1] = height / 2 + 7;
            y_positions[2] = height / 2 + 42;
            y_positions[3] = height / 2 + 77;
            y_positions[4] = height / 2 - 12;
            y_positions[5] = height / 2 + 13;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        case KONSTRUCT_FONT:
            y_positions[0] = height / 2 - 22 + font_offset;
            y_positions[1] = height / 2 + 12;
            y_positions[2] = height / 2 + 46;
            y_positions[3] = height / 2 + 80;
            y_positions[4] = height / 2 - 8;
            y_positions[5] = height / 2 + 17;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
        default:
            // Fallback to Blocko positioning
            y_positions[0] = height / 2 - 30 + font_offset;
            y_positions[1] = height / 2 + 5;
            y_positions[2] = height / 2 + 40;
            y_positions[3] = height / 2 + 75;
            y_positions[4] = height / 2 - 15;
            y_positions[5] = height / 2 + 10;
            x_offset = text_align == ALIGN_LEFT ? 5 : -5;
            break;
    }

    // Return the position for the requested slot
    return create_point(x_offset, y_positions[slot]);
}
