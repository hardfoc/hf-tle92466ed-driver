# ESP32 Examples Configuration Guide

## Overview

This guide explains all configuration files and settings for the TLE92466ED ESP32
examples, including hardware configuration, build settings, and test options.

## 📁 Configuration File Structure

```text
examples/esp32/
├── app_config.yml                    # Build metadata and app definitions
├── main/
│   ├── TLE92466ED_TestConfig.hpp     # Hardware configuration (SPI, pins, limits)
│   ├── DriverIntegrationTest.cpp     # Driver integration test suite
│   ├── SolenoidControlTest.cpp       # Real hardware solenoid control test
│   └── Esp32TleCommInterface.cpp    # ESP32 communication interface
├── sdkconfig                         # ESP-IDF SDK configuration
└── components/
    └── hf_tle92466ed/
        ├── CMakeLists.txt            # Component build config
        └── idf_component.yml         # Component metadata
```

---

## 1. Hardware Configuration (`main/TLE92466ED_TestConfig.hpp`)

### Purpose
**Centralized hardware-specific settings** for ESP32-C6 and TLE92466ED interface.

### Location
```text
examples/esp32/main/TLE92466ED_TestConfig.hpp
```

### Complete Configuration

```cpp
#pragma once

#include <cstdint>

namespace TLE92466ED_TestConfig {

//=============================================================================
// SPI PIN CONFIGURATION
//=============================================================================

struct SPIPins {
    static constexpr int MISO = 2;   // GPIO2 - SPI MISO
    static constexpr int MOSI = 7;   // GPIO7 - SPI MOSI
    static constexpr int SCLK = 6;   // GPIO6 - SPI Clock
    static constexpr int CS = 10;    // GPIO10 - Chip Select
};

//=============================================================================
// SPI PARAMETERS
//=============================================================================

struct SPIParams {
    static constexpr int FREQUENCY = 1000000;  // 1MHz SPI clock
    static constexpr int MODE = 0;             // SPI Mode 0 (CPOL=0, CPHA=0)
    static constexpr int QUEUE_SIZE = 7;       // Max transactions in queue
};

//=============================================================================
// CURRENT LIMITS
//=============================================================================

struct CurrentLimits {
    static constexpr uint16_t SINGLE_CHANNEL_MAX = 2000;   // 2A per channel
    static constexpr uint16_t PARALLEL_CHANNEL_MAX = 4000; // 4A in parallel mode
};

//=============================================================================
// HARDWARE SPECIFICATIONS
//=============================================================================

struct HardwareSpecs {
    static constexpr float SUPPLY_VOLTAGE_MIN = 8.0f;    // Minimum VBAT (V)
    static constexpr float SUPPLY_VOLTAGE_MAX = 28.0f;   // Maximum VBAT (V)
    static constexpr int TEMPERATURE_MAX = 150;          // Max junction temp (°C)
};

//=============================================================================
// TEST CONFIGURATION
//=============================================================================

struct TestConfig {
    static constexpr uint16_t TEST_CURRENT_STEPS[] = {100, 500, 1000, 1500, 2000};
    static constexpr size_t TEST_CURRENT_STEPS_COUNT = 
        sizeof(TEST_CURRENT_STEPS) / sizeof(TEST_CURRENT_STEPS[0]);
    static constexpr uint32_t TEST_DURATION_MS = 5000;
    static constexpr uint32_t DIAGNOSTICS_POLL_INTERVAL_MS = 100;
};

} // namespace TLE92466ED_TestConfig
```text

### Configuration Sections

#### SPI Pins (`SPIPins`)

| Setting | Default | Range | Notes |
|---------|---------|-------|-------|
| `MISO` | GPIO2 | Any GPIO | Data from TLE92466ED |
| `MOSI` | GPIO7 | Any GPIO | Data to TLE92466ED |
| `SCLK` | GPIO6 | Any GPIO | SPI clock output |
| `CS` | GPIO10 | Any GPIO | Chip select (active LOW) |

