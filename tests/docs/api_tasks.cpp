/**
 * @file api_tasks.cpp
 * @brief The API guide's examples, one function per task, compiled by the host build (never run).
 */
#include "my_spi.hpp"

#include <cstdio>

using Driver = tle92466ed::Driver<MySpi>;
using tle92466ed::Channel;
using tle92466ed::DriverError;

// --8<-- [start:init]
bool bring_up(Driver& driver) {
  auto r = driver.Init();  // RESN pulse, EN high, ICVID check, defaults; ends in Config Mode
  if (!r) {
    if (r.error() == DriverError::WrongDeviceID) {
      std::printf("not a TLE92466ED (ICVID high byte is not 0xC1)\n");
    } else if (r.error() == DriverError::FaultDetected) {
      std::printf("critical fault frame: check VDD (5 V) and VIO first\n");
    }
    return false;
  }
  return true;
}
// --8<-- [end:init]

// --8<-- [start:configure]
// In Config Mode: mode, PWM period and dither for one channel.
bool configure_channel(Driver& driver, Channel ch) {
  return driver.SetChannelMode(ch, tle92466ed::ChannelMode::ICC) &&
         driver.ConfigurePwmPeriod(ch, 500.0F) &&      // 500 us = 2 kHz
         driver.ConfigureDither(ch, 40.0F, 200.0F);    // 40 mA at 200 Hz; also starts Tmeas
}
// --8<-- [end:configure]

// --8<-- [start:drive]
bool drive(Driver& driver, Channel ch, uint16_t milliamps) {
  if (!driver.IsMissionMode() && !driver.EnterMissionMode()) {
    return false;
  }
  return driver.SetCurrentSetpoint(ch, milliamps) &&  // specified up to 1500 mA
         driver.EnableChannel(ch, true);
}

bool stop_all(Driver& driver) {
  return driver.DisableAllChannels().has_value();
}
// --8<-- [end:drive]

// --8<-- [start:read]
void report(Driver& driver, Channel ch) {
  if (auto i = driver.GetAverageCurrent(ch); i) {
    std::printf("I_avg = %u mA\n", *i);
  } else if (i.error() == DriverError::FeedbackNotReady) {
    std::printf("no measurement window yet (dither clock not running?)\n");
  }
  if (auto dc = driver.GetDutyCycle(ch); dc) {
    std::printf("duty = %u.%02u %%\n", *dc / 100U, *dc % 100U);
  }
  if (auto v = driver.ReadAllSupplyVoltages(); v) {
    std::printf("VBAT %u mV, VIO %u mV, VDD %u mV\n", v->vbat_mV, v->vio_mV, v->vdd_mV);
  }
}
// --8<-- [end:read]

// --8<-- [start:sample]
// Control loop: one pipelined burst for several channels, never blocks.
void control_tick(Driver& driver) {
  constexpr uint8_t kChannels = 0b000111;  // CH0..CH2
  auto batch = driver.SampleChannelCurrents(kChannels);
  if (!batch) {
    return;
  }
  for (unsigned n = 0; n < 6; ++n) {
    if ((batch->valid_mask & (1U << n)) != 0U) {
      const int32_t ma = batch->channels[n].iavg_ma;  // use it in your current loop
      (void)ma;
    }
  }
  // batch->rejected_chains > 0 means replies arrived out of order on the bus.
}
// --8<-- [end:sample]

// --8<-- [start:faults]
void handle_faults(Driver& driver) {
  auto pin = driver.IsFault();  // FAULTN low?
  if (!pin || !*pin) {
    return;
  }
  auto report = driver.GetAllFaults();
  if (!report) {
    return;
  }
  if (report->vbat_uv || report->vdd_uv) {
    std::printf("supply under-voltage\n");
  }
  for (unsigned n = 0; n < report->channels.size(); ++n) {
    const auto& c = report->channels[n];
    if (c.open_load || c.short_to_ground || c.overcurrent || c.over_temperature) {
      std::printf("CH%u: OL=%d SG=%d OC=%d OT=%d\n", n, c.open_load, c.short_to_ground,
                  c.overcurrent, c.over_temperature);
    }
  }
  (void)driver.ClearFaults();  // after the cause is fixed; a fault that is still there comes back
}
// --8<-- [end:faults]

// --8<-- [start:watchdog]
// With GlobalConfig::spi_watchdog_enabled, call this faster than the watchdog runs out.
bool kick_watchdog(Driver& driver) {
  return driver.ReloadSpiWatchdog(1000).has_value();
}
// --8<-- [end:watchdog]

// --8<-- [start:raw]
bool raw_access(Driver& driver) {
  namespace reg = tle92466ed::CentralReg;
  auto id = driver.ReadRegister(reg::ICVID);  // 0xC1xx: type 0xC1, silicon revision in the low byte
  if (!id) {
    return false;
  }
  const uint16_t setpoint =
      tle92466ed::GetChannelRegister(Channel::CH1, tle92466ed::ChannelReg::SETPOINT);
  return driver.ModifyRegister(setpoint, 0x7FFF, 0x1000).has_value();  // TARGET bits only
}
// --8<-- [end:raw]
