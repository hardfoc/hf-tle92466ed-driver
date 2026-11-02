/**
 * @file test_transport.cpp
 * @brief The pipelined register access of SpiInterface: frame sequence, reply validation and fault
 * frames.
 */
#include "mock_tle92466ed.hpp"
#include "test_harness.hpp"

using tle92466ed::CommError;
using tle92466ed::SPIFrame;
using tle92466ed_test::MockTle92466ed;
namespace CentralReg = tle92466ed::CentralReg;

namespace {
uint32_t readFrame(uint16_t addr) {
  return MockTle92466ed::WithCrc(SPIFrame::MakeRead(addr).word);
}
} // namespace

TEST_CASE(
    "Read() sends a flush dummy, the command and a trailing dummy, and keeps the last reply") {
  MockTle92466ed chip;
  chip.regs()[CentralReg::ICVID] = 0xC1A5;
  auto v = chip.Read(CentralReg::ICVID);
  REQUIRE(v.has_value());
  CHECK_EQ(*v, 0xC1A5U);
  REQUIRE(chip.frames().size() == 3U);
  CHECK_EQ(chip.frames()[0].mosi, readFrame(0));
  CHECK_EQ(chip.frames()[1].mosi, readFrame(CentralReg::ICVID));
  CHECK_EQ(chip.frames()[2].mosi, readFrame(0));
}

TEST_CASE("Read() of a feedback register accepts only a 22-bit reply") {
  MockTle92466ed chip;
  chip.regs()[CentralReg::FB_STAT] = 0x2ABCDE;
  auto v = chip.Read(CentralReg::FB_STAT);
  REQUIRE(v.has_value());
  CHECK_EQ(*v, 0x2ABCDEU);

  chip.forceReply16(true); // another register's 16-bit reply landing in the slot
  auto stale = chip.Read(CentralReg::FB_STAT);
  REQUIRE(!stale.has_value());
  CHECK(stale.error() == CommError::CRCError);
}

TEST_CASE("Read() reports NoReply when MISO stays idle") {
  MockTle92466ed chip;
  chip.setIdleMiso(true);
  auto v = chip.Read(CentralReg::GLOBAL_CONFIG);
  REQUIRE(!v.has_value());
  CHECK(v.error() == CommError::NoReply);
}

TEST_CASE("Write() puts the value in the register and checks the reply's status field") {
  MockTle92466ed chip;
  REQUIRE(chip.Write(CentralReg::VBAT_TH, 0x1234).has_value());
  CHECK_EQ(chip.regs()[CentralReg::VBAT_TH], 0x1234U);

  chip.setWriteStatus(static_cast<uint8_t>(tle92466ed::SPIStatus::WRITE_RO_REG));
  auto w = chip.Write(CentralReg::VBAT_TH, 0x0001);
  REQUIRE(!w.has_value());
  CHECK(w.error() == CommError::TransferError);
}

TEST_CASE("a frame with a bad CRC is answered with the Parity/CRC error status") {
  MockTle92466ed chip;
  const uint32_t good = readFrame(CentralReg::ICVID);
  (void)chip.Transfer32(good ^ 0x01000000U); // corrupt the CRC byte
  auto reply = chip.Transfer32(readFrame(0));
  REQUIRE(reply.has_value());
  SPIFrame f{};
  f.word = *reply;
  CHECK_EQ(f.rx_16bit.status, static_cast<uint32_t>(tle92466ed::SPIStatus::CRC_ERROR));
}

TEST_CASE("ReadMulti() reads N registers in N + 2 frames and marks each valid reply") {
  MockTle92466ed chip;
  chip.regs()[CentralReg::ICVID] = 0xC101;
  chip.regs()[CentralReg::FB_VOLTAGE1] = 0x012345;
  chip.regs()[CentralReg::GLOBAL_CONFIG] = 0x4005;
  const uint16_t addrs[] = {CentralReg::ICVID, CentralReg::FB_VOLTAGE1, CentralReg::GLOBAL_CONFIG};
  uint32_t values[3] = {};
  uint16_t mask = 0;
  REQUIRE(chip.ReadMulti(addrs, values, mask).has_value());
  CHECK_EQ(chip.frames().size(), 5U);
  CHECK_EQ(mask, 0b111U);
  CHECK_EQ(values[0], 0xC101U);
  CHECK_EQ(values[1], 0x012345U);
  CHECK_EQ(values[2], 0x4005U);
}

TEST_CASE("a critical fault frame fails the read and keeps the fault byte") {
  MockTle92466ed chip;
  chip.setCriticalFault(true, 0x1F); // 1V5, 2V5 and bandgap not ok, clock faults
  auto v = chip.Read(CentralReg::GLOBAL_DIAG0);
  REQUIRE(!v.has_value());
  CHECK(v.error() == CommError::BusError);
  CHECK(chip.SawCriticalFault());
  CHECK_EQ(chip.LastCriticalFaultFlags(), 0x1FU);
}