**ESP32-C6 GPIO Considerations**:
- Avoid GPIO0, GPIO8, GPIO9 (strapping pins)
- GPIO12-GPIO13: JTAG (avoid if debugging)
- GPIO15-GPIO20: USB Serial/JTAG
- Recommended: GPIO2-GPIO11 for peripherals

#### SPI Parameters (`SPIParams`)

| Setting | Default | Range | Notes |
|---------|---------|-------|-------|
| `FREQUENCY` | 1000000 | 100000-10000000 | SPI clock (Hz, datasheet: 0.1-10MHz) |
| `MODE` | 1 | 0-3 | SPI mode (TLE92466ED requires Mode 1: CPOL=0, CPHA=1) |
| `QUEUE_SIZE` | 7 | 1-10 | Transaction queue depth |

**SPI Mode 1** (per datasheet):
- CPOL = 0 (clock idle LOW)
- CPHA = 1 (sample on falling edge)
- **Required by TLE92466ED** - do not change!

**Frequency Guidelines**:
- **100kHz**: Debug/troubleshooting
- **1MHz**: Default, reliable (recommended)
- **2-4MHz**: Higher performance
- **8-10MHz**: Maximum (may require shorter cables, datasheet max: 10MHz)

#### Current Limits (`CurrentLimits`)

| Setting | Default | Range | Notes |
|---------|---------|-------|-------|
| `SINGLE_CHANNEL_MAX` | 2000 | 0-2000 | Max per channel (mA) |
| `PARALLEL_CHANNEL_MAX` | 4000 | 0-4000 | Parallel mode max (mA) |

**Safety Notes**:
- TLE92466ED maximum: 2A per channel
- Parallel mode: 4A across paired channels
- Driver enforces these limits
- Exceeding causes overcurrent fault

#### Hardware Specs (`HardwareSpecs`)

| Setting | Default | Range | Notes |
|---------|---------|-------|-------|
| `SUPPLY_VOLTAGE_MIN` | 5.5 | 5.5-41.0 | Minimum VBAT (V) |
| `SUPPLY_VOLTAGE_MAX` | 41.0 | 5.5-41.0 | Maximum VBAT (V) |
| `TEMPERATURE_MAX` | 150 | -40 to 150 | Max junction temp (°C) |

**Operating Range** (per datasheet):
- VBAT: 5.5V to 41V recommended operating (typical: 12V or 24V automotive)
- VDD: 5V input to TLE92466ED (powers central logic) - Required for IC operation
- VIO: 3.0V to 5.5V input (sets I/O voltage levels for SPI communication, typical: 3.3V or 5.0V)
- Temperature: -40°C to 150°C junction

#### Test Configuration (`TestConfig`)

| Setting | Default | Notes |
|---------|---------|-------|
| `TEST_CURRENT_STEPS` | {100, 500, 1000, 1500, 2000} | Current test points (mA) |
| `TEST_DURATION_MS` | 5000 | Test duration (ms) |
| `DIAGNOSTICS_POLL_INTERVAL_MS` | 100 | Polling interval (ms) |

**Used by test suites** for consistent test parameters.

### Modifying Hardware Configuration

#### Example: Different GPIO Pins

```cpp
struct SPIPins {
    static constexpr int MISO = 4;   // Changed to GPIO4
    static constexpr int MOSI = 5;   // Changed to GPIO5
    static constexpr int SCLK = 3;   // Changed to GPIO3
    static constexpr int CS = 11;    // Changed to GPIO11
};
```text

#### Example: Higher SPI Frequency

```cpp
struct SPIParams {
    static constexpr int FREQUENCY = 4000000;  // 4MHz (higher performance)
    static constexpr int MODE = 0;
    static constexpr int QUEUE_SIZE = 7;
};
```text

#### Example: Custom Test Currents

