/**
 * @file test_frames.cpp
 * @brief SPI frame layout and CRC-8 SAE J1850 encoding, checked against the datasheet
 * (Rev. 1.2, 5.1.2 and 5.2.3).
 */
#include "test_harness.hpp"
#include "tle92466ed.hpp"

#include <cstdint>

using tle92466ed::CalculateCrc8J1850;
using tle92466ed::CalculateFrameCrc;
using tle92466ed::SPIFrame;
using tle92466ed::VerifyFrameCrc;

TEST_CASE("CRC-8 SAE J1850 gives the standard check value 0x4B for \"123456789\"") {
  const uint8_t msg[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  CHECK_EQ(CalculateCrc8J1850(msg, sizeof msg), 0x4B);
}

TEST_CASE("CRC-8 SAE J1850 of zero bytes is 0x00 (init 0xFF, final XOR 0xFF)") {
  CHECK_EQ(CalculateCrc8J1850(nullptr, 0), 0x00);
}

TEST_CASE("a write frame carries the 7-bit address in [23:17], R/W = 1 and the data in [15:0]") {
  const SPIFrame f = SPIFrame::MakeWrite(0x0045, 0xBEEF);
  CHECK_EQ((f.word >> 17) & 0x7FU, 0x45U);
  CHECK_EQ((f.word >> 16) & 0x1U, 1U);
  CHECK_EQ(f.word & 0xFFFFU, 0xBEEFU);
  CHECK_EQ(f.word >> 24, 0U); // CRC is added separately
}

TEST_CASE("a read frame carries the 16-bit address in [15:0], R/W = 0 and don't-care [23:17] = 0") {
  const SPIFrame f = SPIFrame::MakeRead(0x0242);
  CHECK_EQ(f.word & 0xFFFFU, 0x0242U);
  CHECK_EQ((f.word >> 16) & 0x1U, 0U);
  CHECK_EQ((f.word >> 17) & 0x7FU, 0U);
}

TEST_CASE("the frame CRC covers frame[7:0], frame[15:8], frame[23:16] in that order") {
  SPIFrame f = SPIFrame::MakeWrite(0x0002, 0x1234);
  const uint8_t bytes[] = {static_cast<uint8_t>(f.word), static_cast<uint8_t>(f.word >> 8),
                           static_cast<uint8_t>(f.word >> 16)};
  CHECK_EQ(CalculateFrameCrc(f), CalculateCrc8J1850(bytes, 3));
  f.tx_fields.crc = CalculateFrameCrc(f);
  CHECK(VerifyFrameCrc(f));
}

TEST_CASE("VerifyFrameCrc rejects a frame with any single bit flipped") {
  SPIFrame f = SPIFrame::MakeRead(0x0200);
  f.tx_fields.crc = CalculateFrameCrc(f);
  for (unsigned bit = 0; bit < 32; ++bit) {
    SPIFrame bad = f;
    bad.word ^= (1U << bit);
    CHECK(!VerifyFrameCrc(bad));
  }
}

TEST_CASE("reply frames decode reply mode, status and data from their fields") {
  SPIFrame r16{};
  r16.word = (0x00U << 22) | (0x03U << 17) | (1U << 16) | 0xA55AU;
  CHECK_EQ(r16.rx_common.reply_mode, 0U);
  CHECK_EQ(r16.rx_16bit.status, static_cast<uint32_t>(tle92466ed::SPIStatus::WRITE_RO_REG));
  CHECK_EQ(r16.rx_16bit.rw_echo, 1U);
  CHECK_EQ(r16.rx_16bit.data, 0xA55AU);

  SPIFrame r22{};
  r22.word = (0x01U << 22) | 0x2ABCDEU;
  CHECK_EQ(r22.rx_common.reply_mode, 1U);
  CHECK_EQ(r22.rx_22bit.data, 0x2ABCDEU);
}

TEST_CASE("critical fault flags decode bit 7 (1V5 ok) down to bit 0 (reference clock missing)") {
  SPIFrame f{};
  f.word = (0x2U << 22) | 0b1010'0001U;
  const auto flags = tle92466ed::CriticalFaultFlags::Extract(f);
  CHECK(flags.supply_1v5_ok);
  CHECK(!flags.supply_2v5_ok);
  CHECK(flags.adc_bandgap_ok);
  CHECK(!flags.clk_too_slow);
  CHECK(flags.wd_ref_clk_missing);
}
