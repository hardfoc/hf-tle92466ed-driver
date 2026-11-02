/**
 * @file quickstart.cpp
 * @brief The Quickstart page's example, compiled by the host build (never run).
 */
#include "my_spi.hpp"

// --8<-- [start:quickstart]
#include "tle92466ed.hpp"

int drive_ch0_at_500_ma(MySpi& spi) {
  using tle92466ed::Channel;
  tle92466ed::Driver<MySpi> driver(spi);

  if (!driver.Init()) {                                       // 1. reset, ICVID, Config Mode
    return -1;                                                //    VDD, wiring, SPI mode 1
  }
  if (!driver.ConfigurePwmPeriod(Channel::CH0, 1000.0F) ||    // 2. 1 kHz PWM,
      !driver.ConfigureDither(Channel::CH0, 50.0F, 100.0F)) { //    50 mA dither at 100 Hz
    return -2;
  }
  if (!driver.EnterMissionMode() ||                           // 3. outputs can switch
      !driver.SetCurrentSetpoint(Channel::CH0, 500) ||        // 4. regulate 500 mA
      !driver.EnableChannel(Channel::CH0, true)) {
    return -3;
  }
  return 0;
}
// --8<-- [end:quickstart]
