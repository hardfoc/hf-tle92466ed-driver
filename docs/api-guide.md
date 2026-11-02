# API guide

How to do the common jobs with the driver: bring the chip up, configure channels, drive currents, read feedback and
handle errors.

Every snippet on this page is compiled by the host CI from
[`tests/docs/api_tasks.cpp`](../tests/docs/api_tasks.cpp), with `Driver` standing for
`tle92466ed::Driver<MySpi>`. The [API reference](api-reference.md) lists every member.

## The shape of the API

One `Driver<Comm>` object per chip. Its members fall into these groups:

| Group | Members | Mode |
| --- | --- | --- |
| Bring-up and modes | `Init()`, `EnterMissionMode()`, `EnterMissionModeChecked()`, `EnterConfigMode()`, `IsMissionMode()` | |
| Global configuration | `ConfigureGlobal()`, `SetCrcEnabled()`, `SetVbatThresholds()`, `SetVioLevel()`, `ConfigureClockSource()` | Config |
| Channel setup | `SetChannelMode()`, `ConfigureChannel()`, `SetParallelOperation()` | Config |
| Current, PWM, dither | `SetCurrentSetpoint()`, `SetCurrentSetpointUa()`, `ConfigurePwmPeriod()`, `ConfigureDither()`, `ConfigureDitherClock()` | Any |
| Switching | `EnableChannel()`, `EnableChannels()`, `EnableAllChannels()`, `DisableAllChannels()` | Mission |
| Feedback | `GetAverageCurrent()`, `GetDutyCycle()`, `ReadChannelFeedback()`, `SampleChannelCurrents()`, `ReadAllSupplyVoltages()` | Any |
| Faults | `GetAllFaults()`, `GetDeviceStatus()`, `GetChannelDiagnostics()`, `ClearFaults()`, `IsFault()`, `SetFaultMask()` | Any |
| Watchdog and identity | `ReloadSpiWatchdog()`, `VerifyDevice()`, `GetIcVersion()`, `GetChipId()` | Any |
| Control pins | `HoldReset()`, `ReleaseReset()`, `Enable()`, `Disable()` | Any |
| Registers | `ReadRegister()`, `ReadRegisterMulti()`, `WriteRegister()`, `ModifyRegister()` | Any |

Channels are `tle92466ed::Channel::CH0` to `CH5`. Every call returns a `DriverResult<T>` and never throws: test it
with `if (!r)` and read `r.error()`, a `DriverError`.

## Initialise

```cpp
--8<-- "tests/docs/api_tasks.cpp:init"
```

`Init()` initialises your port, pulses RESN (10 ms low, 50 ms settle), sets EN, reads `ICVID` up to five times,
refuses unprogrammed silicon, writes defaults (every channel `ICC`, setpoint 0, clock watchdog on, CRC and SPI
watchdog off) and clears the power-on flags. `Init(false)` skips the RESN pulse, for a retry on a shared bus.

## Configure

Configure in Config Mode, before `EnterMissionMode()`: the chip accepts channel modes, the global configuration and
parallel pairs only there.

```cpp
--8<-- "tests/docs/api_tasks.cpp:configure"
```

`ConfigureChannel()` sets everything about a channel in one call (mode, setpoint, slew rate, diagnosis current,
open-load threshold, PWM, dither); [Configuration](configuration.md) lists the fields of `ChannelConfig` and
`GlobalConfig`. [Current control](current-control.md) gives the ranges and formulas behind the PWM and dither
calls.

## Drive a current

```cpp
--8<-- "tests/docs/api_tasks.cpp:drive"
```

`SetCurrentSetpoint()` takes milliamperes (`SetCurrentSetpointUa()` microamperes) and can be called in either mode;
the current flows once the channel is enabled in Mission Mode with EN high. Pass `parallel_mode = true` for the
master channel of a parallel pair.

## Read feedback

```cpp
--8<-- "tests/docs/api_tasks.cpp:read"
```

`GetAverageCurrent()` waits for a completed measurement window (up to about 8 ms) and is meant for consoles and
diagnostics. Supplies come back in millivolts. In a control loop, sample instead:

```cpp
--8<-- "tests/docs/api_tasks.cpp:sample"
```

`SampleChannelCurrents()` reads every requested channel in one pipelined burst and never waits: a channel whose
window is not ready keeps its bit clear in `valid_mask` and is picked up on the next tick.

## Use any register directly

```cpp
--8<-- "tests/docs/api_tasks.cpp:raw"
```

Central registers are in `tle92466ed::CentralReg`; channel registers are an offset in `ChannelReg` added to the
channel's base (`GetChannelRegister()`). `WriteRegister()` reads the value back by default; `ModifyRegister()`
reads, masks and writes. Writes reach only `0x00` to `0x7F` ([SPI protocol](spi-protocol.md#frames)).

## Errors

| `DriverError` | Meaning | First thing to check |
| --- | --- | --- |
| `NotInitialized` | `Init()` has not succeeded | Call order |
| `WrongMode` | Needs Config Mode (configuration) or Mission Mode (enable) | `EnterConfigMode()` / `EnterMissionMode()` |
| `InvalidChannel`, `InvalidParameter` | Argument out of range | Channel, setpoint scale, PWM period |
| `DeviceNotResponding` | MISO idle on every attempt | Power, VIO, CSN, MISO wiring |
| `CRCError` | Replies arrived but none passed CRC and width checks | SPI mode, clock speed, CSN timing, shared bus |
| `SPIFrameError` | The chip flagged the previous frame (status field) | Frame length (exactly 32 bits), CRC |
| `FaultDetected` | Critical-fault reply | VDD, internal supplies, clock ([SPI protocol](spi-protocol.md#critical-fault-frames)) |
| `WrongDeviceID` | `ICVID` is not `0xC1xx` | Another device on CSN, wrong SPI mode |
| `RegisterError` | A write did not read back | Bus integrity, Mission Mode restrictions |
| `FeedbackNotReady` | No measurement window yet | Dither clock (`ConfigureDither()`), feedback frozen |
| `ConfigurationError` | Unprogrammed OTP, or a configuration the chip rejected | The part, the arguments |
| `TimeoutError` | A wait (feedback, mission mode, BIST) ran out | Timeout argument, chip state |
| `HardwareError` | Your port returned an error | The port |

Faults the chip detects are not errors of the call; read them with `GetAllFaults()`:

```cpp
--8<-- "tests/docs/api_tasks.cpp:faults"
```

[Diagnostics and faults](diagnostics.md) explains every fault source; [Troubleshooting](troubleshooting.md) maps
symptoms to causes.

## Keep the SPI watchdog fed

```cpp
--8<-- "tests/docs/api_tasks.cpp:watchdog"
```

Only needed when you enable the watchdog in `ConfigureGlobal()`; the
[timeout table](diagnostics.md#spi-watchdog) converts `WD_TIME` to milliseconds.

## Threads

The driver is not thread-safe. Give each chip one owner task, or guard every call on a driver object (and its
port) with one mutex.
