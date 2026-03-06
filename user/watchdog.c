/***************************************************************************//**
 * @file watchdog.c
 * @brief Watchdog Timer Implementation for JNC-BT-Mesh
 * @details
 *   Low-level watchdog driver for BGM13P32F512GA.
 *   Provides initialization, feeding, and interrupt handling.
 *   Note: WDOG uses internal 1kHz ULFRCO clock (always enabled).
 ******************************************************************************/
#include "global.h"
#include "watchdog.h"
#include "em_device.h"
#if defined(_CMU_HFPERCLKEN0_MASK)
#include "em_cmu.h"
#endif
#include "debugprint.h"

/***************************************************************************//**
 * Local Variables
 ******************************************************************************/
static bool wdog_enabled = false;
static watchdog_period_t wdog_timeout_period = WDOG_PERIOD_8S;

/***************************************************************************//**
 * @brief Map timeout period enum to register value (for PERSEL field)
 * @param[in] period Watchdog timeout period
 * @return Register timeout value (PERSEL bits [11:8])
 * @details
 *   Enum values directly correspond to PERSEL register values per EFM32 spec.
 ******************************************************************************/
static inline uint8_t watchdog_period_to_register(watchdog_period_t period)
{
  return (uint8_t)period;  /* Direct PERSEL value (enum = PERSEL value) */
}

/***************************************************************************//**
 * @brief Convert timeout enum to seconds
 * @param[in] period Watchdog timeout period
 * @return Timeout value in seconds (rounded)
 * @details
 *   Based on ULFRCO clock = 1 kHz = 1000 cycles/second
 *   PERSEL values from EFM32 spec (cycle counts):
 *   PERSEL 0-15: 9, 17, 33, 65, 129, 257, 513, 1K, 2K, 4K, 8K, 16K, 32K, 64K, 128K, 512K
 ******************************************************************************/
static uint16_t watchdog_period_to_seconds(watchdog_period_t period)
{
  /* Cycle counts from PERSEL encoding at ULFRCO = 1 kHz */
  const uint32_t persel_cycles[] = {
    9,        /* PERSEL=0 - 9 cycles ≈ 0.009 sec */
    17,       /* PERSEL=1 - 17 cycles ≈ 0.017 sec */
    33,       /* PERSEL=2 - 33 cycles ≈ 0.033 sec */
    65,       /* PERSEL=3 - 65 cycles ≈ 0.065 sec */
    129,      /* PERSEL=4 - 129 cycles ≈ 0.129 sec */
    257,      /* PERSEL=5 - 257 cycles ≈ 0.257 sec */
    513,      /* PERSEL=6 - 513 cycles ≈ 0.513 sec */
    1024,     /* PERSEL=7 - 1K cycles ≈ 1.024 sec */
    2048,     /* PERSEL=8 - 2K cycles ≈ 2.048 sec */
    4096,     /* PERSEL=9 - 4K cycles ≈ 4.096 sec */
    8192,     /* PERSEL=10 - 8K cycles ≈ 8.192 sec */
    16384,    /* PERSEL=11 - 16K cycles ≈ 16.384 sec */
    32768,    /* PERSEL=12 - 32K cycles ≈ 32.768 sec */
    65536,    /* PERSEL=13 - 64K cycles ≈ 65.536 sec */
    131072,   /* PERSEL=14 - 128K cycles ≈ 131.072 sec */
    524288    /* PERSEL=15 - 512K cycles ≈ 524.288 sec */
  };

  if (period > 15) return 0;
  
  /* Convert cycles to seconds: cycles / 1000, rounded */
  return (persel_cycles[period] + 500) / 1000;
}

/***************************************************************************//**
 * @brief Default watchdog warning callback (weak)
 ******************************************************************************/
__weak void watchdog_warning_callback(void)
{
  /* Default empty implementation - can be overridden by application */
  dprint("WATCHDOG: Warning interrupt triggered!\r\n");
}

/***************************************************************************//**
 * @brief Watchdog warning interrupt handler
 * @details
 *   Called when watchdog warning interrupt fires (at configurable
 *   percentage of timeout period). Provides chance to log/cleanup
 *   before device reboot.
 ******************************************************************************/
void WDOG0_IRQHandler(void)
{
  uint32_t flags = WDOG0->IF;
  
  if (flags & WDOG_IF_WARN) {
    WDOG0->IFC = WDOG_IF_WARN;  /* Clear the warning interrupt flag */
    dprint("[WATCHDOG] Warning interrupt - timeout imminent!\r\n");
    watchdog_warning_callback();
  }
}

/***************************************************************************//**
 * @brief Initialize watchdog with default configuration
 * @return true if successful
 ******************************************************************************/
