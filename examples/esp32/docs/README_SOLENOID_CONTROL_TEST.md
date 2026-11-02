# Solenoid Control Test - TLE92466ED ESP32-C6

## Table of Contents

1. [Overview](#overview)
2. [Purpose](#purpose)
3. [Hardware Requirements](#hardware-requirements)
4. [System Architecture](#system-architecture)
5. [Hardware Setup](#hardware-setup)
6. [Configuration](#configuration)
7. [Building and Running](#building-and-running)
8. [Operation](#operation)
9. [Real-Time Telemetry](#real-time-telemetry)
10. [Control Algorithm](#control-algorithm)
11. [Troubleshooting](#troubleshooting)
12. [Performance Characteristics](#performance-characteristics)
13. [Safety Considerations](#safety-considerations)
14. [Related Documentation](#related-documentation)

---

## Overview

The **Solenoid Control Test** is a real hardware test application that demonstrates actual solenoid control using the TLE92466ED driver. This test uses **ADC-based current control** where an analog voltage input (0-3.3V) controls the current setpoint for two solenoids, with real-time telemetry monitoring.

### Key Features

- **Real Hardware Testing**: Tests actual solenoids, not just driver logic
- **ADC-Based Control**: Analog voltage input (0-3.3V) maps to current percentage (0-100%)
- **Dual Solenoid Setup**: One single channel, one parallel pair
- **Independent Current Ranges**: Each solenoid has different min/max but same percentage mapping
- **Real-Time Telemetry**: Monitoring every 1 second
- **Continuous Operation**: Runs indefinitely until reset

### Test Configuration

```
┌─────────────────────────────────────────────────────────────┐
│  Solenoid Configuration                                     │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Solenoid 1 (Single Channel):                               │
│    └─ Channel: CH0                                          │
│    └─ Mode: Single Channel (ICC)                           │
│    └─ Current Range: 200-1500 mA                            │
│    └─ Percentage Mapping: 0-100% → 200-1500 mA             │
│                                                             │
│  Solenoid 2 (Parallel Pair):                                │
│    └─ Channels: CH1/CH2 (Parallel)                          │
│    └─ Mode: Parallel Pair (ICC)                             │
│    └─ Current Range: 400-3000 mA                             │
│    └─ Percentage Mapping: 0-100% → 400-3000 mA              │
│                                                             │
│  Control Input:                                              │
│    └─ ADC Pin: GPIO0 (ADC1_CH0)                             │
│    └─ Voltage Range: 0-3.3V                                  │
│    └─ Maps to: 0-100% current for both solenoids            │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Purpose

This test serves multiple purposes:

1. **Real Hardware Validation**: Tests actual solenoid operation with real loads
2. **Parallel Operation Verification**: Validates parallel channel pairing works correctly
3. **Current Control Accuracy**: Tests current regulation with real solenoids
4. **Telemetry Validation**: Demonstrates real-time monitoring capabilities
5. **System Integration**: Tests complete system from ADC input to solenoid output
6. **Performance Evaluation**: Measures actual current regulation accuracy

**Note**: This test requires actual solenoids and proper power supplies. For driver-only testing without hardware, use the **Driver Integration Test**.

---

## Hardware Requirements

### Required Components

- **ESP32-C6-DevKitC-1** development board
- **TLE92466ED** evaluation board or custom PCB
- **Two Solenoids** with appropriate current ratings:
  - Solenoid 1: 200-1500mA range
  - Solenoid 2: 400-3000mA range (parallel pair)
- **5.5-41V Power Supply** for VBAT (5A+ recommended for parallel operation, typical: 12V or 24V automotive)
- **3.0-5.5V Power Supply** for VIO (I/O level supply for SPI, typical: 3.3V or 5.0V)
- **5V Power Supply** for VDD (internal logic supply) - *Required for IC operation*
- **ADC Control Source**: Potentiometer, DAC, or voltage source (0-3.3V)
- **Current Sense Resistors** (if using external current monitoring)
- **Oscilloscope/Logic Analyzer** (optional, for signal analysis)
- Connecting wires and appropriate connectors

### Power Supply Requirements

```
┌─────────────────────────────────────────────────────────────┐
│  Power Supply Specifications                                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  VDD (Internal Logic Supply):                               │
│    └─ Voltage: 5V input to TLE92466ED                       │
│    └─ Purpose: Powers central logic of the IC               │
│    └─ Required: YES (for IC operation)                      │
│                                                             │
│  VIO (I/O Level Supply):                                    │
│    └─ Voltage: 3.0V to 5.5V (input)                          │
│    └─ Purpose: Sets I/O voltage levels for SPI              │
│    └─ Typical: 3.3V or 5.0V                                 │
│    └─ Current: 10-20mA                                       │
│    └─ Required: YES (for SPI communication)                 │
│                                                             │
│  VBAT (Load Supply):                                       │
│    └─ Voltage: 5.5V to 41V (recommended operating)          │
│    └─ Typical: 12V or 24V automotive                        │
│    └─ Current: 5A+ (for parallel operation)                  │
│    └─ Required: YES (for solenoid operation)                │
│    └─ Note: Must be capable of supplying peak currents       │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Solenoid Specifications

**Solenoid 1 (Single Channel)**:
- Recommended current range: 200-1500mA
- Typical resistance: 6-120Ω
- Power rating: 2.4-18W at 12V
- Must be rated for continuous operation

**Solenoid 2 (Parallel Pair)**:
- Recommended current range: 400-3000mA
- Typical resistance: 3-60Ω (combined)
- Power rating: 4.8-36W at 12V
- Must be rated for continuous operation
- Requires proper current sharing between CH1 and CH2

---

## System Architecture

### Block Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                        System Architecture                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐         ┌──────────────────┐                   │
│  │   Control    │         │   ESP32-C6        │                   │
│  │   Source     │         │                   │                   │
│  │              │         │  ┌──────────────┐ │                   │
│  │  Pot/DAC    │────────>│  │ ADC (GPIO0)  │ │                   │
│  │  0-3.3V     │         │  └──────┬───────┘ │                   │
│  └──────────────┘         │         │         │                   │
│                            │         ▼         │                   │
│                            │  ┌──────────────┐ │                   │
│                            │  │   Control    │ │                   │
│                            │  │   Logic      │ │                   │
│                            │  └──────┬───────┘ │                   │
│                            │         │         │                   │
│                            │         ▼         │                   │
│                            │  ┌──────────────┐ │                   │
│                            │  │   TLE92466ED │ │                   │
│                            │  │   Driver     │ │                   │
│                            │  └──────┬───────┘ │                   │
│                            └─────────┼──────────┘                   │
│                                      │                               │
│                            ┌─────────┴──────────┐                   │
│                            │                    │                    │
│                    ┌───────▼───────┐  ┌───────▼───────┐            │
│                    │  TLE92466ED   │  │  TLE92466ED   │            │
│                    │     IC        │  │     IC        │            │
│                    └───────┬───────┘  └───────┬───────┘            │
│                            │                   │                    │
│                    ┌───────▼───────┐  ┌───────▼───────┐            │
│                    │  Solenoid 1   │  │  Solenoid 2   │            │
│                    │  (CH0)        │  │  (CH1/CH2)    │            │
│                    │  200-1500mA   │  │  400-3000mA   │            │
│                    └───────────────┘  └───────────────┘            │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Control Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                      Control Flow Diagram                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. ADC Reading (100ms)                                        │
│     └─> Read GPIO0 voltage (0-3.3V)                            │
│     └─> Convert to percentage (0-100%)                           │
│                                                                 │
│  2. Current Calculation                                         │
│     └─> Solenoid 1: percentage → 200-1500mA                    │
│     └─> Solenoid 2: percentage → 400-3000mA                     │
│                                                                 │
│  3. Current Update                                              │
│     └─> SetCurrentSetpoint(Solenoid 1)                          │
│     └─> SetCurrentSetpoint(Solenoid 2)                           │
│                                                                 │
│  4. Telemetry Collection (1 second)                             │
│     └─> Read device status                                       │
│     └─> Read channel diagnostics                                 │
│     └─> Read voltages (VBAT, VIO)                                │
│     └─> Read currents (setpoint, actual)                          │
│     └─> Read duty cycles                                         │
│     └─> Check faults and warnings                                │
│                                                                 │
│  5. Display Telemetry                                           │
│     └─> Format and print status                                  │
│                                                                 │
│  Loop: Steps 1-5 repeat continuously                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Hardware Setup

### Complete Wiring Diagram

```
┌──────────────────────────────────────────────────────────────────────┐
│                         Complete Wiring                              │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ESP32-C6                    TLE92466ED                             │
│  ────────                    ──────────                             │
│                                                                      │
│  GPIO2  ───────────────────> MISO                                   │
│  GPIO7  ───────────────────> MOSI                                    │
│  GPIO6  ───────────────────> SCLK                                   │
│  GPIO18 ───────────────────> CS                                     │
│  GPIO21 ───────────────────> RESN                                    │
│  GPIO20 ───────────────────> EN                                     │
│  GPIO19 <─────────────────── FAULTN                                 │
│  GPIO0  <─────────────────── [ADC Input]                            │
│                                                                      │
│  5V     ───────────────────> VDD                                    │
│  3.3V   ───────────────────> VIO                                    │
│  GND    ───────────────────> GND                                     │
│                                                                      │
│  External Supply            TLE92466ED                             │
│  ───────────────            ──────────                             │
│                                                                      │
│  5.5-41V (12-24V typical) ──> VBAT                                    │
│  GND    ───────────────────> GND                                     │
│                                                                      │
│  Solenoid 1                TLE92466ED                             │
│  ──────────                ──────────                             │
│                                                                      │
│  Solenoid+ ────────────────> OUT0 (CH0)                             │
│  Solenoid- ────────────────> GND (via sense)                        │
│                                                                      │
│  Solenoid 2                TLE92466ED                             │
│  ──────────                ──────────                             │
│                                                                      │
│  Solenoid+ ────────────────> OUT1 (CH1) ──┐                        │
│                          ┌─────────────────┼──> OUT2 (CH2)         │
│                          │                 │                        │
│  Solenoid- ──────────────┴─────────────────┴──> GND (via sense)   │
│                                                                      │
│  Control Source          ESP32-C6                                  │
│  ──────────────          ────────                                    │
│                                                                      │
│  Potentiometer:                                                     │
│    3.3V ──[R1]─── GPIO0 ──[R2]─── GND                              │
│    (or DAC, voltage source, etc.)                                   │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

### Pin Assignment Table

| Function | ESP32-C6 GPIO | TLE92466ED Pin | Direction | Notes |
|----------|---------------|----------------|-----------|-------|
| **SPI MISO** | GPIO2 | MISO | Input | Data from TLE92466ED |
| **SPI MOSI** | GPIO7 | MOSI | Output | Data to TLE92466ED |
| **SPI SCLK** | GPIO6 | SCLK | Output | 1MHz SPI Clock |
| **SPI CS** | GPIO18 | CS | Output | Active Low |
| **RESN** | GPIO21 | RESN | Output | Reset pin (active low, must be HIGH) |
| **EN** | GPIO20 | EN | Output | Enable pin (active high) |
| **FAULTN** | GPIO19 | FAULTN | Input | Fault pin (active low) |
| **ADC Input** | GPIO0 | - | Input | Control voltage (0-3.3V) |
| **VDD Supply** | 5V | VDD | Power | 5V (Required) |
| **VIO Supply** | 3.3V/5V | VIO | Power | 3.0-5.5V (Required) |
| **Load Supply** | 12-24V typical | VBAT | Power | 5.5-41V, 5A+ recommended (Required) |
| **Ground** | GND | GND | Ground | Common reference |

### Solenoid Connections

**Solenoid 1 (Single Channel - CH0)**:
```
Solenoid Terminal+ ────> TLE92466ED OUT0
Solenoid Terminal- ────> GND (via current sense resistor if used)
```

**Solenoid 2 (Parallel Pair - CH1/CH2)**:
```
Solenoid Terminal+ ────┬───> TLE92466ED OUT1 (CH1)
                       └───> TLE92466ED OUT2 (CH2)
Solenoid Terminal- ────> GND (via current sense resistor if used)
```

**Important Notes**:
- Both channels in parallel pair must be connected to the same solenoid
- Ensure proper current sharing between CH1 and CH2
- Use appropriate wire gauge for high currents (parallel operation)
- Add current sense resistors if external current monitoring is desired

### ADC Control Input Setup

**Option 1: Potentiometer**
```
3.3V ──[10kΩ Pot]── GPIO0
              │
             GND
```

**Option 2: DAC Output**
```
DAC Output (0-3.3V) ──> GPIO0
```

**Option 3: Voltage Source**
```
Variable Voltage Source (0-3.3V) ──> GPIO0
```

**ADC Configuration**:
- Pin: GPIO0 (ADC1_CH0)
- Range: 0-3.3V
- Resolution: 12-bit (0-4095)
- Attenuation: 12dB (full range)
- Sampling: 1kHz

---

## Configuration

### Solenoid Configuration (`main/SolenoidControlTest.cpp`)

```cpp
/**
 * @brief Solenoid 1 Configuration (Single Channel)
 */
struct Solenoid1Config {
    static constexpr Channel CHANNEL = Channel::CH0;        // Single channel
    static constexpr uint16_t MIN_CURRENT_MA = 200;          // Minimum current (mA)
    static constexpr uint16_t MAX_CURRENT_MA = 1500;         // Maximum current (mA)
    static constexpr bool PARALLEL_MODE = false;             // Single channel mode
};

/**
 * @brief Solenoid 2 Configuration (Parallel Pair)
 */
struct Solenoid2Config {
    static constexpr ParallelPair PAIR = ParallelPair::CH1_CH2; // Parallel pair
    static constexpr Channel PRIMARY_CHANNEL = Channel::CH1;     // Primary channel
    static constexpr uint16_t MIN_CURRENT_MA = 400;              // Minimum current (mA)
    static constexpr uint16_t MAX_CURRENT_MA = 3000;             // Maximum current (mA)
    static constexpr bool PARALLEL_MODE = true;                  // Parallel mode
};
```

**To modify solenoid configuration**: Edit these structs in `main/SolenoidControlTest.cpp`

### ADC Configuration

```cpp
/**
 * @brief ADC Configuration
 */
struct ADCConfig {
    static constexpr adc_unit_t UNIT = ADC_UNIT_1;           // ADC unit (ADC1)
    static constexpr adc_channel_t CHANNEL = ADC_CHANNEL_0;  // ADC channel (GPIO0)
    static constexpr adc_atten_t ATTEN = ADC_ATTEN_DB_12;    // 0-3.3V range
    static constexpr adc_bitwidth_t BITWIDTH = ADC_BITWIDTH_12; // 12-bit resolution
    static constexpr float VREF_MV = 3300.0f;                 // Reference voltage (3.3V)
    static constexpr float ADC_MAX = 4095.0f;                 // Maximum ADC value
    static constexpr int GPIO_PIN = 0;                        // GPIO pin number
};
```

**To change ADC pin**: Modify `CHANNEL` and `GPIO_PIN` (ensure GPIO supports ADC)

### Control Loop Timing

```cpp
const TickType_t update_interval = pdMS_TO_TICKS(100);      // 100ms current update
const TickType_t telemetry_interval = pdMS_TO_TICKS(1000);   // 1 second telemetry
```

**To modify timing**: Edit these constants in `main/SolenoidControlTest.cpp`

---

## Building and Running

### Quick Start

```bash
cd examples/esp32

# Build the solenoid control test
./scripts/build_app.sh solenoid_control_test Debug

# Flash to ESP32-C6
./scripts/flash_app.sh solenoid_control_test Debug

# Monitor output
./scripts/monitor_app.sh solenoid_control_test
```

### Manual Build

```bash
cd examples/esp32

# Set target
idf.py set-target esp32c6

# Build with APP_TYPE=solenoid_control_test
idf.py -DAPP_TYPE=solenoid_control_test build

# Flash
idf.py -p /dev/ttyUSB0 flash

# Monitor
idf.py -p /dev/ttyUSB0 monitor
```

### Build Requirements

The solenoid control test requires the `esp_adc` component, which is automatically added by CMake when `APP_TYPE=solenoid_control_test`.

---

## Operation

### Startup Sequence

```
┌─────────────────────────────────────────────────────────────┐
│                    Startup Sequence                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  1. Initialize HAL                                          │
│     └─> Create Esp32TleCommInterface                       │
│     └─> Initialize SPI bus                                  │
│     └─> Configure GPIO pins                                  │
│                                                             │
│  2. Initialize Driver                                       │
│     └─> Create tle92466ed::Driver instance                  │
│     └─> Call Init() (resets device, verifies communication) │
│                                                             │
│  3. Initialize ADC                                          │
│     └─> Configure ADC1 channel 0 (GPIO0)                   │
│     └─> Set attenuation for 0-3.3V range                    │
│     └─> Enable calibration (if available)                    │
│                                                             │
│  4. Configure Solenoids                                     │
│     └─> Enter Config Mode                                   │
│     └─> Set Solenoid 1 to ICC mode (CH0)                    │
│     └─> Enable parallel operation for Solenoid 2 (CH1/CH2)  │
│     └─> Set Solenoid 2 to ICC mode (CH1)                    │
│     └─> Enter Mission Mode                                  │
│     └─> Enable outputs (EN pin)                             │
│                                                             │
│  5. Enable Solenoids                                        │
│     └─> Enable CH0 (Solenoid 1)                             │
│     └─> Enable CH1 (Solenoid 2, enables both in parallel)   │
│                                                             │
│  6. Start Control Loop                                      │
│     └─> Continuous operation begins                         │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Control Loop Operation

The control loop runs continuously with the following timing:

```
┌─────────────────────────────────────────────────────────────┐
│                    Control Loop Timing                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Every 100ms:                                               │
│    └─> Read ADC voltage (GPIO0)                             │
│    └─> Convert to percentage (0-100%)                       │
│    └─> Calculate currents for both solenoids                │
│    └─> Update current setpoints                             │
│                                                             │
│  Every 1 second:                                            │
│    └─> Collect telemetry                                    │
│    └─> Display formatted telemetry report                   │
│                                                             │
│  Continuous:                                                │
│    └─> Monitor for faults                                    │
│    └─> Update currents based on ADC                         │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Current Calculation

The ADC reading (0-3.3V) is mapped to a percentage (0-100%), which is then used to calculate the current for each solenoid:

```
┌─────────────────────────────────────────────────────────────┐
│              Current Calculation Formula                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Step 1: ADC to Percentage                                  │
│    percentage = (ADC_voltage / 3.3V) × 100%                 │
│                                                             │
│  Step 2: Percentage to Current                               │
│    Solenoid 1:                                               │
│      current = 200mA + (1500mA - 200mA) × (percentage/100)  │
│      Range: 200-1500mA                                      │
│                                                             │
│    Solenoid 2:                                               │
│      current = 400mA + (3000mA - 400mA) × (percentage/100)  │
│      Range: 400-3000mA                                     │
│                                                             │
│  Example:                                                    │
│    ADC = 1.65V (50%)                                        │
│    Solenoid 1: 200 + (1500-200)×0.5 = 850mA                 │
│    Solenoid 2: 400 + (3000-400)×0.5 = 1700mA                │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Real-Time Telemetry

### Telemetry Display Format

Every 1 second, the system displays telemetry:

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                        REAL-TIME TELEMETRY                                    ║
╠══════════════════════════════════════════════════════════════════════════════╣
  ADC Input: 1650.0 mV (50.0%)
  Target Currents: Solenoid 1=850 mA, Solenoid 2=1700 mA

  Device Status:
    Mode: Mission | Init: Done | Fault Pin: OK
    VBAT: 12000 mV | VIO: 3300 mV

  Solenoid 1 (CH0):
    Setpoint: 850 mA | Actual: 845 mA | Duty: 16384 (0x4000)
    Current Error: -5 mA (-0.6%)
    ✅ Status: Normal

  Solenoid 2 (CH1):
    Setpoint: 1700 mA | Actual: 1695 mA | Duty: 32768 (0x8000)
    Current Error: -5 mA (-0.3%)
    ✅ Status: Normal
╚══════════════════════════════════════════════════════════════════════════════╝
```

### Telemetry Data Explained

#### Device-Level Telemetry

- **Mode**: Current device mode (Config or Mission)
- **Init**: Initialization status (Done or Pending)
- **Fault Pin**: Hardware fault pin state (OK or FAULT)
- **VBAT**: VBAT supply voltage in millivolts
- **VIO**: VIO supply voltage in millivolts
- **Device Faults**: Any global fault conditions (UV/OV, temperature, clock, etc.)

#### Per-Solenoid Telemetry

- **Setpoint**: Target current in milliamperes
- **Actual**: Measured average current in milliamperes
- **Duty**: PWM duty cycle (raw 16-bit value and hex)
- **Current Error**: Difference between setpoint and actual (mA and percentage)
- **Faults**: Channel-specific fault conditions
- **Warnings**: Channel-specific warning conditions
- **Status**: Overall channel status (Normal or with faults/warnings)

### Telemetry Interpretation

#### Normal Operation
```
✅ Status: Normal
```
- No faults or warnings detected
- Current regulation working correctly
- All systems operational

#### Current Error Analysis
```
Current Error: -5 mA (-0.6%)
```
- **Negative error**: Actual current is less than setpoint
- **Positive error**: Actual current is greater than setpoint
- **Small errors (<5%)**: Normal for current regulation
- **Large errors (>10%)**: May indicate load mismatch or regulation issues

#### Fault Conditions
```
⚠️  Faults:
  - Over-current
  - Short to Ground
  - Open Load
```
- **Over-current**: Load drawing more than expected
- **Short to Ground**: Load or wiring shorted
- **Open Load**: No load connected or connection issue
- **Over-temperature**: IC or load overheating

#### Warning Conditions
```
⚠️  Warnings:
  - Current Regulation Warning
  - PWM Regulation Warning
```
- **Current Regulation Warning**: Current regulation may be at limits
- **PWM Regulation Warning**: PWM may be saturated
- **OT Warning**: Approaching temperature limits

---

## Control Algorithm

### Current Control Flow

```
┌─────────────────────────────────────────────────────────────┐
│              Current Control Algorithm                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  1. ADC Reading                                             │
│     Input: GPIO0 voltage (0-3.3V)                           │
│     Process: 12-bit ADC conversion                          │
│     Output: Raw ADC value (0-4095)                          │
│                                                             │
│  2. Voltage Calculation                                      │
│     Input: Raw ADC value                                    │
│     Process: Convert to voltage (with calibration)          │
│     Output: Voltage in millivolts (0-3300mV)                │
│                                                             │
│  3. Percentage Calculation                                   │
│     Input: Voltage (mV)                                      │
│     Process: percentage = (voltage / 3300) × 100              │
│     Output: Percentage (0.0-100.0%)                         │
│                                                             │
│  4. Current Calculation (Per Solenoid)                       │
│     Input: Percentage                                        │
│     Process:                                                 │
│       current = min + (max - min) × (percentage / 100)      │
│     Output: Current in milliamperes                         │
│                                                             │
│  5. Current Update                                           │
│     Input: Calculated current                                │
│     Process: SetCurrentSetpoint(channel, current)            │
│     Output: Current setpoint updated in TLE92466ED           │
│                                                             │
│  6. Current Regulation (TLE92466ED)                          │
│     Input: Current setpoint                                  │
│     Process: ICC (Integrated Current Control)                │
│     Output: Regulated current to solenoid                    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Parallel Operation

For Solenoid 2 (parallel pair), the current is set on the primary channel (CH1), and the TLE92466ED automatically shares the current between CH1 and CH2:

```
┌─────────────────────────────────────────────────────────────┐
│            Parallel Channel Current Sharing                 │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Setpoint: 1700mA (on CH1)                                  │
│                                                             │
│  TLE92466ED automatically:                                  │
│    └─> Shares current between CH1 and CH2                   │
│    └─> Each channel: ~850mA                                 │
│    └─> Total: ~1700mA to solenoid                          │
│                                                             │
│  Current Sharing:                                           │
│    └─> Automatic balancing by TLE92466ED                    │
│    └─> No manual current splitting required                  │
│    └─> Both channels must be enabled                        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Troubleshooting

### Common Issues

#### 1. Solenoids Not Responding

**Symptom**: No current flow, solenoids not activating

**Root Causes**:
- VBAT not connected or insufficient voltage
- EN pin not enabled
- Channels not enabled
- Device not in Mission Mode
- Solenoid wiring incorrect

**Solutions**:
```
Checklist:
□ VBAT: 5.5-41V connected (typical 12-24V) and sufficient current capacity
□ EN pin: HIGH (outputs enabled)
□ Channels: Enabled via EnableChannel()
□ Mode: Mission Mode (EnterMissionMode())
□ Wiring: Solenoid+ to OUT, Solenoid- to GND
□ RESN: HIGH (not in reset)
```

---

#### 2. Current Not Regulating Correctly

**Symptom**: Actual current doesn't match setpoint

**Root Causes**:
- Load resistance mismatch
- Power supply voltage too low
- Current limits exceeded
- Solenoid resistance out of range

**Solutions**:
```
1. Check load resistance:
   - Solenoid 1: R = VBAT / I_max = 12V / 1.5A = 8Ω minimum
   - Solenoid 2: R = VBAT / I_max = 12V / 3.0A = 4Ω minimum

2. Verify power supply:
   - VBAT voltage: 5.5V to 41V (typical 12-24V)
   - Current capacity: 5A+ for parallel operation

3. Check telemetry:
   - Current error should be <5% for good regulation
   - Duty cycle should be <90% for headroom
```

---

#### 3. ADC Reading Incorrect

**Symptom**: ADC voltage doesn't match control source

**Root Causes**:
- ADC pin not connected correctly
- Control source voltage out of range
- ADC calibration issue
- GPIO pin conflict

**Solutions**:
```
1. Verify connections:
   - Control source connected to GPIO0
   - Ground reference common

2. Check voltage range:
   - Must be 0-3.3V
   - Use voltage divider if needed

3. Verify ADC configuration:
   - GPIO0 supports ADC1_CH0
   - Attenuation set to 12dB (full range)
```

---

#### 4. Parallel Operation Not Working

**Symptom**: Solenoid 2 not working or current not shared

**Root Causes**:
- Parallel operation not enabled
- Only one channel enabled
- Wiring incorrect
- Current sharing imbalance

**Solutions**:
```
1. Verify configuration:
   - SetParallelOperation(CH1_CH2, true) called
   - Both CH1 and CH2 enabled (enabling CH1 enables both)
   - Device in Config Mode when configuring

2. Check wiring:
   - Solenoid+ connected to BOTH OUT1 and OUT2
   - Solenoid- connected to GND
   - Proper wire gauge for high current

3. Monitor telemetry:
   - Check diagnostics for both channels
   - Verify current is being shared
```

---

#### 5. Faults Detected

**Symptom**: Telemetry shows faults or warnings

**Over-current**:
- Load drawing too much current
- Reduce setpoint or check load resistance
- Verify power supply can deliver required current

**Short to Ground**:
- Load or wiring shorted
- Check continuity
- Inspect for solder bridges or damaged wiring

**Open Load**:
- No load connected (expected if testing without solenoid)
- Loose connection
- Load resistance too high

**Over-temperature**:
- IC or load overheating
- Add heatsink
- Reduce duty cycle
- Lower ambient temperature
- Check for adequate airflow

---

## Performance Characteristics

### Control Loop Performance

| Parameter | Value | Notes |
|----------|-------|-------|
| **Current Update Rate** | 10 Hz (100ms) | Fast enough for smooth control |
| **Telemetry Update Rate** | 1 Hz (1 second) | Status display |
| **ADC Sampling Rate** | 1 kHz | 12-bit resolution |
| **Current Regulation Response** | <100ms | TLE92466ED ICC response time |
| **Telemetry Latency** | <50ms | Time to read all telemetry |

### Current Regulation Accuracy

| Condition | Typical Error | Maximum Error |
|-----------|--------------|---------------|
| **Normal Operation** | ±2-5% | ±10% |
| **Low Current (<500mA)** | ±5-10% | ±15% |
| **High Current (>1000mA)** | ±2-5% | ±10% |
| **Parallel Operation** | ±3-7% | ±12% |

**Note**: Accuracy depends on load characteristics, power supply stability, and temperature.

### Power Consumption

| Component | Typical Current | Notes |
|-----------|----------------|-------|
| **ESP32-C6 (Idle)** | 20-30mA | Without WiFi/BLE |
| **ESP32-C6 (Active)** | 50-80mA | During operation |
| **TLE92466ED (VDD)** | - | 5V input (powers central logic) |
| **TLE92466ED (VIO)** | 10-20mA | I/O level supply (for SPI) |
| **Solenoid 1 (Max)** | 1500mA | At maximum setpoint |
| **Solenoid 2 (Max)** | 3000mA | At maximum setpoint (parallel) |

---

## Safety Considerations

### Electrical Safety

```
┌─────────────────────────────────────────────────────────────┐
│                    Safety Guidelines                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ⚠️  HIGH VOLTAGE WARNING                                   │
│    └─> VBAT can be up to 41V (recommended operating)        │
│    └─> Use appropriate safety measures                      │
│    └─> Ensure proper grounding                              │
│                                                             │
│  ⚠️  HIGH CURRENT WARNING                                   │
│    └─> Parallel operation can draw 3A+                       │
│    └─> Use appropriate wire gauge                           │
│    └─> Ensure connectors are rated for current             │
│                                                             │
│  ⚠️  THERMAL CONSIDERATIONS                                 │
│    └─> TLE92466ED can get hot during operation              │
│    └─> Add heatsink if needed                               │
│    └─> Monitor temperature via telemetry                   │
│                                                             │
│  ⚠️  SHORT CIRCUIT PROTECTION                               │
│    └─> TLE92466ED has built-in protection                   │
│    └─> Monitor FAULTN pin                                    │
│    └─> Check telemetry for fault conditions                 │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Operational Safety

1. **Start with Low Currents**: Begin testing at minimum current setpoints
2. **Monitor Telemetry**: Watch for faults and warnings
3. **Check Temperatures**: Ensure adequate cooling
4. **Verify Wiring**: Double-check all connections before powering on
5. **Use Fuses**: Add appropriate fuses in power supply lines
6. **Emergency Stop**: Have a way to quickly disable power (EN pin or power supply)

---

## Related Documentation

### ESP32 Examples
- [Driver Integration Test](readme_driver_integration_test.md) - Driver API validation (no hardware)
- [Test Framework Reference](../main/TLE92466ED_TestFramework.hpp) - Framework API
- [Hardware Configuration](../main/TLE92466ED_TestConfig.hpp) - Pin and parameter config
- [ESP32-C6 CommInterface](../main/Esp32TleCommInterface.hpp) - CommInterface implementation

### Driver Documentation
- [Driver API Reference](../../../docs/07_Driver_API.md) - Complete API documentation
- [CommInterface Implementation Guide](../../../docs/08_HAL_Implementation.md) - Porting to new platforms
- [Register Map](../../../docs/03_Register_Map.md) - TLE92466ED register details
- [Diagnostics](../../../docs/06_Diagnostics.md) - Fault detection and handling

### Hardware Documentation
- [TLE92466ED Datasheet](../../../Datasheet/) - Complete IC specifications
- [ESP32-C6 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf) - MCU documentation

---

## Notes

- This test requires actual solenoids and proper power supplies
- VBAT is **required** for solenoid operation (unlike driver integration test)
- Both solenoids use the same ADC percentage but different current ranges
- Parallel operation automatically shares current between channels
- Telemetry updates every 1 second
- Control loop updates every 100ms for responsive current control
- The test runs continuously until reset or power cycle

---

**Example**: Solenoid Control Test  
**Platform**: ESP32-C6  
**Framework**: FreeRTOS  
**Status**: ✅ Production Ready  
**Hardware Required**: TLE92466ED IC + Two Solenoids + Power Supplies

