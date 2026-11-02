/**
 * @file my_spi.hpp
 * @brief The SPI port shown on the documentation's Quickstart and Porting pages, compiled by the host build.
 *
 * Every platform call goes through the `platform_*` functions declared below; a real port replaces them
 * with its SDK (ESP-IDF, STM32 HAL, Zephyr, Linux spidev). The class is what the driver needs, nothing more.
 */
#pragma once

#include "tle92466ed.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>

// Platform hooks: your SDK's calls. Declared only, so the docs build checks the port compiles.
bool platform_spi_init() noexcept;                                           // mode 1, MSB first, <= 8 MHz
bool platform_spi_transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept;  // CSN low .. high
void platform_spi_lock() noexcept;                                           // own the bus (shared buses)
void platform_spi_unlock() noexcept;
bool platform_gpio_write(int gpio, bool level) noexcept;
bool platform_gpio_read(int gpio, bool& level) noexcept;
void platform_delay_us(uint32_t us) noexcept;

// --8<-- [start:port]
class MySpi : public tle92466ed::SpiInterface<MySpi> {
public:
  template <typename T>
  using Result = tle92466ed::CommResult<T>;
  using Error = tle92466ed::CommError;
  using tle92466ed::SpiInterface<MySpi>::Log;  // keep the driver's printf-style Log() visible

  // SPI peripheral and the three GPIOs. The driver calls this first, from Init().
  Result<void> Init() noexcept {
    ready_ = platform_spi_init();
    return ready_ ? Result<void>{} : tle::unexpected(Error::HardwareNotReady);
  }
  Result<void> Deinit() noexcept {
    ready_ = false;
    return {};
  }

  // One 32-bit frame, MSB first, CSN low for exactly these four bytes.
  Result<uint32_t> Transfer32(uint32_t tx) noexcept {
    const uint8_t out[4] = {uint8_t(tx >> 24), uint8_t(tx >> 16), uint8_t(tx >> 8), uint8_t(tx)};
    uint8_t in[4] = {};
    if (!platform_spi_transfer(out, in, sizeof out)) {
      return tle::unexpected(Error::TransferError);
    }
    return (uint32_t(in[0]) << 24) | (uint32_t(in[1]) << 16) | (uint32_t(in[2]) << 8) | in[3];
  }

  // Several frames back to back: a CSN pulse per frame, the bus held for the whole sequence.
  Result<void> TransferMulti(std::span<const uint32_t> tx, std::span<uint32_t> rx) noexcept {
    if (tx.size() != rx.size()) {
      return tle::unexpected(Error::InvalidParameter);
    }
    platform_spi_lock();
    for (std::size_t i = 0; i < tx.size(); ++i) {
      auto r = Transfer32(tx[i]);
      if (!r) {
        platform_spi_unlock();
        return tle::unexpected(r.error());
      }
      rx[i] = *r;
    }
    platform_spi_unlock();
    return {};
  }

  // RESN and FAULTN are active low, EN is active high (datasheet pin table).
  Result<void> GpioSet(tle92466ed::CtrlPin pin, tle92466ed::GpioSignal s) noexcept {
    const bool active = (s == tle92466ed::GpioSignal::ACTIVE);
    const bool ok = (pin == tle92466ed::CtrlPin::RESN) ? platform_gpio_write(kResn, !active)
                    : (pin == tle92466ed::CtrlPin::EN) ? platform_gpio_write(kEn, active)
                                                       : false;  // FAULTN is an input
    return ok ? Result<void>{} : tle::unexpected(Error::InvalidParameter);
  }
  Result<tle92466ed::GpioSignal> GpioRead(tle92466ed::CtrlPin pin) noexcept {
    if (pin != tle92466ed::CtrlPin::FAULTN) {
      return tle::unexpected(Error::InvalidParameter);
    }
    bool level = true;
    if (!platform_gpio_read(kFaultn, level)) {
      return tle::unexpected(Error::BusError);
    }
    return level ? tle92466ed::GpioSignal::INACTIVE : tle92466ed::GpioSignal::ACTIVE;
  }

  Result<void> Delay(uint32_t us) noexcept {
    platform_delay_us(us);
    return {};
  }
  Result<void> Configure(const tle92466ed::SPIConfig& /*config*/) noexcept {
    return {};  // fixed bus settings in this port
  }
  bool IsReady() const noexcept {
    return ready_;
  }
  Error GetLastError() const noexcept {
    return Error::None;
  }
  Result<void> ClearErrors() noexcept {
    return {};
  }
  void Log(tle92466ed::LogLevel /*lvl*/, const char* tag, const char* fmt, va_list args) noexcept {
    std::printf("[%s] ", tag);
    std::vprintf(fmt, args);
    std::printf("\n");
  }

private:
  static constexpr int kResn = 6;    // your board's GPIO numbers
  static constexpr int kEn = 5;
  static constexpr int kFaultn = 16;
  bool ready_{false};
};
// --8<-- [end:port]
