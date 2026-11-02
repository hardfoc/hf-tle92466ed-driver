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

## 1.0.0 (version in the build settings; not tagged)

- TLE92466ED support: Config and Mission Mode, per-channel ICC setpoint, PWM and dither, parallel pairs, averaged
  and batched current feedback, global and per-channel diagnostics, SPI watchdog, BIST and the ESP32-S3 examples.