bool watchdog_init(void)
{
  watchdog_config_t default_config = {
    .timeout_period = WDOG_PERIOD_8S,
    .warning_percent = WDOG_WARNING_NEVER,
    .enable_warning = false,
    .run_in_debug = false,
    .enable_lockup = true
  };
  
  return watchdog_init_config(&default_config);
}

/***************************************************************************//**
 * @brief Initialize watchdog with custom configuration
 * @param[in] config Configuration structure
 * @return true if successful
 ******************************************************************************/
bool watchdog_init_config(const watchdog_config_t *config)
{
  if (!config) return false;
  
  /* Store configuration */
  wdog_timeout_period = config->timeout_period;
  
  /* Enable HFPER bus clock if available (some devices require it) */
#if defined(_CMU_HFPERCLKEN0_MASK)
  CMU_ClockEnable(cmuClock_HFPER, true);
#endif
  
  /* Build control register value - start fresh */
  uint32_t ctrl = 0;
  
  /* Set timeout period (PERSEL bits [11:8]) - direct enumeration value */
  ctrl |= (watchdog_period_to_register(config->timeout_period) << _WDOG_CTRL_PERSEL_SHIFT);
  
  /* Set clock source to ULFRCO (bits [13:12]) - default is already ULFRCO, but explicitly set */
  /* CLKSEL = 0x0 for ULFRCO, bits [13:12] */
  /* ctrl &= ~_WDOG_CTRL_CLKSEL_MASK;  */ /* Already 0 from init */
  
  /* Enable the watchdog (EN bit 0) */
  ctrl |= WDOG_CTRL_EN;
  
  /* Configure warning interrupt if enabled (WARNSEL bits [17:16]) */
  if (config->enable_warning) {
    ctrl |= ((config->warning_percent & 0x3) << _WDOG_CTRL_WARNSEL_SHIFT);
  }
  
  /* Configure lockup reset - bit 31 (WDOGRSTDIS) */
  if (!config->enable_lockup) {
    ctrl |= WDOG_CTRL_WDOGRSTDIS;  /* Disable reset on lockup */
  }
  
  /* Configure debug mode - bit 1 (DEBUGRUN) */
  if (config->run_in_debug) {
    ctrl |= WDOG_CTRL_DEBUGRUN;  /* Keep running in debug mode */
  }
  
  /* Write control register */
  WDOG0->CTRL = ctrl;
  
  /* Wait for synchronization */
  while (WDOG0->SYNCBUSY & WDOG_SYNCBUSY_CTRL) {
    /* Busy wait for CTRL sync */
  }
  
  /* Feed watchdog immediately after initialization to reset counter */
  WDOG0->CMD = WDOG_CMD_CLEAR;
  
  /* Wait for CMD sync */
  while (WDOG0->SYNCBUSY & WDOG_SYNCBUSY_CMD) {
    /* Busy wait for CMD sync */
  }
  
  /* Enable warning interrupt in IEN register if requested */
  if (config->enable_warning) {
    WDOG0->IEN |= WDOG_IF_WARN;  /* Set the WARN bit in interrupt enable */
    
    /* Enable WDOG0 interrupt in NVIC */
    NVIC_ClearPendingIRQ(WDOG0_IRQn);
    NVIC_EnableIRQ(WDOG0_IRQn);
  }
  
  wdog_enabled = true;
  
  dprint("[WATCHDOG] Initialized - Timeout: %d sec\r\n", 
         watchdog_period_to_seconds(config->timeout_period));
  
  return true;
}

/***************************************************************************//**
 * @brief Feed (pet) the watchdog
 ******************************************************************************/
void watchdog_feed(void)
{
  if (!wdog_enabled) return;
  
  /* Feed WDOG0 - write CLEAR command to CMD register */
  WDOG0->CMD = WDOG_CMD_CLEAR;  /* Reset the watchdog counter */
}

/***************************************************************************//**
 * @brief Disable watchdog
 ******************************************************************************/
void watchdog_disable(void)
{
  if (!wdog_enabled) return;
  
  /* Disable the watchdog by clearing EN bit */
  WDOG0->CTRL &= ~WDOG_CTRL_EN;
  
  wdog_enabled = false;
  
  dprint("[WATCHDOG] Disabled\r\n");
}

/***************************************************************************//**
 * @brief Check if watchdog is enabled
 * @return true if enabled
 ******************************************************************************/
bool watchdog_is_enabled(void)
{
  return wdog_enabled;
}

/***************************************************************************//**
 * @brief Get current watchdog timeout in seconds
 * @return Timeout value in seconds
 ******************************************************************************/
uint16_t watchdog_get_timeout_seconds(void)
{
  return watchdog_period_to_seconds(wdog_timeout_period);
}
