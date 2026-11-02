# Configuration

Every field of `GlobalConfig` and `ChannelConfig`, the defaults `Init()` writes, and the compiled examples that
apply them.

Both structures are in [`inc/tle92466ed.hpp`](../inc/tle92466ed.hpp). Apply them in Config Mode: the chip
accepts the global configuration and channel modes only there.

## Global configuration

```cpp
--8<-- "tests/docs/configuration.cpp:global"
```

| `GlobalConfig` field | Default | Register | Effect |
| --- | --- | --- | --- |
| `crc_enabled` | `true` | `GLOBAL_CONFIG.CRC_EN` | The chip checks the CRC byte of each frame; the driver checks write replies too |
| `spi_watchdog_enabled` | `true` | `GLOBAL_CONFIG.SPI_WD_EN` | Config Mode when `WD_TIME` runs out ([SPI watchdog](diagnostics.md#spi-watchdog)) |
| `clock_watchdog_enabled` | `true` | `GLOBAL_CONFIG.CLK_WD_EN` | Supervises f<sub>SYS</sub> (27 MHz to 29 MHz) |
| `vio_5v` | `false` | `GLOBAL_CONFIG.VIO_SEL` | VIO monitoring thresholds for 5 V (`true`) or 3.3 V (`false`) |
| `vbat_uv_voltage` | `4.0` V | `VBAT_TH` | VBAT under-voltage threshold |
| `vbat_ov_voltage` | `41.0` V | `VBAT_TH` | VBAT over-voltage threshold |
| `spi_watchdog_reload` | `1000` | `WD_RELOAD` | Initial `WD_TIME`, about 0.585 ms per count |

The structure's defaults are not what `Init()` writes. `Init()` keeps the bus permissive for bring-up:

| Setting | After `Init()` |
| --- | --- |
| CRC check (`CRC_EN`) | Off; the driver still sends correct CRCs |
| SPI watchdog | Off |
| Clock watchdog | On |
| VIO | 3.3 V thresholds |
| VBAT thresholds | 7 V under-voltage, 40 V over-voltage |
| Every channel | `ICC` mode, setpoint 0, slew rate 2.5 V/µs, `MIN_INT_THRESH` at the datasheet floor |

Call `ConfigureGlobal()` once the bus is proven to turn the CRC check and the watchdog on. `SetCrcEnabled()`,
`SetVbatThresholds()` and `SetVioLevel()` change one setting at a time.

## Channel configuration

```cpp
--8<-- "tests/docs/configuration.cpp:channel"
```

| `ChannelConfig` field | Default | Effect |
| --- | --- | --- |
| `mode` | `ICC` | Channel mode ([Current control](current-control.md#channel-modes)) |
| `current_setpoint_ma` | `0` | Setpoint in mA, 0 to 2000 (register scale; 1500 specified) |
| `slew_rate` | `MEDIUM_2V5_US` | Output slew rate |
| `diag_current` | `I_80UA` | Off-state diagnosis current source |
| `open_load_threshold` | `3` | Open load when the current is below `n`/8 of setpoint plus dither; 0 disables |
| `pwm_period_mantissa`, `pwm_period_exponent` | `0`, `0` | PWM target period; 0 leaves `PERIOD` unchanged |
| `auto_limit_disabled` | `false` | Disables the integrator auto-limit after setpoint changes |
| `olsg_warning_enabled` | `false` | Reports the OLSG warning |
| `min_int_thresh` | datasheet floor | `CTRL.MIN_INT_THRESH`; at 0 the power stage can stop switching |
| `pwm_period_calc_mode` | `false` | PWM controller ignores falling dither slopes |
| `deep_dither_enabled` | `false` | Deep dither |
| `dither_step_size`, `dither_steps`, `dither_flat` | `0` | Raw dither shape; 0 leaves dither unchanged |

| `SlewRate` | Typical | `DiagCurrent` | Typical |
| --- | --- | --- | --- |
| `SLOW_1V0_US` | 1 V/µs | `I_80UA` | 80 µA |
| `MEDIUM_2V5_US` | 2.5 V/µs | `I_190UA` | 190 µA |
| `FAST_5V0_US` | 5 V/µs | `I_720UA` | 720 µA |
| `FASTEST_10V0_US` | 10 V/µs | `I_1250UA` | 1250 µA |

Slew rates are typical at VBAT = 14 V into 10 Ω; the datasheet allows half to twice the value. A faster slew lowers
switching losses and raises emissions.

## Parallel pairs

```cpp
--8<-- "tests/docs/configuration.cpp:parallel"
```

[Current control](current-control.md#parallel-channels) lists the pairs and what happens to the slave channel.

## Clock source

`ConfigureClockSource(ClockSource::ExternalClockPll, f_clk_Hz)` switches the PLL to the CLK pin (1 MHz to 8 MHz)
and computes the dividers for f<sub>SYS</sub> = 28 MHz; `ClockSource::InternalOscillator` switches back. Config
Mode only. The evaluation PCB drives CLK from a 2 MHz oscillator.

Next: [Porting](porting.md).
