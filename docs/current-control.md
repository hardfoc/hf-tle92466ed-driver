# Current control

How the TLE92466ED regulates a coil current, how the driver scales setpoints, PWM and dither, and when current
feedback is valid.

Section references are to the [datasheet](datasheet/tle92466ed-datasheet.pdf) (Rev. 1.2). f<sub>SYS</sub> is
28 MHz.

## Channel modes

A channel's mode is set in Config Mode (`SetChannelMode()` or `ChannelConfig::mode`) and decides how the output
switches and which period the feedback averages over.

| `ChannelMode` | Output | Feedback period T<sub>meas</sub> |
| --- | --- | --- |
| `OFF` | Off | No measurement |
| `ICC` | Integrated current control to the `SETPOINT` target | Dither period T<sub>Dither</sub> |
| `DIRECT_DRIVE_SPI` | On-time from the `TON` register | T<sub>Dither</sub> |
| `DIRECT_DRIVE_DRV0`, `DIRECT_DRIVE_DRV1` | Follows the DRV0 / DRV1 pin | T<sub>Dither</sub> if configured, else the DRV period |
| `FREE_RUN_MEAS` | Switch off; the shunt measures only | T<sub>Dither</sub> |

`Init()` sets every channel to `ICC` with a zero setpoint.

## Setpoint

The 15-bit `TARGET` field of each channel's `SETPOINT` register sets the average current:

| Mode | Formula | Full scale | One LSB | Specified range |
| --- | --- | --- | --- | --- |
| Single channel | I<sub>set</sub> = 2 A × `TARGET` / (2<sup>15</sup> − 1) | 2 A | 61 µA | 0 mA to 1500 mA |
| Parallel pair | I<sub>set</sub> = 4 A × `TARGET` / (2<sup>15</sup> − 1) | 4 A | 122 µA | up to 2.7 A |

`SetCurrentSetpoint(channel, mA, parallel)` and `SetCurrentSetpointUa(channel, µA, parallel)` round to the
nearest `TARGET`. They accept the full register scale (2000 mA, 4000 mA in parallel) and refuse anything above it
with `InvalidParameter`; keep the steady-state setpoint inside the specified range. After the write the driver
reads `SETPOINT` back and retries up to three times. `GetCurrentSetpoint()` converts the register back to mA.

The controller is an integrator: it switches the output on and off so that the average current over a PWM period
matches the target. Accuracy is ±2.5 mA from 10 mA to 250 mA and ±1 % above (single channel, section 4.6.3).

## PWM period

The PWM frequency controller adjusts the switching period to a target set in the `PERIOD` register
(section 4.6.2.3):

| Range | Formula | Period | Frequency |
| --- | --- | --- | --- |
| Standard (`LOW_FREQ_RANGE_EN` = 0) | T = `PERIOD_MANT` × 2<sup>`PERIOD_EXP`</sup> / f<sub>SYS</sub> | 250 µs to 9.09 ms | 4 kHz to 110 Hz |
| Low frequency (`LOW_FREQ_RANGE_EN` = 1) | T = `PERIOD_MANT` × 8 × 2<sup>`PERIOD_EXP`</sup> / f<sub>SYS</sub> | 2 ms to 72.7 ms | 500 Hz to 13.75 Hz |

The datasheet specifies the controller for 110 Hz to 4 kHz; the low-frequency limits are that range divided by
eight. `ConfigurePwmPeriod(channel, period_us)` picks the range, mantissa and exponent, and returns
`InvalidParameter` outside 250 µs to 72.7 ms. `ConfigurePwmPeriodRaw()` writes the fields directly.

## Dither

Dither overlays a triangular or trapezoidal wave on the setpoint to reduce a valve's hysteresis
(section 4.7):

| Quantity | Formula |
| --- | --- |
| Amplitude | I<sub>Dither</sub> = `STEPS` × `STEP_SIZE` × 2 A / (2<sup>15</sup> − 1) |
| Period | T<sub>Dither</sub> = (4 × `STEPS` + 2 × `FLAT`) × t<sub>ref_clk</sub> |
| Reference clock | t<sub>ref_clk</sub> = `MANT` × 2<sup>`EXP`</sup> / f<sub>SYS</sub> (`DITHER_CLK_DIV`) |

`ConfigureDither(channel, amplitude_mA, frequency_Hz)` uses 16 steps and 2 flat steps, derives t<sub>ref_clk</sub>
from the frequency and `STEP_SIZE` from the amplitude, and enables setpoint synchronisation so a new averaging
window starts with each setpoint write. Keep I<sub>set</sub> − I<sub>Dither</sub> above zero and
I<sub>set</sub> + I<sub>Dither</sub> inside the measurement range. Dither stops while the setpoint is 0.

!!! warning "No dither clock, no feedback"
    `DITHER_CLK_DIV` resets to 0. With t<sub>ref_clk</sub> = 0 the measurement period is 0 and the averaged
    feedback (`FB_DC`, `FB_I_AVG`, `FB_VBAT`) never updates. Call `ConfigureDither()` (any amplitude, including 0)
    or `ConfigureDitherClock()` for every channel whose current you read.

## Current feedback

Each channel measures its current over T<sub>meas</sub> and publishes mantissa / exponent results
(section 4.10.2). The average current is

I<sub>avg</sub> = 4 A × `I_AVG_MANT` / `TP_MANT`

with `I_AVG_MANT` signed (from `FB_I_AVG`) and `TP_MANT` the measurement window (from `FB_DC`). A small
`TP_MANT` means no window has completed, so the driver refuses to divide by it:

| Call | Use it for | Blocks | When there is no window |
| --- | --- | --- | --- |
| `GetAverageCurrent(ch)` | Console, diagnostics | Up to about 8 ms (freezes, polls `FB_UPD`) | `FeedbackNotReady` |
| `GetDutyCycle(ch)` | Duty in permyriad (0 to 10000) | No | Returns 0 |
| `ReadChannelFeedback(ch)` | Average, min / max current and period, VBAT, in one freeze | Up to the timeout (10 ms default) | `TimeoutError` |
| `SampleChannelCurrents(mask)` | Control loops: several channels, one burst | Never | The channel's bit is clear in `valid_mask` |

`SampleChannelCurrents()` returns a `CurrentFeedbackBatch`: per-channel `iavg_ma`, the measurement window, and
`rejected_chains`, which counts read chains discarded because replies arrived out of order.

## Parallel channels

Channels CH0/CH3, CH1/CH2 and CH4/CH5 can drive one load together (section 4.5.2):

| `ParallelPair` | Master | Slave |
| --- | --- | --- |
| `CH0_CH3` | CH0 | CH3 |
| `CH1_CH2` | CH1 | CH2 |
| `CH4_CH5` | CH4 | CH5 |

Enable the pair in Config Mode with `SetParallelOperation()`, then configure and drive only the master, passing
`parallel_mode = true` to the setpoint and dither calls. The slave's `MODE` and `SETPOINT` writes are ignored and
read back zero. In parallel mode one `TARGET` LSB is worth twice the current.

```cpp
--8<-- "tests/docs/configuration.cpp:parallel"
```

Next: [Diagnostics and faults](diagnostics.md).