```cpp
struct TestConfig {
    static constexpr uint16_t TEST_CURRENT_STEPS[] = {250, 750, 1250, 1750};
    static constexpr size_t TEST_CURRENT_STEPS_COUNT = 4;
    static constexpr uint32_t TEST_DURATION_MS = 10000;  // 10 seconds
    static constexpr uint32_t DIAGNOSTICS_POLL_INTERVAL_MS = 50;  // 50ms
};
```text

---

## 2. Application Configuration (`app_config.yml`)

### Purpose
**Build metadata and app definitions** for the dynamic build system.

### Location
```text
examples/esp32/app_config.yml
```text

### Structure

```yaml
apps:
  driver_integration_test:
    description: "Comprehensive driver integration test suite (no hardware required)"
    source_file: "DriverIntegrationTest.cpp"
    category: "test"
    idf_versions: ["release/v5.5"]
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true
    documentation: "docs/readme_driver_integration_test.md"
```

### App Definition Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `description` | string | Yes | Human-readable description |
| `source_file` | string | Yes | Main source file in `main/` |
| `category` | string | Yes | Category: `demo`, `test`, `application` |
| `idf_versions` | list | Yes | Compatible ESP-IDF versions |
| `build_types` | list | Yes | Supported build types |
| `ci_enabled` | bool | Yes | Enable in CI/CD pipeline |
| `featured` | bool | No | Feature in documentation |
| `documentation` | string | No | Path to documentation file |

### Build Types

#### Debug
```yaml
build_types: ["Debug"]
```text
- **Optimization**: `-Og` (debug-friendly)
- **Symbols**: Full debug symbols
- **Assertions**: Enabled
- **Size**: Larger binaries
- **Speed**: Slower execution
- **Use**: Development, debugging

#### Release
```yaml
build_types: ["Release"]
```text
- **Optimization**: `-O2` (performance)
- **Symbols**: Minimal symbols
- **Assertions**: Disabled
- **Size**: Smaller binaries
- **Speed**: Faster execution
- **Use**: Production, benchmarking

### IDF Versions

```yaml
idf_versions:
  - "release/v5.5"    # ESP-IDF v5.5.x (recommended)
  - "release/v5.4"    # ESP-IDF v5.4.x (compatible)
  - "release/v5.3"    # ESP-IDF v5.3.x (minimum)
```text

**Requirements**:
- **Minimum**: ESP-IDF v5.3 (C++23 support)
- **Recommended**: ESP-IDF v5.5 (latest features)

### Adding a New App

```yaml
apps:
  my_custom_app:
    description: "My custom TLE92466ED application"
    source_file: "MyCustomApp.cpp"
    category: "application"
    idf_versions: ["release/v5.5"]
    build_types: ["Debug", "Release"]
    ci_enabled: false
    featured: false
    documentation: "docs/README_MY_CUSTOM_APP.md"
```text

Then create:
```text
examples/esp32/main/MyCustomApp.cpp
examples/esp32/docs/README_MY_CUSTOM_APP.md
```text

---

## 3. Test Section Configuration

### Purpose
**Compile-time enable/disable** of test sections.

### Location
In each example source file (e.g., `DriverIntegrationTest.cpp`):

```cpp
//=============================================================================
// TEST CONFIGURATION
//=============================================================================

#define ENABLE_INITIALIZATION_TESTS 1
#define ENABLE_BASIC_OPERATION_TESTS 1
#define ENABLE_CURRENT_CONTROL_TESTS 1
```text

### Test Sections

| Section | Purpose | Tests |
|---------|---------|-------|
| `INITIALIZATION_TESTS` | HAL and driver setup | HAL init, driver init, chip ID |
| `BASIC_OPERATION_TESTS` | Core operations | Enable/disable, current setting, diagnostics |
| `CURRENT_CONTROL_TESTS` | Advanced control | Current ramping, multi-channel |

### Disabling Sections

Set to `0` to disable:

```cpp
#define ENABLE_INITIALIZATION_TESTS 1     // Enabled
#define ENABLE_BASIC_OPERATION_TESTS 1    // Enabled
#define ENABLE_CURRENT_CONTROL_TESTS 0    // DISABLED
```text

