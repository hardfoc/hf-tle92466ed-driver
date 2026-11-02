/**
 * @file mock_tle92466ed.hpp
 * @brief A register model of the TLE92466ED behind the CRTP SPI interface, for the host unit tests.
 *
 * The model follows the datasheet's SPI protocol (Rev. 1.2, section 5.2):
 *
 * - every frame is 32 bits, MSB first, with a CRC-8 SAE J1850 byte in bits [31:24];
 * - the reply to a frame is clocked out during the **next** frame (one frame of pipeline);
 * - a write frame carries a 7-bit address in [23:17], a read frame a 16-bit address in [15:0];
 * - replies are 16-bit (reply mode 00B, with a 5-bit status) or 22-bit (01B, feedback registers);
 * - a frame with a bad CRC is answered by a 16-bit reply with status "Parity/CRC error".
 *
 * Faults are injected per test: a critical-fault state, an idle MISO line, a forced reply width.
 */
#pragma once

#include "tle92466ed.hpp"

#include <array>
#include <cstdarg>
#include <cstdint>
#include <vector>

namespace tle92466ed_test {

class MockTle92466ed : public tle92466ed::SpiInterface<MockTle92466ed> {
public:
  using Base = tle92466ed::SpiInterface<MockTle92466ed>;
  using Base::Log; // keep the driver's variadic Log() visible next to the va_list hook below

  template <typename T>
  using CommResult = tle92466ed::CommResult<T>;
  using CommError = tle92466ed::CommError;
  using CtrlPin = tle92466ed::CtrlPin;
  using GpioSignal = tle92466ed::GpioSignal;

  /// One frame as the chip saw it.
  struct Frame {
    uint32_t mosi;
    uint32_t miso;
  };

  explicit MockTle92466ed(uint16_t icvid = 0xC101) noexcept {
    regs_.fill(0);
    regs_[tle92466ed::CentralReg::ICVID] = icvid;
    pending_ = Reply16(0, false, 0);
  }

  // ---- The interface the driver needs
  // -------------------------------------------------------------
  CommResult<void> Init() noexcept {
    ready_ = true;
    return {};
  }
  CommResult<void> Deinit() noexcept {
    ready_ = false;
    return {};
  }
  CommResult<uint32_t> Transfer32(uint32_t tx) noexcept {
    const uint32_t rx = idle_miso_ ? 0U : pending_;
    pending_ = Process(tx);
    frames_.push_back({tx, rx});
    return rx;
  }
  CommResult<void> TransferMulti(std::span<const uint32_t> tx, std::span<uint32_t> rx) noexcept {
    if (tx.size() != rx.size()) {
      return tle::unexpected(CommError::InvalidParameter);
    }
    for (std::size_t i = 0; i < tx.size(); ++i) { // one CS window per 32-bit frame
      auto r = Transfer32(tx[i]);
      if (!r) {
        return tle::unexpected(r.error());
      }
      rx[i] = *r;
    }
    return {};
  }
  CommResult<void> Delay(uint32_t us) noexcept {
    elapsed_us_ += us;
    return {};
  }
  CommResult<void> Configure(const tle92466ed::SPIConfig& /*config*/) noexcept {
    return {};
  }
  bool IsReady() const noexcept {
    return ready_;
  }
  CommError GetLastError() const noexcept {
    return CommError::None;
  }
  CommResult<void> ClearErrors() noexcept {
    return {};
  }
  CommResult<void> GpioSet(CtrlPin pin, GpioSignal signal) noexcept {
    pins_[static_cast<uint8_t>(pin)] = signal;
    pin_log_.push_back({pin, signal});
    return {};
  }
  CommResult<GpioSignal> GpioRead(CtrlPin pin) noexcept {
    return pins_[static_cast<uint8_t>(pin)];
  }
  void Log(tle92466ed::LogLevel /*level*/, const char* /*tag*/, const char* /*fmt*/,
           va_list /*args*/) noexcept {}

