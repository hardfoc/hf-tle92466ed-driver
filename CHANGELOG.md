# Changelog

All notable changes to this driver are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- Host unit tests (`-DHF_TLE92466ED_BUILD_TESTS=ON`) against a register model of the TLE92466ED's SPI (frames,
  CRC-8, reply pipeline, reply widths, fault replies), and a CI workflow that runs them with GCC and Clang.

### Changed

- Documentation moved to the HardFOC MkDocs platform: new page set (quickstart, hardware, architecture, SPI
  protocol, current control, diagnostics, API guide, configuration, porting, testing), code snippets compiled by
  the host build from `tests/docs/`, Doxygen under `/api/`, versioned with mike. The Jekyll site files and the
  doxygen-awesome submodule are gone.
- Hardware facts corrected against the datasheet (Rev. 1.2): SPI clock up to 8 MHz (not 10 MHz), VBAT 6 V to 18 V
  functional (not 5.5 V to 41 V), VDD is a 5 V supply input (not an output), outputs are LOADx with an external
  freewheeling diode at FWDx.

### Fixed

- A critical-fault reply frame is now recognised by its reply mode before the CRC check. Its bits 31:24 are don't
  care (datasheet 5.2.3.5), so `Read()`, `ReadMulti()` and `Write()` could report `CRCError` instead of
  `CommError::BusError` / `DriverError::FaultDetected`. The unused `transferFrame()` path reports `FaultDetected`
  too (was `HardwareError`).
- **BREAKING (behaviour):** `GlobalConfig{}` defaults are now the configuration `Init()` writes: `crc_enabled` and
  `spi_watchdog_enabled` default to `false` (were `true`), `vbat_uv_voltage` / `vbat_ov_voltage` to 7 V / 40 V
  (were 4 V / 41 V). Code that relied on the old defaults to switch on the CRC check or the SPI watchdog must set
  those fields explicitly. `ConfigureGlobal(GlobalConfig{})` no longer arms the SPI watchdog by surprise.
- `ConfigureChannel()` returns `InvalidParameter` for `pwm_period_mantissa` above 255 or `pwm_period_exponent` above
  7 (datasheet 5.3, PERIOD register) instead of truncating them; `ConfigurePwmPeriodRaw()` rejects an exponent
  above 7.
- `cmake --install` now installs `src/tle92466ed.ipp`, which `tle92466ed.hpp` includes, so the installed package
  builds. Headers install under `include/hf_tle92466ed/inc/` and the exported target is `hf::tle92466ed`, the same
  name as the build-tree alias (it was `hf::hf_tle92466ed`, which never built from an install). A CTest check
  builds a `find_package()` consumer from a scratch install.
- The ESP32 example port holds the bus with `spi_device_acquire_bus()` across `TransferMulti()`, so another device
  on a shared bus cannot take the reply of a pipelined access (datasheet 5.2.3).
- Header and example comments give the datasheet SPI limits (5.1.4, Table 18): 8 MHz maximum clock, 600 ns CSN high
  time between frames, 250 ns CSN lead and lag.

## 1.0.0 (version in the build settings; not tagged)

- TLE92466ED support: Config and Mission Mode, per-channel ICC setpoint, PWM and dither, parallel pairs, averaged
  and batched current feedback, global and per-channel diagnostics, SPI watchdog, BIST and the ESP32-S3 examples.