**Benefits**:
- Faster iteration during development
- Focus on specific features
- Reduce test time
- Save memory if needed

### Usage in Code

```cpp
RUN_TEST_SECTION_IF_ENABLED(
    ENABLE_INITIALIZATION_TESTS, "INITIALIZATION TESTS",
    // Tests only run if flag is 1
    RUN_TEST_IN_TASK("hal_init", test_hal_initialization, 8192, 5);
);
```text

---

## 4. ESP-IDF SDK Configuration (`sdkconfig`)

### Purpose
**ESP-IDF framework configuration** - compiler, components, features.

### Location
```text
examples/esp32/sdkconfig
```text

### Key Settings

#### Compiler Optimization

```ini
CONFIG_COMPILER_OPTIMIZATION_SIZE=n
CONFIG_COMPILER_OPTIMIZATION_PERF=y
CONFIG_COMPILER_OPTIMIZATION_DEBUG=n
```text

**Options**:
- `SIZE`: Optimize for size (`-Os`)
- `PERF`: Optimize for performance (`-O2`) **← Default**
- `DEBUG`: Optimize for debugging (`-Og`)

#### C++ Standard

```ini
CONFIG_COMPILER_CXX_STD_GNUXX23=y
```text

**Required**: TLE92466ED driver uses C++23 features (`std::expected`)

#### Log Level

```ini
CONFIG_LOG_DEFAULT_LEVEL_INFO=y
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=n
```text

**Options**:
- `NONE`: No logging
- `ERROR`: Errors only
- `WARN`: Warnings and errors
- `INFO`: Information, warnings, errors **← Default**
- `DEBUG`: Debug + all above
- `VERBOSE`: Verbose + all above

#### Stack Sizes

```ini
CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192
CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE=4096
```text

**Recommendations**:
- Main task: 8KB (default sufficient)
- Increase if stack overflow errors occur

#### FreeRTOS

```ini
CONFIG_FREERTOS_HZ=1000
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y
```text

**Important**:
- `HZ=1000`: 1ms tick resolution (for accurate timing)
- Trace facility: Useful for debugging

### Modifying sdkconfig

#### Via menuconfig (Interactive)

```bash
cd examples/esp32
idf.py menuconfig
```text

Navigate with arrow keys, space to select, 'S' to save.

#### Via sdkconfig.defaults

Create `sdkconfig.defaults`:

```ini
# Custom defaults
CONFIG_COMPILER_OPTIMIZATION_PERF=y
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=y
CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384
```text

Then:
```bash
idf.py fullclean
idf.py reconfigure
```text

---

## 5. Component Configuration

### Component CMakeLists.txt

**Location**: `examples/esp32/components/hf_tle92466ed/CMakeLists.txt`

```cmake
# Source resolution (automatic)
if(EXISTS "${PROJECT_ROOT}/src" AND EXISTS "${PROJECT_ROOT}/include")
    set(SRC_ROOT "${PROJECT_ROOT}")     # CI environment
else()
    get_filename_component(SRC_ROOT "${CMAKE_SOURCE_DIR}/../../" ABSOLUTE)
endif()

# Source files
set(TLE92466ED_SRCS
    "${SRC_ROOT}/src/tle92466ed.cpp"
)

# Component registration
idf_component_register(
    SRCS ${EXISTING_SRCS}
    INCLUDE_DIRS "${SRC_ROOT}/include"
    REQUIRES driver esp_timer freertos esp_driver_spi
)

# C++23 standard
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_23)
```text

**Key Points**:
- Automatically finds driver sources
- Handles CI and development paths
- C++23 required
- No manual source list maintenance

### Component Manifest (idf_component.yml)

**Location**: `examples/esp32/components/hf_tle92466ed/idf_component.yml`

