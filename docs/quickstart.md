# Quickstart

Regulate a solenoid current from your MCU: wire the SPI bus and three control pins, write a small port class,
call five driver functions.

!!! warning "Before you energise a coil"
    Use a current-limited VBAT supply for the first run and start with a low setpoint. Every inductive load
    needs a freewheeling diode from FWDx to its supply (see [Hardware](hardware.md#load-connection)).
    The example drives 500 mA into CH0.

## 1. Wire it

| MCU | TLE92466ED (pin) | Notes |
| --- | --- | --- |
| SPI SCLK, MOSI, MISO | SCK (6), SI (7), SO (9) | Mode 1, 1 MHz to start |
| SPI CS | CSN (5) | One pulse per 32-bit frame |
| GPIO out | RESN (26) | Active low reset |
| GPIO out | EN (27) | Active high enable |
| GPIO in, pull-up | FAULTN (11) | Open drain, active low |
| 3.3 V or 5 V | VIO (8) | The MCU's I/O rail |
| 5 V | VDD (29) | 4.5 V to 5.5 V |
| GND | GND, GNDPx | Common ground |

VBAT (pin 22) is the load supply, 6 V to 18 V. The [Hardware](hardware.md) page has every pin, the load
connection and the timing.

## 2. Add the driver

Add the repository as a submodule and link the `hf_tle92466ed` CMake target (or include `inc/` and build with
C++20). [Installation](installation.md) covers CMake and ESP-IDF.

```bash
git submodule add https://github.com/hardfoc/hf-tle92466ed-driver.git external/hf-tle92466ed-driver
```

## 3. Write the port

The port is the only platform code. It moves 32-bit frames over SPI, sets RESN and EN, reads FAULTN and waits.
Replace the `platform_*` calls with your SDK's.

```cpp title="my_spi.hpp"
--8<-- "tests/docs/my_spi.hpp:port"
```

[Porting](porting.md) explains each method, the frame rules and how to check a port.

## 4. Drive a channel

```cpp title="main.cpp"
--8<-- "tests/docs/quickstart.cpp:quickstart"
```

What each step does:

1. `Init()` pulses RESN, asserts EN, reads `ICVID` (expects `0xC1xx`), writes a default configuration and leaves
   the chip in Config Mode. Nothing else works until it succeeds.
2. `ConfigurePwmPeriod()` sets the channel's PWM target (110 Hz to 4 kHz). `ConfigureDither()` sets the dither and
   starts the channel's measurement clock, without which current feedback never updates.
3. `EnterMissionMode()` sets `OP_MODE`: channels can be switched on only in Mission Mode.
4. `SetCurrentSetpoint()` writes the 15-bit target for 500 mA; `EnableChannel()` sets `EN_CH0` and the current
   controller starts regulating.

This example is built by the host CI, so it compiles against the current headers.

## If it does not work

| Symptom | First thing to check |
| --- | --- |
| `Init()` returns `DeviceNotResponding` or `HardwareError` | MISO wiring, SPI mode 1, CSN per 32-bit frame, VIO present |
| `Init()` returns `FaultDetected` | Critical fault frames: VDD (5 V) missing or out of range, clock fault |
| `Init()` returns `WrongDeviceID` | Another device on the bus, or bits shifted by a wrong SPI mode |
| `EnableChannel()` returns `WrongMode` | Call `EnterMissionMode()` first |
| Current reads `FeedbackNotReady` | Dither clock not configured (`ConfigureDither()` or `ConfigureDitherClock()`) |

[Troubleshooting](troubleshooting.md) has the full list.

## Next

- [API guide](api-guide.md): configure channels, read feedback, handle faults.
- [Current control](current-control.md): setpoint scaling, PWM, dither and the feedback window.
- [Examples](examples.md): ESP32-S3 applications, from a no-hardware API test to a proportional-valve cycle.