  // ---- Test controls
  // -------------------------------------------------------------------------------
  std::array<uint32_t, 0x400>& regs() noexcept {
    return regs_;
  }
  const std::vector<Frame>& frames() const noexcept {
    return frames_;
  }
  void clearFrames() noexcept {
    frames_.clear();
  }
  struct PinEvent {
    CtrlPin pin;
    GpioSignal signal;
  };
  const std::vector<PinEvent>& pinLog() const noexcept {
    return pin_log_;
  }
  void setPin(CtrlPin pin, GpioSignal s) noexcept {
    pins_[static_cast<uint8_t>(pin)] = s;
  }
  /// Every reply becomes a critical fault frame carrying @p flags (bits 7..0).
  void setCriticalFault(bool on, uint8_t flags = 0x00) noexcept {
    critical_ = on;
    fault_flags_ = flags;
  }
  /// MISO stays low: no device on the bus.
  void setIdleMiso(bool on) noexcept {
    idle_miso_ = on;
  }
  /// Answer every read with a 16-bit reply, whatever width the register has.
  void forceReply16(bool on) noexcept {
    force16_ = on;
  }
  /// Status code (bits 21:17) of the next 16-bit replies to writes.
  void setWriteStatus(uint8_t status) noexcept {
    write_status_ = status;
  }
  uint32_t elapsedUs() const noexcept {
    return elapsed_us_;
  }

  // ---- Frame builders, shared with the tests
  // ---------------------------------------------------------
  static uint32_t WithCrc(uint32_t word) noexcept {
    tle92466ed::SPIFrame f{};
    f.word = word & 0x00FFFFFFU;
    f.tx_fields.crc = tle92466ed::CalculateFrameCrc(f);
    return f.word;
  }
  static uint32_t Reply16(uint16_t data, bool rw, uint8_t status) noexcept {
    return WithCrc(static_cast<uint32_t>(data) | (rw ? 1U << 16 : 0U) |
                   (static_cast<uint32_t>(status & 0x1FU) << 17));
  }
  static uint32_t Reply22(uint32_t data) noexcept {
    return WithCrc((data & 0x3FFFFFU) | (0x1U << 22));
  }

private:
  uint32_t Process(uint32_t mosi) noexcept {
    tle92466ed::SPIFrame f{};
    f.word = mosi;
    if (critical_) {
      return WithCrc((0x2U << 22) | fault_flags_);
    }
    if (!tle92466ed::VerifyFrameCrc(f)) {
      return Reply16(0, false, static_cast<uint8_t>(tle92466ed::SPIStatus::CRC_ERROR));
    }
    if (f.tx_fields.rw == 1U) {
      const uint16_t addr = f.tx_fields.address;
      regs_[addr] = f.tx_fields.data;
      return Reply16(static_cast<uint16_t>(regs_[addr]), true, write_status_);
    }
    const uint16_t addr = static_cast<uint16_t>(f.tx_fields.data & 0x03FFU);
    if (!force16_ && tle92466ed::ExpectedReplyFor(addr) == tle92466ed::ExpectedReply::Bits22) {
      return Reply22(regs_[addr]);
    }
    return Reply16(static_cast<uint16_t>(regs_[addr]), false, 0);
  }

  std::array<uint32_t, 0x400> regs_{};
  std::array<GpioSignal, 3> pins_{GpioSignal::INACTIVE, GpioSignal::INACTIVE, GpioSignal::INACTIVE};
  std::vector<Frame> frames_;
  std::vector<PinEvent> pin_log_;
  uint32_t pending_{0};
  uint32_t elapsed_us_{0};
  uint8_t fault_flags_{0};
  uint8_t write_status_{0};
  bool ready_{false};
  bool critical_{false};
  bool idle_miso_{false};
  bool force16_{false};
};

} // namespace tle92466ed_test