```yaml
name: hf-tle92466ed-driver
version: "2.0.0"
description: TLE92466ED driver for ESP-IDF
dependencies:
  idf: ">=5.0.0"
targets:
  - esp32c6
  - esp32
  - esp32s3
  - esp32c3
```

**Purpose**: ESP-IDF Component Manager metadata

---

## 🔧 Configuration Workflows

### Development Setup

1. **Clone repository**
   ```bash
   git clone https://github.com/hardfoc/hf-tle92466ed-driver.git
   cd hf-tle92466ed-driver/examples/esp32
   ```
2. **Initialize submodules**
   ```bash
   git submodule update --init --recursive
   ```
3. **Configure hardware** (if needed)
   ```bash
   nano main/TLE92466ED_TestConfig.hpp
   # Modify SPI pins, frequency, etc.
   ```
4. **Set ESP-IDF target**
   ```bash
   idf.py set-target esp32c6
   ```
5. **Configure SDK** (optional)
   ```bash
   idf.py menuconfig
   ```
6. **Build**
   ```bash
   ./scripts/build_app.sh basic_usage Debug
   ```

### Production Setup

1. **Use Release build**
   ```bash
   ./scripts/build_app.sh basic_usage Release
   ```
2. **Optimize sdkconfig**
   ```ini
   CONFIG_COMPILER_OPTIMIZATION_PERF=y
   CONFIG_LOG_DEFAULT_LEVEL_WARN=y
   CONFIG_BOOTLOADER_LOG_LEVEL_WARN=y
   ```
3. **Disable debug features**
   ```cpp
   #define ENABLE_DEBUG_LOGGING 0
   #define ENABLE_STRESS_TESTS 0
   ```
4. **Verify settings**
   ```bash
   idf.py size
   idf.py size-components
   ```

### Troubleshooting Configuration

#### GPIO Conflict

**Error**: `GPIO already in use`

**Solution**: Change pins in `TLE92466ED_TestConfig.hpp`:
```cpp
struct SPIPins {
    static constexpr int MISO = 4;   // Try different GPIO
    // ...
};
```

#### SPI Communication Failure

**Error**: `Failed to initialize HAL`

**Solutions**:
1. Check frequency: Lower to 100kHz for testing
2. Verify wiring matches configuration
3. Try different SPI host (SPI2_HOST vs SPI3_HOST)

#### Stack Overflow

**Error**: `***ERROR*** A stack overflow in task`

**Solutions**:
1. Increase task stack in `RUN_TEST_IN_TASK`:
   ```cpp
   RUN_TEST_IN_TASK("test", test_func, 16384, 5);  // 16KB
   ```
2. Increase main stack in sdkconfig:
   ```ini
   CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384
   ```

#### Build Type Issues

**Error**: Unexpected optimization level

**Solution**:
```bash
# Clean and rebuild with explicit build type
idf.py fullclean
idf.py -DCMAKE_BUILD_TYPE=Debug build
```

---

## 📚 Configuration Reference

### File Purposes Summary

| File | Purpose | When to Modify |
|------|---------|----------------|
| `TLE92466ED_TestConfig.hpp` | Hardware settings | Different GPIO/hardware |
| `app_config.yml` | Build metadata | Add new app/example |
| `DriverIntegrationTest.cpp` (flags) | Test sections | Enable/disable tests |
| `sdkconfig` | ESP-IDF settings | Compiler/framework config |
| `components/.../CMakeLists.txt` | Component build | Never (auto-configured) |
| `components/.../idf_component.yml` | Component metadata | Version/dependency changes |

### Configuration Hierarchy

```text
Hardware Layer
└── TLE92466ED_TestConfig.hpp          (SPI pins, frequencies, limits)

Application Layer
├── app_config.yml                  (Build metadata, app list)
└── *Example.cpp                    (Test section flags)

Framework Layer
├── sdkconfig                       (ESP-IDF configuration)
└── components/                     (Component configuration)
```text

---

**Document Version**: 1.0.0  
**Last Updated**: 2025-10-21  
**Status**: ✅ Complete
