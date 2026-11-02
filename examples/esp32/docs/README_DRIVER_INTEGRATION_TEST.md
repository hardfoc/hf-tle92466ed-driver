# Driver Integration Test Suite - TLE92466ED ESP32-C6

## Table of Contents

1. [Overview](#overview)
2. [Purpose](#purpose)
3. [Test Framework Architecture](#test-framework-architecture)
4. [Test Suite Structure](#test-suite-structure)
5. [Hardware Setup](#hardware-setup)
6. [Configuration](#configuration)
7. [Building and Running](#building-and-running)
8. [Expected Output](#expected-output)
9. [Test Coverage](#test-coverage)
10. [Individual Test Details](#individual-test-details)
11. [Troubleshooting](#troubleshooting)
12. [Performance Characteristics](#performance-characteristics)
13. [Related Documentation](#related-documentation)

---

## Overview

The **Driver Integration Test Suite** validates all functionality of the TLE92466ED driver without requiring actual solenoids or loads. This test suite provides API validation with **40+ individual tests** organized into **13 test sections**, covering all driver capabilities.

### Key Features

- **Complete API Coverage**: Tests all public driver functions
- **No Hardware Required**: Validates driver logic and SPI communication only
- **Test Framework**: FreeRTOS task-based execution with automatic result tracking
- **GPIO14 Progress Indicator**: Visual feedback on oscilloscope/logic analyzer
- **Telemetry Testing**: Validates all status and diagnostic reading functions
- **Error Condition Testing**: Verifies proper error handling and validation

---

## Purpose

This test suite serves multiple purposes:

1. **Driver Validation**: Ensures all driver APIs work correctly
2. **Regression Testing**: Detects breaking changes during development
3. **CI/CD Integration**: Automated testing in continuous integration pipelines
4. **Documentation**: Demonstrates proper usage of all driver functions
5. **Development Aid**: Helps developers understand driver capabilities
6. **Quality Assurance**: Validates driver behavior before hardware testing

**Note**: This test validates the driver and SPI communication but does not test actual solenoid operation. For real hardware testing with solenoids, use the **Solenoid Control Test**.

---

## Test Framework Architecture

The test suite uses the **TLE92466ED Test Framework** (`TLE92466ED_TestFramework.hpp`), which provides:

#### FreeRTOS Task-Based Execution
- Each test runs in isolated FreeRTOS task
- Custom stack size per test (8KB default)
- Automatic semaphore synchronization
- 30-second timeout protection
- Fallback to inline execution on failure

#### GPIO14 Progress Indicator
- Visual feedback on oscilloscope/logic analyzer
- **Toggle**: HIGH/LOW on each test completion
- **Blink Pattern**: 5 blinks at section start/end
- Hardware-level test progression tracking

#### Automatic Result Tracking
- Pass/fail counting
- Microsecond-precision execution timing
- Success rate calculation
- Summary reports

#### Test Section Management
- Compile-time section enable/disable
- Section-based organization
- Custom blink patterns per section
- Structured output formatting

---

## Test Suite Structure

This integration test suite validates all functionality of the TLE92466ED driver. The suite includes **13 test sections** with **40+ individual tests** covering all driver APIs.

### Test Sections

#### 1. INITIALIZATION_TESTS (`#define ENABLE_INITIALIZATION_TESTS 1`)
Tests fundamental setup and communication:
- `test_hal_initialization` - ESP32-C6 HAL setup and SPI configuration
- `test_driver_initialization` - TLE92466ED driver instance creation
- `test_chip_id` - SPI communication verification via chip ID
- `test_ic_version` - IC version register reading
- `test_device_verification` - Device ID verification

#### 2. MODE_CONTROL_TESTS (`#define ENABLE_MODE_CONTROL_TESTS 1`)
Tests mode transitions and state management:
- `test_enter_mission_mode` - Transition to Mission Mode
- `test_enter_config_mode` - Transition to Config Mode
- `test_mode_transitions` - Multiple mode transition cycles

#### 3. GLOBAL_CONFIGURATION_TESTS (`#define ENABLE_GLOBAL_CONFIG_TESTS 1`)
Tests global device configuration:
- `test_crc_control` - CRC enable/disable functionality
- `test_vbat_thresholds` - VBAT UV/OV threshold configuration (high-level and raw APIs)
- `test_global_configuration` - Complete global configuration

#### 4. CHANNEL_CONTROL_TESTS (`#define ENABLE_CHANNEL_CONTROL_TESTS 1`)
Tests channel enable/disable and mode configuration:
- `test_single_channel_control` - Single channel enable/disable
- `test_all_channels_control` - All channels enable/disable
- `test_channel_mask_control` - Channel mask-based control
- `test_channel_mode_configuration` - All channel modes (OFF, ICC, Direct Drive, etc.)

#### 5. CURRENT_CONTROL_TESTS (`#define ENABLE_CURRENT_CONTROL_TESTS 1`)
Tests current setpoint control:
- `test_current_setpoint` - Current setting with read-back verification (100mA, 500mA, 1000mA, 1500mA, 2000mA)
- `test_current_ramping` - Smooth 0→1000mA→0mA transitions

#### 6. PWM_CONFIGURATION_TESTS (`#define ENABLE_PWM_CONFIG_TESTS 1`)
Tests PWM period configuration:
- `test_pwm_period_configuration` - High-level API (period in microseconds)
- `test_pwm_period_raw` - Low-level API (mantissa, exponent, low_freq_range)

#### 7. DITHER_CONFIGURATION_TESTS (`#define ENABLE_DITHER_CONFIG_TESTS 1`)
Tests dither configuration:
- `test_dither_configuration` - High-level API (amplitude in mA, frequency in Hz)
- `test_dither_raw` - Low-level API (step_size, num_steps, flat_steps)

#### 8. DIAGNOSTICS_TESTS (`#define ENABLE_DIAGNOSTICS_TESTS 1`)
Tests diagnostics and monitoring:
- `test_device_status` - Global device status reading
- `test_channel_diagnostics` - Channel-specific diagnostics
- `test_voltage_reading` - VBAT and VIO voltage reading
- `test_current_reading` - Average current and duty cycle reading
- `test_all_channels_telemetry` - Telemetry for all 6 channels
- `test_device_telemetry` - Device telemetry
- `test_telemetry_with_active_channel` - Telemetry with active channel

#### 9. FAULT_MANAGEMENT_TESTS (`#define ENABLE_FAULT_MANAGEMENT_TESTS 1`)
Tests fault handling:
- `test_fault_clearing` - Fault flag clearing
- `test_software_reset` - Software reset functionality

#### 10. WATCHDOG_TESTS (`#define ENABLE_WATCHDOG_TESTS 1`)
Tests watchdog functionality:
- `test_spi_watchdog` - SPI watchdog reload with various values

#### 11. GPIO_CONTROL_TESTS (`#define ENABLE_GPIO_CONTROL_TESTS 1`)
Tests GPIO pin control:
- `test_gpio_control` - Reset, Enable, and Fault pin control

#### 12. MULTI_CHANNEL_TESTS (`#define ENABLE_MULTI_CHANNEL_TESTS 1`)
Tests all channels individually:
- `test_all_channels_individually` - All 6 channels tested with enable/disable and current control

#### 13. PARALLEL_OPERATION_TESTS (`#define ENABLE_PARALLEL_OPERATION_TESTS 1`)
Tests parallel channel operation:
- `test_parallel_operation` - Parallel pair configuration (CH0/CH3, CH1/CH2, CH4/CH5)

#### 14. ERROR_CONDITION_TESTS (`#define ENABLE_ERROR_CONDITION_TESTS 1`)
Tests error handling and validation:
- `test_error_conditions` - Wrong mode operations (should fail correctly)

### Test Function Signatures

All test functions follow this pattern:
```cpp
static bool test_function_name() noexcept;
```

**Key Characteristics**:
- Return `bool` (true = passed, false = failed)
- Declared `noexcept` for embedded safety
- Self-contained and isolated
- Descriptive ESP_LOG output
- Proper error handling with `std::expected`

---

## Hardware Setup

### Required Components

- **ESP32-C6-DevKitC-1** development board
- **TLE92466ED** evaluation board or custom PCB
- **5.5-41V power supply** for VBAT (2A minimum) - *Note: VBAT not required for SPI communication, but recommended for complete testing. Typical: 12V or 24V automotive*
- **3.0-5.5V power supply** for VIO (I/O level supply) - *Required for SPI communication (sets I/O voltage levels). Typical: 3.3V or 5.0V*
- **5V power supply** for VDD (internal logic supply) - *Required for IC operation*
- **Oscilloscope/Logic Analyzer** (optional, for GPIO14 monitoring)
- Connecting wires

### Wiring Diagram

```
┌─────────────────┐         ┌──────────────────┐
│   ESP32-C6      │         │   TLE92466ED     │
│                 │         │                  │
│  GPIO2  ───────┼────────>│  MISO            │
│  GPIO7  ───────┼────────>│  MOSI            │
│  GPIO6  ───────┼────────>│  SCLK            │
│  GPIO18 ───────┼────────>│  CS              │
│  GPIO21 ───────┼────────>│  RESN            │
│  GPIO20 ───────┼────────>│  EN              │
│  GPIO19 <──────┼─────────│  FAULTN          │
│  GPIO14 ───────┼────────>│  [LED/Scope]    │
│                 │         │                  │
│  5V     ───────┼────────>│  VDD             │
│  3.3V   ───────┼────────>│  VIO             │
│  GND    ───────┼────────>│  GND             │
└─────────────────┘         └──────────────────┘
                                     │
                            ┌────────┴────────┐
                            │                 │
                    External Supply      Test Load
                    ──────────────      ──────────
                    12-24V ──────> VBAT  OUT0 ──> Load
                    GND   ──────> GND   GND  ──> Return
```

### Complete Pin Configuration

| Function | ESP32-C6 GPIO | TLE92466ED Pin | Configuration | Notes |
|----------|---------------|----------------|---------------|-------|
| **SPI MISO** | GPIO2 | MISO | Input | Data from TLE92466ED |
| **SPI MOSI** | GPIO7 | MOSI | Output | Data to TLE92466ED |
| **SPI SCLK** | GPIO6 | SCLK | Output | 1MHz SPI Clock |
| **SPI CS** | GPIO18 | CS | Output | Active Low |
| **RESN** | GPIO21 | RESN | Output | Reset pin (active low) |
| **EN** | GPIO20 | EN | Output | Enable pin (active high) |
| **FAULTN** | GPIO19 | FAULTN | Input | Fault pin (active low) |
| **Test Indicator** | GPIO14 | - | Output | Progress indicator |
| **VDD Supply** | 5V | VDD | Power | 5V (Required for IC operation) |
| **VIO Supply** | 3.3V/5V | VIO | Power | 3.0-5.5V (Required for SPI) |
| **Load Supply** | External | VBAT | Power | 5.5-41V, 2A+ (Optional for SPI) |
| **Ground** | GND | GND | Ground | Common reference |

### GPIO14 Test Indicator Setup

```
ESP32-C6 GPIO14 ──┬── LED (Anode)
                  │   └── LED (Cathode) ── 220Ω ── GND
                  │
                  └── Oscilloscope/Logic Analyzer Probe
```

**Purpose**: Visual and measurable feedback showing test progression

### Power Supply Requirements

```
┌─────────────────────────────────────────────────┐
│  Power Supply Requirements                      │
├─────────────────────────────────────────────────┤
│                                                 │
│  VDD (Internal Logic Supply):                   │
│    └─ 5V input to TLE92466ED                    │
│    └─ Powers central logic of the IC            │
│    └─ Required for IC operation                  │
│                                                 │
│  VIO (I/O Level Supply):                        │
│    └─ 3.0V to 5.5V (input)                       │
│    └─ Sets I/O voltage levels for SPI           │
│    └─ Required for SPI communication             │
│    └─ Typical: 3.3V or 5.0V                     │
│    └─ Typical current: 10-20mA                  │
│                                                 │
│  VBAT (Load Supply):                            │
│    └─ 5.5V to 41V (recommended operating)       │
│    └─ Typical: 12V or 24V automotive            │
│    └─ NOT required for SPI communication        │
│    └─ Required for actual load operation        │
│    └─ Typical: 2A+ per channel                   │
│                                                 │
└─────────────────────────────────────────────────┘
```

**Important**: For this driver integration test, **VBAT is not required**. The test validates SPI communication and driver logic only. **VDD is required** (powers central logic). **VIO is required for SPI communication** (sets I/O voltage levels).

---

## Configuration

### Hardware Configuration (`main/TLE92466ED_TestConfig.hpp`)

All hardware-specific settings are centralized:

```cpp
namespace TLE92466ED_TestConfig {

struct SPIPins {
    static constexpr uint8_t MISO = 2;   // GPIO2
    static constexpr uint8_t MOSI = 7;   // GPIO7
    static constexpr uint8_t SCLK = 6;   // GPIO6
    static constexpr uint8_t CS = 18;    // GPIO18
};

struct ControlPins {
    static constexpr uint8_t RESN = 21;   // GPIO21
    static constexpr uint8_t EN = 20;     // GPIO20
    static constexpr uint8_t FAULTN = 19; // GPIO19
    static constexpr uint8_t DRV0 = 22;   // GPIO22
    static constexpr uint8_t DRV1 = 23;   // GPIO23
};

struct SPIParams {
    static constexpr uint32_t FREQUENCY = 1000000;  // 1MHz
    static constexpr uint8_t MODE = 1;              // SPI Mode 1 (CPOL=0, CPHA=1)
    static constexpr uint8_t QUEUE_SIZE = 1;
    static constexpr uint8_t CS_ENA_PRETRANS = 1;
    static constexpr uint8_t CS_ENA_POSTTRANS = 1;
};

} // namespace TLE92466ED_TestConfig
```

**To modify hardware settings**: Edit `main/TLE92466ED_TestConfig.hpp`

### Test Configuration (`main/DriverIntegrationTest.cpp`)

Enable/disable test sections at compile time:

```cpp
// Enable/disable test sections (set to 0 to skip a section)
#define ENABLE_INITIALIZATION_TESTS 1
#define ENABLE_MODE_CONTROL_TESTS 1
#define ENABLE_GLOBAL_CONFIG_TESTS 1
#define ENABLE_CHANNEL_CONTROL_TESTS 1
#define ENABLE_CURRENT_CONTROL_TESTS 1
#define ENABLE_PWM_CONFIG_TESTS 1
#define ENABLE_DITHER_CONFIG_TESTS 1
#define ENABLE_DIAGNOSTICS_TESTS 1
#define ENABLE_FAULT_MANAGEMENT_TESTS 1
#define ENABLE_WATCHDOG_TESTS 1
#define ENABLE_GPIO_CONTROL_TESTS 1
#define ENABLE_MULTI_CHANNEL_TESTS 1
#define ENABLE_PARALLEL_OPERATION_TESTS 1
#define ENABLE_ERROR_CONDITION_TESTS 1
```

**To disable a section**: Set to `0` and rebuild. This allows you to run only specific test categories during development or debugging.

### Build Configuration (`app_config.yml`)

Application metadata and build settings:

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
```

**Note**: This is for build metadata only. Hardware config is in `TLE92466ED_TestConfig.hpp`.

---

## Building and Running

### Quick Start

```bash
cd examples/esp32

# Build the driver integration test
./scripts/build_app.sh driver_integration_test Debug

# Flash to ESP32-C6
./scripts/flash_app.sh driver_integration_test Debug

# Monitor output
./scripts/monitor_app.sh driver_integration_test
```

### Manual Build

```bash
cd examples/esp32

# Set target
idf.py set-target esp32c6

# Configure (optional)
idf.py menuconfig

# Build with APP_TYPE=driver_integration_test
idf.py -DAPP_TYPE=driver_integration_test build

# Flash
idf.py -p /dev/ttyUSB0 flash

# Monitor
idf.py -p /dev/ttyUSB0 monitor
```

### Build Targets

```bash
# Debug build (optimized for debugging)
idf.py -DAPP_TYPE=driver_integration_test -DCMAKE_BUILD_TYPE=Debug build

# Release build (optimized for performance)
idf.py -DAPP_TYPE=driver_integration_test -DCMAKE_BUILD_TYPE=Release build
```

---

## Expected Output

### Test Header

```
╔══════════════════════════════════════════════════════════════════════════════╗
║         TLE92466ED DRIVER INTEGRATION TEST SUITE - ESP32-C6                 ║
║                         HardFOC Core Drivers                              ║
╚══════════════════════════════════════════════════════════════════════════════╝
Target: esp32c6
ESP-IDF Version: v5.5.1

╔══════════════════════════════════════════════════════════════════════════════╗
║ TLE92466ED TEST CONFIGURATION                                                
╠══════════════════════════════════════════════════════════════════════════════╣
║ Test sections will execute based on compile-time configuration              ║
║ GPIO14 test progression indicator: ENABLED                                   ║
╚══════════════════════════════════════════════════════════════════════════════╝
```

### Test Execution

```
╔══════════════════════════════════════════════════════════════════════════════╗
║ SECTION: INITIALIZATION TESTS                                                ║
╚══════════════════════════════════════════════════════════════════════════════╝
[GPIO14: Blink pattern - 5 blinks]

╔══════════════════════════════════════════════════════════════════════════════╗
║ Running (task): hal_initialization                                           ║
╚══════════════════════════════════════════════════════════════════════════════╝
I (1234) TLE92466ED_Test: Creating HAL instance...
I (1235) Esp32TleComm: Esp32TleCommInterface created with SPI config
I (1236) Esp32TleComm:   MISO: GPIO2, MOSI: GPIO7, SCLK: GPIO6, CS: GPIO18
I (1237) Esp32TleComm:   Frequency: 1000000 Hz, Mode: 1
I (1238) TLE92466ED_Test: Initializing HAL...
I (1240) Esp32TleComm: Esp32TleCommInterface initialized successfully
I (1241) TLE92466ED_Test: ✅ HAL initialized successfully
[SUCCESS] PASSED (task): hal_initialization (12.34 ms)
Test task completed: hal_initialization
[GPIO14: Toggle HIGH]
```

### Test Results Summary

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                           TEST RESULTS SUMMARY                               ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  Total Tests:      40+                                                       ║
║  Passed:           40+                                                       ║
║  Failed:           0                                                         ║
║  Success Rate:     100.00%                                                   ║
║  Total Time:       ~10-15 seconds                                            ║
╚══════════════════════════════════════════════════════════════════════════════╝

I (15000) TLE92466ED_Test: ✅ ALL TESTS PASSED! System will restart in 10 seconds...
```

### GPIO14 Signal Pattern

```
Time    │ GPIO14 │ Event
────────┼────────┼──────────────────────────────
0ms     │   LOW  │ Boot
100ms   │ BLINK  │ Section start (5 blinks)
        │   ↑↓   │ 
200ms   │  HIGH  │ Test 1 complete
300ms   │   LOW  │ Test 2 complete
400ms   │  HIGH  │ Test 3 complete
500ms   │ BLINK  │ Section 2 start (5 blinks)
600ms   │   LOW  │ Test 4 complete
...
```

---

## Test Coverage

### Complete API Coverage

This test suite covers all TLE92466ED driver functionality:

#### Initialization & Device Information
- HAL initialization
- Driver initialization
- Chip ID reading
- IC version reading
- Device verification

#### Mode Management
- Mission Mode entry/exit
- Config Mode entry/exit
- Mode transition validation

#### Global Configuration
- CRC enable/disable
- VBAT threshold configuration (high-level and raw)
- Complete global configuration

#### Channel Control
- Single channel enable/disable
- All channels control
- Channel mask control
- Channel mode configuration (all modes)

#### Current Control
- Current setpoint setting (with read-back)
- Current ramping (smooth transitions)

#### PWM Configuration
- High-level API (period in microseconds)
- Low-level API (mantissa, exponent, low_freq_range)

#### Dither Configuration
- High-level API (amplitude in mA, frequency in Hz)
- Low-level API (step_size, num_steps, flat_steps)

#### Diagnostics & Monitoring
- Device status reading
- Channel diagnostics
- Voltage reading (VBAT, VIO)
- Current and duty cycle reading
- **Telemetry for all channels**
- **Device-level telemetry**
- **Telemetry with active channels**

#### Fault Management
- Fault clearing
- Software reset

#### Watchdog
- SPI watchdog reload

#### GPIO Control
- Reset pin control
- Enable pin control
- Fault pin reading

#### Multi-Channel Operations
- All 6 channels individually tested

#### Parallel Operation
- Parallel pair configuration

#### Error Handling
- Wrong mode operation validation
- Error condition testing

---

## Individual Test Details

### Initialization Tests

#### `test_hal_initialization()`

**Purpose**: Validate ESP32-C6 HAL creation and SPI initialization

**Operations**:
1. Create Esp32TleCommInterface instance with configuration from `TLE92466ED_TestConfig.hpp`
2. Initialize SPI peripheral (SPI2_HOST)
3. Configure GPIO pins for SPI
4. Set up SPI parameters (1MHz, Mode 1)

**Expected Result**: ✅ PASS
- HAL instance created successfully
- SPI bus initialized
- GPIO pins configured
- Ready for communication

**Failure Modes**:
- SPI bus already initialized
- GPIO pins in use
- Invalid SPI configuration
- Hardware fault

---

#### `test_driver_initialization()`

**Purpose**: Create TLE92466ED driver instance and initialize IC

**Operations**:
1. Create TLE92466ED driver instance with HAL reference
2. Execute driver initialization sequence
3. Configure TLE92466ED registers
4. Set up default parameters

**Expected Result**: ✅ PASS
- Driver instance created
- IC initialized successfully
- Registers configured
- Ready for operations

**Failure Modes**:
- HAL not initialized
- SPI communication failure
- TLE92466ED not powered (VIO required for SPI)
- IC initialization timeout

---

### Telemetry Tests

#### `test_all_channels_telemetry()`

**Purpose**: Test telemetry reading for all 6 channels

**Operations**:
1. Iterate through all channels (CH0-CH5)
2. For each channel, test:
   - `GetCurrentSetpoint()` - Read current setpoint
   - `GetAverageCurrent()` - Read average current
   - `GetDutyCycle()` - Read PWM duty cycle
   - `GetChannelDiagnostics()` - Read complete diagnostics
3. Report faults and warnings

**Expected Result**: ✅ PASS
- All telemetry functions work for all channels
- Values read successfully (may be 0 if channels not enabled)

**Telemetry Data Read**:
- Current setpoint (mA)
- Average current (mA)
- Duty cycle (raw 16-bit value)
- Diagnostics (faults, warnings, min/max current)

---

#### `test_device_telemetry()`

**Purpose**: Test device-level telemetry reading

**Operations**:
1. Read device status (`GetDeviceStatus()`)
2. Read VBAT voltage (`GetVbatVoltage()`)
3. Read VIO voltage (`GetVioVoltage()`)
4. Read fault pin status (`IsFault()`)

**Expected Result**: ✅ PASS
- All device telemetry functions work
- Status information displayed
- Voltage readings obtained

**Telemetry Data Read**:
- Device mode (Config/Mission)
- Initialization status
- Fault conditions
- Supply voltages (VBAT, VIO)
- Fault pin state

---

#### `test_telemetry_with_active_channel()`

**Purpose**: Test telemetry with an active channel (enabled with current set)

**Operations**:
1. Enter Mission Mode
2. Enable outputs (EN pin)
3. Set current setpoint to 500mA
4. Enable channel
5. Wait for current to stabilize (500ms)
6. Read all telemetry:
   - Current setpoint
   - Average current
   - Duty cycle
   - Complete diagnostics
7. Verify values are reasonable
8. Disable channel

**Expected Result**: ✅ PASS
- All telemetry functions work with active channel
- Current setpoint matches set value
- Average current is measured
- Duty cycle is non-zero
- Diagnostics show channel is active

**Telemetry Validation**:
- Setpoint matches configured value
- Average current is within reasonable range
- Duty cycle reflects current regulation
- No unexpected faults

---

### Mode Control Tests

#### `test_mode_transitions()`

**Purpose**: Validate mode transitions work correctly

**Operations**:
1. Perform 3 cycles of Config → Mission → Config transitions
2. Verify mode state after each transition
3. Ensure no errors occur

**Expected Result**: ✅ PASS
- All mode transitions succeed
- Mode state is correct after each transition
- No communication errors

---

### Error Condition Tests

#### `test_error_conditions()`

**Purpose**: Verify proper error handling

**Operations**:
1. Try to enable channel in Config Mode (should fail)
2. Try to configure channel mode in Mission Mode (should fail)
3. Try to configure global settings in Mission Mode (should fail)
4. Verify all operations correctly return errors

**Expected Result**: ✅ PASS
- All invalid operations correctly rejected
- Appropriate error codes returned
- No crashes or undefined behavior

---

## Troubleshooting

### Common Issues

#### 1. All Tests Fail

**Symptom**: Every test returns FAILED

**Root Causes**:
- TLE92466ED not powered (check VIO: 3.0-5.5V, typical 3.3V or 5.0V)
- SPI wiring incorrect
- Common ground missing
- ESP32-C6 not programmed correctly

**Solutions**:
```bash
# Check wiring
- VIO: 3.0-5.5V present (REQUIRED for SPI, typical 3.3V or 5.0V)
- GND: Common ground between ESP32-C6, TLE92466ED
- SPI pins: MISO, MOSI, SCLK, CS connected correctly
- RESN: Must be HIGH (not in reset)

# Re-flash
idf.py -p /dev/ttyUSB0 erase-flash
idf.py -p /dev/ttyUSB0 flash
```

---

#### 2. HAL Initialization Fails

**Symptom**: `test_hal_initialization` fails

**Root Causes**:
- SPI2 bus already in use
- GPIO pins conflicting
- Invalid SPI configuration

**Solutions**:
1. Check `sdkconfig` for SPI conflicts
2. Verify GPIO pins are not used elsewhere
3. Try different GPIO pins (modify `TLE92466ED_TestConfig.hpp`)
4. Check ESP32-C6 hardware

---

#### 3. Driver Initialization Fails

**Symptom**: `test_driver_initialization` fails

**Root Causes**:
- SPI communication not working
- TLE92466ED not responding
- Power supply issue (VIO required for SPI)

**Solutions**:
1. Verify SPI with logic analyzer/oscilloscope
2. Check VIO voltage (must be 3.0V to 5.5V, typical 3.3V or 5.0V)
3. Check VBAT voltage if testing with loads (5.5V to 41V, typical 12V or 24V)
4. Verify RESN pin is HIGH (not in reset)
5. Verify TLE92466ED is not damaged
6. Check VDD voltage (must be 5V) - powers central logic of the IC
7. Check VIO voltage (must be 3.0V to 5.5V, typical 3.3V or 5.0V) - sets SPI I/O levels

---

#### 4. Telemetry Tests Fail

**Symptom**: Telemetry reading functions fail

**Root Causes**:
- Channels not enabled (expected for some tests)
- Device not in correct mode
- Register read errors

**Solutions**:
1. Check if test expects channels to be enabled
2. Verify device is in correct mode (Config vs Mission)
3. Check SPI communication is working
4. Some telemetry failures are expected if channels are disabled

---

## Performance Characteristics

### Typical Execution Times

| Test Section | Expected Time | Max Time |
|--------------|--------------|----------|
| Initialization Tests | 50-100 ms | 200 ms |
| Mode Control Tests | 30-50 ms | 100 ms |
| Global Configuration Tests | 200-300 ms | 500 ms |
| Channel Control Tests | 500-800 ms | 1500 ms |
| Current Control Tests | 2000-3000 ms | 5000 ms |
| PWM Configuration Tests | 300-500 ms | 1000 ms |
| Dither Configuration Tests | 300-500 ms | 1000 ms |
| Diagnostics Tests | 1000-1500 ms | 3000 ms |
| Fault Management Tests | 100-200 ms | 400 ms |
| Watchdog Tests | 200-300 ms | 500 ms |
| GPIO Control Tests | 200-300 ms | 500 ms |
| Multi-Channel Tests | 2000-3000 ms | 5000 ms |
| Parallel Operation Tests | 300-500 ms | 1000 ms |
| Error Condition Tests | 100-200 ms | 400 ms |
| **Total Suite** | **~10-15 seconds** | **~30 seconds** |

### Memory Usage

| Resource | Usage | Notes |
|----------|-------|-------|
| **Flash** | ~350KB | Driver + HAL + test framework |
| **RAM (static)** | ~15KB | Driver instance + buffers |
| **Stack (per task)** | 8KB | Configurable in `RUN_TEST_IN_TASK` |
| **Heap (dynamic)** | ~5KB | HAL instance + std::unique_ptr |

### SPI Performance

- **Clock Frequency**: 1MHz (datasheet maximum: 8 MHz)
- **Transaction Time**: ~32μs per 32-bit frame
- **Register Read**: ~100μs (including overhead)
- **Register Write**: ~80μs (including overhead)

---

## Related Documentation

### ESP32 Examples
- [Solenoid Control Test](readme_solenoid_control_test.md) - Real hardware solenoid testing
- [Test Framework Reference](../main/TLE92466ED_TestFramework.hpp) - Framework API
- [Hardware Configuration](../main/TLE92466ED_TestConfig.hpp) - Pin and parameter config
- [ESP32-C6 CommInterface](../main/Esp32TleCommInterface.hpp) - CommInterface implementation details

### Driver Documentation
- [Driver API Reference](../../../docs/07_Driver_API.md) - Complete API documentation
- [CommInterface Implementation Guide](../../../docs/08_HAL_Implementation.md) - Porting to new platforms
- [Register Map](../../../docs/03_Register_Map.md) - TLE92466ED register details
- [Diagnostics](../../../docs/06_Diagnostics.md) - Fault detection and handling

### Build System
- [App Configuration](../app_config.yml) - Build metadata
- [CMake Setup](../CMakeLists.txt) - Build system configuration
- [Component Structure](../components/hf_tle92466ed/) - ESP-IDF component

---

## Notes

- This test suite is designed to validate driver functionality without requiring actual solenoids
- All tests are independent and can be enabled/disabled individually
- The suite validates both high-level and low-level APIs
- Error conditions are tested to ensure proper validation
- The suite covers all driver functionality
- Test execution time depends on enabled sections (typically 10-15 seconds for full suite)
- **VBAT is not required** for this test - only VDD and VIO are needed for SPI communication

---

**Example**: Driver Integration Test Suite  
**Platform**: ESP32-C6  
**Framework**: FreeRTOS + Test Framework  
**Test Sections**: 13  
**Total Tests**: 40+  
**Status**: ✅ Production Ready  
**Hardware Required**: TLE92466ED IC (no solenoids needed)

