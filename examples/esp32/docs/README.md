# TLE92466ED ESP32 Examples Documentation

## 📚 Documentation Index

Complete documentation for TLE92466ED driver examples on ESP32-C6.

---

## 🎯 Quick Start

| Document | Purpose | When to Read |
|----------|---------|--------------|
| **[Driver Integration Test](readme_driver_integration_test.md)** | Driver API validation | Start here for driver testing |
| **[Solenoid Control Test](readme_solenoid_control_test.md)** | Real hardware solenoid testing | For actual solenoid operation |
| **[Configuration Guide](readme_configuration.md)** | All configuration options | Before customizing |
| **[Test Framework Reference](README_TEST_FRAMEWORK.md)** | Framework API details | Writing custom tests |

---

## 📖 Documentation Structure

### 1. [Driver Integration Test Suite](readme_driver_integration_test.md)

**Guide to the driver integration test suite**

#### Contents
- ✅ **Test Framework Architecture** - How the test infrastructure works
- ✅ **Test Suite Structure** - All 13 test sections with 40+ tests
- ✅ **Hardware Setup** - Complete wiring diagrams and pin configurations
- ✅ **Configuration** - Hardware and test settings
- ✅ **Building and Running** - Build commands and workflows
- ✅ **Expected Output** - Full example outputs with explanations
- ✅ **Test Coverage** - Complete API coverage breakdown
- ✅ **Individual Test Details** - Deep dive into each test
- ✅ **Telemetry Testing** - Telemetry validation
- ✅ **Troubleshooting** - Common issues and solutions
- ✅ **Performance Characteristics** - Timing and memory usage

**Start here for driver API validation (no hardware required)!**

---

### 2. [Solenoid Control Test](readme_solenoid_control_test.md)

**Complete guide to real hardware solenoid testing**

#### Contents
- ✅ **Hardware Requirements** - Complete component list and specifications
- ✅ **System Architecture** - Block diagrams and control flow
- ✅ **Hardware Setup** - Detailed wiring diagrams and connections
- ✅ **ADC Configuration** - Analog input setup and calibration
- ✅ **Solenoid Configuration** - Single channel and parallel pair setup
- ✅ **Control Algorithm** - Current calculation and regulation
- ✅ **Real-Time Telemetry** - Monitoring and display
- ✅ **Operation** - Startup sequence and control loop
- ✅ **Troubleshooting** - Hardware-specific issues and solutions
- ✅ **Safety Considerations** - Electrical and operational safety

**Use this for real hardware testing with actual solenoids!**

---

### 3. [Configuration Guide](readme_configuration.md)

**Complete reference for all configuration files and options**

#### Contents
- ✅ **Hardware Configuration** (`TLE92466ED_TestConfig.hpp`) - SPI pins, frequencies, current limits
- ✅ **Application Configuration** (`app_config.yml`) - Build metadata and app definitions
- ✅ **Test Section Configuration** - Compile-time test enable/disable
- ✅ **ESP-IDF SDK Configuration** (`sdkconfig`) - Framework settings
- ✅ **Component Configuration** - ESP-IDF component setup
- ✅ **Configuration Workflows** - Development and production setups
- ✅ **Troubleshooting** - Configuration-related issues

**Read this when you need to customize hardware or build settings.**

---

### 4. [Test Framework Reference](README_TEST_FRAMEWORK.md)

**Detailed API reference for the TLE92466ED Test Framework**

#### Contents
- ✅ **GPIO14 Progress Indicator** - Visual feedback system
- ✅ **Test Result Tracking** - Automatic pass/fail counting
- ✅ **Test Execution Macros** - `RUN_TEST`, `RUN_TEST_IN_TASK`, sections
- ✅ **Usage Guide** - How to write test functions
- ✅ **Advanced Usage** - Custom contexts, stack sizes, timeouts
- ✅ **Best Practices** - Test organization
- ✅ **Performance Impact** - Memory and timing overhead
- ✅ **CI/CD Integration** - Automated testing

**Essential reading for creating custom tests or understanding the framework.**

---

## 🚀 Getting Started

### Prerequisites

- **Hardware**:
  - ESP32-C6-DevKitC-1 development board
  - TLE92466ED evaluation board or custom PCB
  - 12-24V power supply (2A minimum)
  - Test load (solenoid, resistor, or LED)
  - USB-C cable for programming

