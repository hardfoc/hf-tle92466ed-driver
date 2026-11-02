# Configuration

This guide covers all configuration options available for the TLE92466ED driver.

## Global Configuration

### Basic Global Setup

```cpp
tle92466ed::GlobalConfig global_config;
global_config.crc_enabled = true;
global_config.spi_watchdog_enabled = true;
global_config.clock_watchdog_enabled = true;
global_config.vio_5v = false;  // false = 3.3V, true = 5.0V
global_config.vbat_uv_voltage = 4.0f;   // UV threshold in volts
global_config.vbat_ov_voltage = 41.0f;  // OV threshold in volts
global_config.spi_watchdog_reload = 1000;

driver.ConfigureGlobal(global_config);
```text

### CRC Configuration

```cpp
driver.SetCrcEnabled(true);  // Enable CRC checking (recommended)
```cpp

- **Enabled**: All SPI frames include CRC-8 (SAE J1850) for error detection
- **Disabled**: No CRC checking (faster, but less reliable)

### VBAT Thresholds

```cpp
// High-level API (recommended)
driver.SetVbatThresholds(4.0f, 41.0f);  // UV=4V, OV=41V

// Low-level API (direct register control)
driver.SetVbatThresholdsRaw(25, 253);  // Register values
```cpp

**Threshold Range**: 0V to ~41.4V (calculated as `register_value * 0.16208V`)

## Channel Configuration

### Basic Channel Setup

```cpp
tle92466ed::ChannelConfig config;
config.mode = tle92466ed::ChannelMode::ICC;
config.current_setpoint_ma = 1500;  // 1.5A
config.slew_rate = tle92466ed::SlewRate::MEDIUM_2V5_US;
config.diag_current = tle92466ed::DiagCurrent::I_80UA;
config.open_load_threshold = 3;  // 3/8 threshold

driver.ConfigureChannel(tle92466ed::Channel::CH0, config);
```cpp

### Channel Modes

#### ICC Mode (Integrated Current Control)

```cpp
config.mode = tle92466ed::ChannelMode::ICC;
```cpp

- **Purpose**: Precise current regulation with 15-bit resolution
- **Use Case**: Solenoids, valves requiring constant current
- **Features**: Automatic current regulation, PWM-based control

#### Direct Drive Modes

```cpp
// SPI-controlled on-time
config.mode = tle92466ed::ChannelMode::DIRECT_DRIVE_SPI;

// External pin control
config.mode = tle92466ed::ChannelMode::DIRECT_DRIVE_DRV0;
config.mode = tle92466ed::ChannelMode::DIRECT_DRIVE_DRV1;
```cpp

- **Purpose**: Direct control via SPI or external pins
- **Use Case**: Simple on/off control, external timing

#### Free-Running Measurement

```cpp
config.mode = tle92466ed::ChannelMode::FREE_RUN_MEAS;
```cpp

- **Purpose**: Continuous current sensing without output control
- **Use Case**: Diagnostics, current monitoring

### Current Settings

```cpp
// Single channel mode (0-2000 mA)
driver.SetCurrentSetpoint(tle92466ed::Channel::CH0, 1500);  // 1.5A

// Parallel mode (0-4000 mA)
driver.SetCurrentSetpoint(tle92466ed::Channel::CH0, 3000, true);  // 3A
```cpp

**Current Range**:

- **Single channel**: 0-2000 mA (1.5A typical continuous, 2.0A max)
- **Parallel mode**: 0-4000 mA (2.7A typical continuous, 4.0A max)
- **Resolution**: 15-bit (0.061mA per LSB single, 0.122mA parallel)

### PWM Configuration

```cpp
// High-level API (recommended)
driver.ConfigurePwmPeriod(tle92466ed::Channel::CH0, 100.0f);  // 100 µs period

// Low-level API (direct control)
driver.ConfigurePwmPeriodRaw(tle92466ed::Channel::CH0, 
                            100,    // mantissa
                            3,      // exponent
                            false); // low_freq_range
```cpp

**PWM Period Range**: ~0.125 µs to ~32.64 ms

### Dither Configuration

```cpp
// High-level API (recommended)
driver.ConfigureDither(tle92466ed::Channel::CH0, 
                       50.0f,   // 50 mA amplitude
                       1000.0f, // 1000 Hz frequency
                       false);  // not parallel mode

// Low-level API (direct control)
driver.ConfigureDitherRaw(tle92466ed::Channel::CH0,
                          100,  // step_size
                          10,   // num_steps
                          5);   // flat_steps
```text

**Dither Amplitude**: 0-1800 mA (configurable)

### Slew Rate

```cpp
config.slew_rate = tle92466ed::SlewRate::SLOW_1V0_US;      // 1.0 V/µs (low EMI)
config.slew_rate = tle92466ed::SlewRate::MEDIUM_2V5_US;    // 2.5 V/µs (standard)
config.slew_rate = tle92466ed::SlewRate::FAST_5V0_US;      // 5.0 V/µs (fast)
config.slew_rate = tle92466ed::SlewRate::FASTEST_10V0_US;  // 10.0 V/µs (maximum)
```cpp

## Parallel Operation

### Enabling Parallel Mode

```cpp
// Pair channels 0 and 3
driver.SetParallelOperation(tle92466ed::ParallelPair::CH0_CH3, true);

// Pair channels 1 and 2
driver.SetParallelOperation(tle92466ed::ParallelPair::CH1_CH2, true);

// Pair channels 4 and 5
driver.SetParallelOperation(tle92466ed::ParallelPair::CH4_CH5, true);
```cpp

**Parallel Pairs**:

- CH0/CH3: Doubles current capability (up to 4A)
- CH1/CH2: Doubles current capability (up to 4A)
- CH4/CH5: Doubles current capability (up to 4A)

**Note**: When in parallel mode, set current with `parallel_mode = true` parameter.

## Default Values

### Global Defaults

- CRC: Enabled
- SPI Watchdog: Enabled
- Clock Watchdog: Enabled
- VIO: 3.3V mode
- VBAT UV: ~4V
- VBAT OV: ~41V

### Channel Defaults

- Mode: ICC
- Current: 0 mA
- Slew Rate: MEDIUM_2V5_US
- Diagnostic Current: I_80UA
- Open Load Threshold: 3/8

## Next Steps

- Review [Examples](examples.md) for configuration examples
- Check [API Reference](api_reference.md) for all configuration methods
- See [Troubleshooting](troubleshooting.md) for configuration issues

---

**Navigation**
⬅️ [Platform Integration](platform_integration.md) | [Next: Examples ➡️](examples.md) | [Back to Index](index.md)
