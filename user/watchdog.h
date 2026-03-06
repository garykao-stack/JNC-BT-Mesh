/***************************************************************************//**
 * @file watchdog.h
 * @brief Watchdog Timer Driver for JNC-BT-Mesh
 * @details
 *   Provides watchdog initialization, feeding, and timeout configuration
 *   for the BGM13P32F512GA platform with WDOG0 as the primary watchdog.
 *
 * Hardware: BGM13P32F512GA (Si7021 with 2x WDOG units)
 ******************************************************************************/

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup watchdog
 * @{
 ******************************************************************************/

/**
 * @brief Watchdog timeout periods
 * @details
 *   Based on ULFRCO clock = 1 kHz (1000 cycles/second)
 *   PERSEL directly maps to timeout in watchdog clock cycles:
 *   - PERSEL encoding follows EFM32 spec (0-15)
 *   - Clock source: ULFRCO = 1 kHz
 */
typedef enum {
  WDOG_PERIOD_33MS      = 2,    /**< ~33 ms   (33 cycles @ 1kHz) */
  WDOG_PERIOD_65MS      = 3,    /**< ~65 ms   (65 cycles @ 1kHz) */
  WDOG_PERIOD_129MS     = 4,    /**< ~129 ms  (129 cycles @ 1kHz) */
  WDOG_PERIOD_257MS     = 5,    /**< ~257 ms  (257 cycles @ 1kHz) */
  WDOG_PERIOD_513MS     = 6,    /**< ~513 ms  (513 cycles @ 1kHz) */
  WDOG_PERIOD_1S        = 7,    /**< ~1.024 s (1K cycles @ 1kHz) */
  WDOG_PERIOD_2S        = 8,    /**< ~2.048 s (2K cycles @ 1kHz) */
  WDOG_PERIOD_4S        = 9,    /**< ~4.096 s (4K cycles @ 1kHz) */
  WDOG_PERIOD_8S        = 10,   /**< ~8.192 s (8K cycles @ 1kHz) */
  WDOG_PERIOD_16S       = 11,   /**< ~16.384 s (16K cycles @ 1kHz) */
  WDOG_PERIOD_32S       = 12,   /**< ~32.768 s (32K cycles @ 1kHz) */
  WDOG_PERIOD_65S       = 13,   /**< ~65.536 s (64K cycles @ 1kHz) */
  WDOG_PERIOD_131S      = 14,   /**< ~131.072 s (128K cycles @ 1kHz) */
  WDOG_PERIOD_262S      = 15    /**< ~262.144 s (256K cycles @ 1kHz) */
} watchdog_period_t;

/**
 * @brief Watchdog warning interrupt periods (in milliseconds)
 */
typedef enum {
  WDOG_WARNING_NEVER    = 0,    /**< No warning interrupt */
  WDOG_WARNING_25PCT    = 1,    /**< Warning at 25% of timeout */
  WDOG_WARNING_50PCT    = 2,    /**< Warning at 50% of timeout */
  WDOG_WARNING_75PCT    = 3     /**< Warning at 75% of timeout */
} watchdog_warning_t;

/**
 * @brief Watchdog configuration structure
 */
typedef struct {
  watchdog_period_t   timeout_period;  /**< Watchdog timeout period */
  watchdog_warning_t  warning_percent; /**< Warning interrupt period */
  bool                enable_warning;  /**< Enable watchdog warning interrupt */
  bool                run_in_debug;    /**< Keep watchdog active during debugging */
  bool                enable_lockup;   /**< Enable lockup reset */
} watchdog_config_t;

/***************************************************************************//**
 * @brief Initialize watchdog with default configuration
 * @details
 *   Configures WDOG0 with:
 *   - Timeout: 8 seconds
 *   - Warning: Disabled
 *   - Lockup reset: Enabled
 *   - Debug mode: Disabled (stops in debug)
 * @return true if initialization successful, false otherwise
 ******************************************************************************/
bool watchdog_init(void);

/***************************************************************************//**
 * @brief Initialize watchdog with custom configuration
 * @param[in] config Pointer to watchdog configuration structure
 * @return true if initialization successful, false otherwise
 ******************************************************************************/
bool watchdog_init_config(const watchdog_config_t *config);

/***************************************************************************//**
 * @brief Feed (pet) the watchdog - must be called regularly
 * @details
 *   Resets the watchdog timer. If watchdog is not fed before
 *   timeout expires, the device will reboot.
 *   Typical usage: call every 1-2 seconds for an 8-second timeout.
 * @return None
 ******************************************************************************/
void watchdog_feed(void);

/***************************************************************************//**
 * @brief Disable watchdog timer
 * @details
 *   Disables the watchdog. Use with caution - device will be unprotected
 *   from system hangs/deadlocks.
 * @return None
 ******************************************************************************/
void watchdog_disable(void);

/***************************************************************************//**
 * @brief Check if watchdog is enabled
 * @return true if watchdog is enabled, false otherwise
 ******************************************************************************/
bool watchdog_is_enabled(void);

/***************************************************************************//**
 * @brief Get current watchdog timeout value in seconds
 * @return Timeout value in seconds (256ms, 512ms, 1s, 2s, 4s, 8s, 16s, 32s)
 ******************************************************************************/
uint16_t watchdog_get_timeout_seconds(void);

/***************************************************************************//**
 * @brief Watchdog warning interrupt callback
 * @details
 *   Weak function that can be overridden by application.
 *   Called when watchdog warning interrupt triggers (before timeout).
 *   Use this to perform critical cleanup/logging before 
 *   watchdog reboot.
 * @return None
 ******************************************************************************/
void watchdog_warning_callback(void);

/** @} (end addtogroup watchdog) */

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOG_H */