- **Software**:
  - ESP-IDF v5.3 or later (v5.5 recommended)
  - Python 3.8+
  - Git with submodules support
  - Serial terminal (screen, minicom, or built-in monitor)

### Quick Start Steps

1. **Clone and setup**:
   ```bash
   git clone https://github.com/hardfoc/hf-tle92466ed-driver.git
   cd hf-tle92466ed-driver/examples/esp32
   git submodule update --init --recursive
   ```

2. **Configure ESP-IDF**:
   ```bash
   . $IDF_PATH/export.sh
   idf.py set-target esp32c6
   ```

3. **Build driver integration test**:
   ```bash
   ./scripts/build_app.sh driver_integration_test Debug
   ```

4. **Flash and monitor**:
   ```bash
   ./scripts/flash_app.sh driver_integration_test Debug
   ./scripts/monitor_app.sh driver_integration_test
   ```

5. **See results**:
   - Serial output shows test execution
   - GPIO14 toggles with test progress
   - Summary printed at end

---

## 📊 Example Overview

### Driver Integration Test

**Purpose**: Driver API validation without hardware requirements

**Test Sections** (13 total):
1. **INITIALIZATION_TESTS** - HAL setup, driver init, chip ID verification
2. **MODE_CONTROL_TESTS** - Mode transitions and state management
3. **GLOBAL_CONFIGURATION_TESTS** - Global device configuration
4. **CHANNEL_CONTROL_TESTS** - Channel enable/disable and mode configuration
5. **CURRENT_CONTROL_TESTS** - Current setpoint control and ramping
6. **PWM_CONFIGURATION_TESTS** - PWM period configuration
7. **DITHER_CONFIGURATION_TESTS** - Dither configuration
8. **DIAGNOSTICS_TESTS** - Telemetry and diagnostics
9. **FAULT_MANAGEMENT_TESTS** - Fault handling
10. **WATCHDOG_TESTS** - Watchdog functionality
11. **GPIO_CONTROL_TESTS** - GPIO pin control
12. **MULTI_CHANNEL_TESTS** - All channels individually tested
13. **PARALLEL_OPERATION_TESTS** - Parallel pair configuration
14. **ERROR_CONDITION_TESTS** - Error handling validation

**Features**:
- ✅ 40+ individual tests
- ✅ Complete API coverage
- ✅ FreeRTOS task-based test execution
- ✅ GPIO14 progress indicator
- ✅ Automatic result tracking
- ✅ Telemetry testing
- ✅ Formatted output
- ✅ No actual solenoids required


**Read**: [Driver Integration Test](readme_driver_integration_test.md)

---

### Solenoid Control Test

**Purpose**: Real hardware solenoid testing with ADC-based current control

**Features**:
- ✅ Two solenoids: single channel and parallel pair
- ✅ ADC-based control (0-3.3V → 0-100% current)
- ✅ Independent current ranges per solenoid
- ✅ Real-time telemetry (every 1 second)
- ✅ Continuous operation
- ✅ Fault monitoring


**Hardware Required**: TLE92466ED IC + Two Solenoids + Power Supplies

**Read**: [Solenoid Control Test](readme_solenoid_control_test.md)

---

## 🔧 Configuration Overview

### Hardware Configuration

**File**: `main/TLE92466ED_TestConfig.hpp`

**What's configured**:
- SPI GPIO pins (MISO, MOSI, SCLK, CS)
- SPI frequency and mode
- Current limits (single and parallel)
- Hardware specifications (voltage, temperature)
- Test parameters

**Example**:
```cpp
struct SPIPins {
    static constexpr int MISO = 2;   // GPIO2
    static constexpr int MOSI = 7;   // GPIO7
    static constexpr int SCLK = 6;   // GPIO6
    static constexpr int CS = 10;    // GPIO10
};
```text

