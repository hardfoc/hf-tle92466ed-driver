/**
 * @file configuration.cpp
 * @brief The Configuration page's examples, compiled by the host build (never run).
 */
#include "my_spi.hpp"

using Driver = tle92466ed::Driver<MySpi>;
using tle92466ed::Channel;

// --8<-- [start:global]
// Config Mode only. CRC and the SPI watchdog are switched on here, after the bus is proven.
bool configure_global(Driver& driver) {
  tle92466ed::GlobalConfig cfg{};
  cfg.crc_enabled = true;             // the chip checks the CRC byte of every frame
  cfg.spi_watchdog_enabled = true;    // reload it with ReloadSpiWatchdog() before it runs out
  cfg.clock_watchdog_enabled = true;
  cfg.vio_5v = false;                 // VIO is 3.3 V on this board
  cfg.vbat_uv_voltage = 7.0F;         // VBAT_TH thresholds, in volts
  cfg.vbat_ov_voltage = 18.0F;
  cfg.spi_watchdog_reload = 1000;     // WD_TIME counts
  return driver.ConfigureGlobal(cfg).has_value();
}
// --8<-- [end:global]

// --8<-- [start:channel]
// Config Mode only: the channel mode and the diagnostics belong to the static configuration.
bool configure_valve(Driver& driver) {
  tle92466ed::ChannelConfig ch{};
  ch.mode = tle92466ed::ChannelMode::ICC;               // integrated current control
  ch.current_setpoint_ma = 0;                           // set the real target in Mission Mode
  ch.slew_rate = tle92466ed::SlewRate::MEDIUM_2V5_US;
  ch.diag_current = tle92466ed::DiagCurrent::I_80UA;    // off-state open-load / short detection
  ch.open_load_threshold = 3;                           // OL threshold code: 0 = off, 1..7
  return driver.ConfigureChannel(Channel::CH2, ch).has_value();
}
// --8<-- [end:channel]

// --8<-- [start:parallel]
// Config Mode only: CH0 and CH3 driven as one 2.7 A output. Only the master (CH0) is configured.
bool configure_parallel(Driver& driver) {
  if (!driver.SetParallelOperation(tle92466ed::ParallelPair::CH0_CH3, true)) {
    return false;
  }
  return driver.SetCurrentSetpoint(Channel::CH0, 2000, /*parallel_mode=*/true).has_value();
}
// --8<-- [end:parallel]
