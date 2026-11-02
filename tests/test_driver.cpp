/**
 * @file test_driver.cpp
 * @brief Driver behaviour over the register model: bring-up, identity, modes, setpoint encoding and
 * errors.
 */
#include "mock_tle92466ed.hpp"
#include "test_harness.hpp"

using tle92466ed::Channel;
using tle92466ed::CtrlPin;
using tle92466ed::DriverError;
using tle92466ed::GpioSignal;
using tle92466ed_test::MockTle92466ed;
using Driver = tle92466ed::Driver<MockTle92466ed>;
namespace CentralReg = tle92466ed::CentralReg;
namespace ChannelReg = tle92466ed::ChannelReg;

namespace {
uint16_t setpointOf(MockTle92466ed& chip, Channel ch) {
  return static_cast<uint16_t>(
      chip.regs()[tle92466ed::GetChannelRegister(ch, ChannelReg::SETPOINT)] & 0x7FFFU);
}
} // namespace

TEST_CASE("Init() pulses RESN, asserts EN, verifies ICVID and leaves the chip in Config Mode") {
  MockTle92466ed chip;
  Driver drv(chip);
  REQUIRE(drv.Init().has_value());
  CHECK(drv.IsInitialized());
  CHECK(drv.IsConfigMode());

  bool reset_asserted = false;
  bool reset_released_after = false;
  for (const auto& e : chip.pinLog()) {
    if (e.pin == CtrlPin::RESN && e.signal == GpioSignal::ACTIVE) {
      reset_asserted = true;
    }
    if (reset_asserted && e.pin == CtrlPin::RESN && e.signal == GpioSignal::INACTIVE) {
      reset_released_after = true;
    }
  }
  CHECK(reset_asserted);
  CHECK(reset_released_after);
  CHECK(chip.GpioRead(CtrlPin::EN).value() == GpioSignal::ACTIVE);
  CHECK(chip.elapsedUs() >= 20000U); // two 10 ms windows around the RESN pulse at least
}

TEST_CASE("Init() rejects a part whose ICVID is not 0xC1xx") {
  MockTle92466ed chip(0x4201);
  Driver drv(chip);
  auto r = drv.Init();
  REQUIRE(!r.has_value());
  CHECK(r.error() == DriverError::WrongDeviceID);
}

TEST_CASE("calls before Init() fail with NotInitialized") {
  MockTle92466ed chip;
  Driver drv(chip);
  auto r = drv.SetCurrentSetpoint(Channel::CH0, 100);
  REQUIRE(!r.has_value());
  CHECK(r.error() == DriverError::NotInitialized);
}

TEST_CASE("SetCurrentSetpoint() writes TARGET = I * 32767 / 2 A, rounded") {
  MockTle92466ed chip;
  Driver drv(chip);
  REQUIRE(drv.Init().has_value());
  REQUIRE(drv.SetCurrentSetpoint(Channel::CH0, 1000).has_value());
  CHECK_EQ(setpointOf(chip, Channel::CH0), 16384U);
  REQUIRE(drv.SetCurrentSetpoint(Channel::CH5, 1500).has_value());
  CHECK_EQ(setpointOf(chip, Channel::CH5), 24575U);
  CHECK_EQ(drv.GetCurrentSetpoint(Channel::CH5).value(), 1500U);
}

TEST_CASE("in parallel mode the same TARGET means twice the current (full scale 4 A)") {
  MockTle92466ed chip;
  Driver drv(chip);
  REQUIRE(drv.Init().has_value());
  REQUIRE(drv.SetCurrentSetpoint(Channel::CH1, 2000, true).has_value());
  CHECK_EQ(setpointOf(chip, Channel::CH1), 16384U);
}

TEST_CASE("setpoints above the register scale and invalid channels are refused") {
  MockTle92466ed chip;
  Driver drv(chip);
  REQUIRE(drv.Init().has_value());
  auto high = drv.SetCurrentSetpoint(Channel::CH0, 2001);
  REQUIRE(!high.has_value());
  CHECK(high.error() == DriverError::InvalidParameter);
  auto bad = drv.SetCurrentSetpoint(Channel::COUNT, 100);
  REQUIRE(!bad.has_value());
  CHECK(bad.error() == DriverError::InvalidChannel);
}

TEST_CASE("EnterMissionMode() sets OP_MODE in CH_CTRL; EnterConfigMode() clears it") {
  MockTle92466ed chip;
  Driver drv(chip);
  REQUIRE(drv.Init().has_value());
  REQUIRE(drv.EnterMissionMode().has_value());
  CHECK(drv.IsMissionMode());
  CHECK((chip.regs()[CentralReg::CH_CTRL] & tle92466ed::CH_CTRL::OP_MODE) != 0U);
  REQUIRE(drv.EnterConfigMode().has_value());
  CHECK(drv.IsConfigMode());
  CHECK_EQ(chip.regs()[CentralReg::CH_CTRL] & tle92466ed::CH_CTRL::OP_MODE, 0U);
}

TEST_CASE("a critical fault reply surfaces as FaultDetected") {
  MockTle92466ed chip;
  Driver drv(chip);
  REQUIRE(drv.Init().has_value());
  chip.setCriticalFault(true, 0x00);
  auto r = drv.ReadRegister(CentralReg::GLOBAL_DIAG0);
  REQUIRE(!r.has_value());
  CHECK(r.error() == DriverError::FaultDetected);
  auto w = drv.WriteRegister(CentralReg::VBAT_TH, 0x0101, true, false);
  REQUIRE(!w.has_value());
  CHECK(w.error() == DriverError::FaultDetected);
}

// GlobalConfig{} describes the state Init() leaves (datasheet 5.3 GLOBAL_CONFIG / VBAT_TH): applying
// the defaults must not switch on the SPI watchdog (it would drop the chip to Config Mode once WD_TIME
// runs out, datasheet 5.2.2) or change CRC_EN and the VBAT thresholds behind the caller's back.
TEST_CASE("ConfigureGlobal(GlobalConfig{}) keeps the configuration Init() wrote") {
  MockTle92466ed chip;
  Driver drv(chip);
  REQUIRE(drv.Init().has_value());
  const uint32_t global_after_init = chip.regs()[CentralReg::GLOBAL_CONFIG];
  const uint32_t vbat_after_init = chip.regs()[CentralReg::VBAT_TH];

  REQUIRE(drv.ConfigureGlobal(tle92466ed::GlobalConfig{}).has_value());
  CHECK_EQ(chip.regs()[CentralReg::GLOBAL_CONFIG], global_after_init);
  CHECK_EQ(chip.regs()[CentralReg::VBAT_TH], vbat_after_init);
  CHECK_EQ(chip.regs()[CentralReg::GLOBAL_CONFIG] & tle92466ed::GLOBAL_CONFIG::SPI_WD_EN, 0U);
  CHECK_EQ(chip.regs()[CentralReg::GLOBAL_CONFIG] & tle92466ed::GLOBAL_CONFIG::CRC_EN, 0U);
}