**Read**: [Configuration Guide - Hardware Configuration](readme_configuration.md#1-hardware-configuration-maintle92466ed_confighpp)

---

### Build Configuration

**File**: `app_config.yml`

**What's configured**:
- Available applications (basic_usage, multi_channel, etc.)
- Build types (Debug, Release)
- ESP-IDF version compatibility
- CI/CD settings
- Documentation links

**Example**:
```yaml
apps:
  basic_usage:
    description: "Basic usage example for TLE92466ED"
    source_file: "BasicUsageExample.cpp"
    build_types: ["Debug", "Release"]
```text

**Read**: [Configuration Guide - Application Configuration](readme_configuration.md#2-application-configuration-app_configyml)

---

### Test Configuration

**File**: Each example source file (e.g., `BasicUsageExample.cpp`)

**What's configured**:
- Test section enable/disable flags
- Compile-time test selection

**Example**:
```cpp
#define ENABLE_INITIALIZATION_TESTS 1     // Enable
#define ENABLE_BASIC_OPERATION_TESTS 1    // Enable
#define ENABLE_CURRENT_CONTROL_TESTS 0    // Disable
```text

**Read**: [Configuration Guide - Test Section Configuration](readme_configuration.md#3-test-section-configuration)

---

## 🧪 Test Framework Features

The examples use a test framework with:

#### ✅ FreeRTOS Task-Based Execution
- Each test runs in isolated task
- Custom stack size per test (default: 8KB)
- Automatic semaphore synchronization
- 30-second timeout protection

#### ✅ GPIO14 Progress Indicator
- **Visual feedback** on oscilloscope/logic analyzer
- **Toggle**: HIGH/LOW on test completion
- **Blink patterns**: 5 blinks at section boundaries
- Hardware-level progress tracking

#### ✅ Automatic Result Tracking
- Pass/fail counting
- Microsecond-precision execution timing
- Success rate calculation
- Formatted summaries

#### ✅ Test Section Management
- Compile-time section enable/disable
- Organized test grouping
- Custom blink patterns per section

**Read**: [Test Framework Reference](README_TEST_FRAMEWORK.md)

---

## 📈 Example Output

### Test Execution

```text
╔══════════════════════════════════════════════════════════════════╗
║         TLE92466ED BASIC USAGE TEST SUITE - ESP32-C6            ║
╚══════════════════════════════════════════════════════════════════╝
Target: esp32c6
ESP-IDF Version: v5.5.0

╔══════════════════════════════════════════════════════════════════╗
║ SECTION: INITIALIZATION TESTS                                    ║
╚══════════════════════════════════════════════════════════════════╝
[GPIO14: Blink pattern - 5 blinks]

╔══════════════════════════════════════════════════════════════════╗
║ Running (task): hal_initialization                               ║
╚══════════════════════════════════════════════════════════════════╝
I (1234) TLE92466ED_Basic: Creating HAL instance...
I (1235) TLE92466ED_Basic: ✅ HAL initialized successfully
[SUCCESS] PASSED (task): hal_initialization (12.34 ms)
[GPIO14: Toggle HIGH]

... (more tests) ...

╔══════════════════════════════════════════════════════════════════╗
║                    TEST RESULTS SUMMARY                          ║
╠══════════════════════════════════════════════════════════════════╣
║  Total Tests:      7                                             ║
║  Passed:           7                                             ║
║  Failed:           0                                             ║
║  Success Rate:     100.00%                                       ║
║  Total Time:       234.56 ms                                     ║
╚══════════════════════════════════════════════════════════════════╝

I (5000) TLE92466ED_Basic: ✅ ALL TESTS PASSED!
```text

---

## 🐛 Troubleshooting Quick Reference

### Common Issues

| Issue | Quick Fix | Full Guide |
|-------|-----------|------------|
| **All tests fail** | Check VIO power (3.0-5.5V, typical 3.3V) and wiring | [Driver Integration Test - Troubleshooting](readme_driver_integration_test.md#troubleshooting) |
| **HAL init fails** | Verify GPIO pins in `TLE92466ED_TestConfig.hpp` | [Configuration - GPIO Conflict](readme_configuration.md#gpio-conflict) |
| **Chip ID read fails** | Check SPI wiring, CS pin, RESN pin | [Driver Integration Test - Troubleshooting](readme_driver_integration_test.md#troubleshooting) |
| **Solenoids not working** | Check VBAT, EN pin, channel enable | [Solenoid Control Test - Troubleshooting](readme_solenoid_control_test.md#troubleshooting) |
| **Stack overflow** | Increase stack size in `RUN_TEST_IN_TASK` | [Configuration - Stack Overflow](readme_configuration.md#stack-overflow) |
| **Build errors** | Clean and rebuild: `idf.py fullclean && idf.py build` | [Configuration - Build Type Issues](readme_configuration.md#build-type-issues) |

---

## 📦 Project Structure

```text
examples/esp32/
├── app_config.yml                  # Build metadata
├── CMakeLists.txt                  # Main CMake configuration
├── sdkconfig                       # ESP-IDF SDK configuration
│
├── main/                           # Application source
│   ├── CMakeLists.txt             # Main component build
│   ├── TLE92466ED_TestConfig.hpp   # Hardware configuration
│   ├── TLE92466ED_TestFramework.hpp  # Test infrastructure
│   ├── ESP32C6_HAL.hpp/.cpp       # ESP32-C6 HAL implementation
│   ├── DriverIntegrationTest.cpp  # Driver integration test suite
│   └── SolenoidControlTest.cpp    # Real hardware solenoid test
│
├── components/                     # ESP-IDF components
│   └── hf_tle92466ed/              # TLE92466ED driver component
│       ├── CMakeLists.txt
│       ├── idf_component.yml
│       └── README.md
│
├── docs/                           # Documentation
│   ├── README.md                          # This file
│   ├── readme_driver_integration_test.md  # Driver integration test guide
│   ├── readme_solenoid_control_test.md    # Solenoid control test guide
│   ├── readme_configuration.md            # Configuration reference
│   └── README_TEST_FRAMEWORK.md           # Test framework API
│
└── scripts/                        # Build/flash scripts (submodule)
    ├── build_app.sh
    ├── flash_app.sh
    └── monitor_app.sh
```text

---

## 🔗 Related Documentation

### Driver Documentation
- [Driver Overview](../../../README.md) - Main driver README
- [Driver API Reference](../../../docs/07_Driver_API.md) - Complete API docs
- [HAL Implementation Guide](../../../docs/08_HAL_Implementation.md) - Porting guide
- [Register Map](../../../docs/03_Register_Map.md) - TLE92466ED registers
- [Diagnostics](../../../docs/06_Diagnostics.md) - Fault detection

### ESP32 Resources
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [ESP32-C6 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)

### TLE92466ED Resources
- [TLE92466ED Datasheet](../../../Datasheet/)
- [Infineon Product Page](https://www.infineon.com/cms/en/product/power/motor-control-ics/intelligent-motor-control-ics/multi-half-bridge-ics/tle92466ed/)

---

## 🎓 Learning Path

### Beginner Path

1. **Read**: [Driver Integration Test](readme_driver_integration_test.md)
2. **Build**: Driver integration test (Debug mode)
3. **Run**: Flash to ESP32-C6 and observe output
4. **Verify**: All tests should pass (100% success rate)
5. **Explore**: Review telemetry tests and diagnostics

### Intermediate Path

1. **Read**: [Solenoid Control Test](readme_solenoid_control_test.md)
2. **Setup**: Connect solenoids and power supplies
3. **Configure**: Adjust solenoid current ranges if needed
4. **Test**: Run solenoid control test with real hardware
5. **Monitor**: Observe real-time telemetry and current regulation

### Advanced Path

1. **Read**: [Test Framework Reference](README_TEST_FRAMEWORK.md)
2. **Extend**: Add custom test functions
3. **Port**: Implement HAL for different MCU (ESP32-S3, etc.)
4. **Integrate**: Set up CI/CD pipeline with test parsing
5. **Optimize**: Profile and optimize for your application

---

## 💡 Tips and Best Practices

### Development Tips

1. **Start with Debug builds** for detailed logging
2. **Use GPIO14 indicator** to track progress without serial
3. **Monitor SPI signals** with logic analyzer during bring-up
4. **Test with known-good load** (resistor) before real solenoids
5. **Enable all test sections** initially, disable as needed

### Configuration Tips

1. **Keep hardware config centralized** in `TLE92466ED_TestConfig.hpp`
2. **Use compile-time flags** for test sections (faster than runtime)
3. **Document custom settings** in code comments
4. **Version control sdkconfig.defaults** for team consistency
5. **Use Release builds** for final performance testing

### Testing Tips

1. **Run full test suite** after any changes
2. **Check GPIO14 patterns** on oscilloscope for test flow
3. **Analyze execution times** to find performance issues
4. **Test with realistic loads** matching your application
5. **Monitor diagnostics** for fault conditions

---

## 🆘 Getting Help

### Documentation Issues

If documentation is unclear or incomplete:
1. Check the specific guide (Basic Usage, Configuration, Test Framework)
2. Search for keywords in all docs
3. Review related driver documentation
4. Open GitHub issue with details

### Build/Configuration Issues

If you have build or configuration problems:
1. Check [Configuration Guide - Troubleshooting](readme_configuration.md#troubleshooting-configuration)
2. Verify ESP-IDF version (v5.3+)
3. Clean and rebuild: `idf.py fullclean && idf.py build`
4. Check `sdkconfig` for conflicts

### Hardware/Test Issues

If tests fail or hardware doesn't work:
1. **Driver Integration Test**: Check [Driver Integration Test - Troubleshooting](readme_driver_integration_test.md#troubleshooting)
2. **Solenoid Control Test**: Check [Solenoid Control Test - Troubleshooting](readme_solenoid_control_test.md#troubleshooting)
3. Verify wiring against pin configuration
4. Measure power supplies (VDD: 5V for central logic; VIO: 3.0-5.5V for SPI I/O levels; VBAT: 5.5-41V for solenoids)
5. Test SPI communication with logic analyzer
6. Try lower SPI frequency (100kHz) for debugging

### Support Channels

- **GitHub Issues**: Bug reports, feature requests
- **GitHub Discussions**: Questions, general help
- **Driver Documentation**: Technical reference

---

## 📊 Example Statistics

### Driver Integration Test

| Metric | Value |
|--------|-------|
| **Total Tests** | 40+ |
| **Test Sections** | 13 |
| **Typical Execution Time** | 10-15 seconds |
| **Flash Size** | ~350KB |
| **RAM Usage** | ~15KB |
| **Stack per Task** | 8KB |

### Test Breakdown

| Section | Tests | Time |
|---------|-------|------|
| **Initialization** | 5 | ~50-100ms |
| **Mode Control** | 3 | ~30-50ms |
| **Global Configuration** | 3 | ~200-300ms |
| **Channel Control** | 4 | ~500-800ms |
| **Current Control** | 2 | ~2000-3000ms |
| **PWM Configuration** | 2 | ~300-500ms |
| **Dither Configuration** | 2 | ~300-500ms |
| **Diagnostics** | 7 | ~1000-1500ms |
| **Fault Management** | 2 | ~100-200ms |
| **Watchdog** | 1 | ~200-300ms |
| **GPIO Control** | 1 | ~200-300ms |
| **Multi-Channel** | 1 | ~2000-3000ms |
| **Parallel Operation** | 1 | ~300-500ms |
| **Error Conditions** | 1 | ~100-200ms |

### Solenoid Control Test

| Metric | Value |
|--------|-------|
| **Control Update Rate** | 10 Hz (100ms) |
| **Telemetry Update Rate** | 1 Hz (1 second) |
| **ADC Sampling Rate** | 1 kHz |
| **Flash Size** | ~380KB |
| **RAM Usage** | ~18KB |
| **Stack** | 8KB |

---

## 🎉 Success Criteria

You've successfully set up the examples when:

- ✅ Build completes without errors
- ✅ Flash and monitor show output
- ✅ All tests pass (100% success rate)
- ✅ GPIO14 toggles during execution
- ✅ Test summary shows timing information
- ✅ System restarts automatically after tests

Example success output:
```text
╔══════════════════════════════════════════════════════════════════╗
║                    TEST RESULTS SUMMARY                          ║
╠══════════════════════════════════════════════════════════════════╣
║  Total Tests:      7                                             ║
║  Passed:           7                                             ║
║  Failed:           0                                             ║
║  Success Rate:     100.00%                                       ║
║  Total Time:       234.56 ms                                     ║
╚══════════════════════════════════════════════════════════════════╝

✅ ALL TESTS PASSED! System will restart in 10 seconds...
```text

---

**Documentation Version**: 1.0.0  
**Last Updated**: 2025-10-21  
**Status**: ✅ Complete

**Next**: Start with [Driver Integration Test](readme_driver_integration_test.md) →
