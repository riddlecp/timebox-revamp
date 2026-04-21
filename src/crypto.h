/**
 * crypto.h - Cryptocurrency price display interface.
 *
 * Provides functions to:
 *   - Request crypto price updates from the companion app
 *   - Update the display with received prices
 *   - Store prices in persistent memory
 *   - Manage the update interval
 */

#ifndef __TIMEBOXED_CRYPTO_
#define __TIMEBOXED_CRYPTO_

#if !defined PBL_PLATFORM_APLITE
#include <pebble.h>

/** Check if crypto tracking is enabled */
bool is_crypto_enabled(void);

/** Update the primary cryptocurrency price display */
void update_crypto_price(char *price);

/** Update the secondary cryptocurrency price display */
void update_crypto_price_b(char *price);

/** Update the tertiary cryptocurrency price display */
void update_crypto_price_c(char *price);

/** Update the quaternary cryptocurrency price display */
void update_crypto_price_d(char *price);

/** Toggle crypto tracking based on user configuration */
void toggle_crypto(uint8_t reload_origin);

/** Store a crypto price in persistent memory */
void store_crypto_price(char *price);

/** Store a crypto price B in persistent memory */
void store_crypto_price_b(char *price);

/** Store a crypto price C in persistent memory */
void store_crypto_price_c(char *price);

/** Store a crypto price D in persistent memory */
void store_crypto_price_d(char *price);

/** Force a crypto price update */
void update_crypto(bool force);
#endif
#endif // __TIMEBOXED_CRYPTO_
