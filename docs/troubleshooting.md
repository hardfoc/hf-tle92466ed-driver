# Troubleshooting

Symptoms you may see while bringing up a TLE92466ED, their causes and the fix for each, cheapest check first.

Start with the error a call returns and, once `Init()` has passed, with `GetAllFaults()`: the error tells bus
problems (no reply, bad replies) apart from chip state (wrong mode, a fault the chip reports).

## Bring-up

| Symptom | Cause | Fix |
| --- | --- | --- |
| `Init()` returns `DeviceNotResponding` | MISO idle: no VIO, CSN not reaching the chip, MISO not connected | Measure VIO at pin 8; scope CSN and SO during `Init()` |
| `Init()` returns `FaultDetected` | Every reply is a critical-fault frame | VDD (4.5 V to 5.5 V at pin 29) first, then the clock source |
| Reads fail with `CRCError` right after power-up | A critical-fault frame whose top byte is not a valid CRC ([SPI protocol](spi-protocol.md#how-a-reply-is-validated)) | Check VDD as for `FaultDetected` |
| `Init()` returns `WrongDeviceID` | `ICVID` is not `0xC1xx`: bits shifted by a wrong SPI mode, or another device on the same CSN | SPI mode 1; one CSN per device |
| `Init()` returns `ConfigurationError` | `OTP_VIRGIN`: the part's calibration OTP is not programmed | Use a production part |
| `CRCError` or `SPIFrameError` now and then | Frames not exactly 32 bits, CSN high time under 600 ns, clock too fast for the wiring | One 4-byte transfer per CSN; lower SCK to 1 MHz; shorten the wires |
| Wrong values on a shared bus | Another device's frame between the command and its reply | Hold the bus lock for the whole `TransferMulti()` ([Porting](porting.md#transfer-rules)) |

## Channels

| Symptom | Cause | Fix |
| --- | --- | --- |
| `EnableChannel()` returns `WrongMode` | Still in Config Mode | `EnterMissionMode()` first |
| `ConfigureChannel()` or `SetChannelMode()` returns `WrongMode` | In Mission Mode | `EnterConfigMode()`, configure, then go back |
| Channel enabled, no current | EN low (all channels off), setpoint 0, or the chip dropped to Config Mode | `Enable()`; `IsMissionMode()`; `GetAllFaults()` for supply or watchdog faults |
| Channel drops out after a while | SPI watchdog expired: the chip returns to Config Mode | Reload it faster ([Diagnostics](diagnostics.md#spi-watchdog)) or leave it off |
| Current far from the setpoint | No freewheeling diode at FWDx, setpoint above the specified 1.5 A, `parallel_mode` missing on a pair | [Hardware](hardware.md#load-connection), [Current control](current-control.md#setpoint) |
| `SetCurrentSetpoint()` returns `InvalidParameter` | Above 2000 mA (4000 mA in parallel) | Lower it; pass `parallel_mode = true` for a pair |
| A parallel pair drives half the current | The slave was configured instead of the master | Configure and drive CH0, CH1 or CH4 only |

## Feedback

| Symptom | Cause | Fix |
| --- | --- | --- |
| `GetAverageCurrent()` returns `FeedbackNotReady` | `DITHER_CLK_DIV` still 0, so the measurement period is 0 | `ConfigureDither()` or `ConfigureDitherClock()` on that channel |
| Feedback frozen at an old value | `FB_FRZ` left set by an interrupted snapshot | `ReleaseFeedbackFreeze()` |
| `SampleChannelCurrents()` sets `rejected_chains` | Replies arrived out of order on the bus | Bus integrity: shared-bus lock, wiring, SCK speed |
| `ReadChannelFeedback()` returns `TimeoutError` | No feedback update within the timeout | Longer timeout, or a faster dither (shorter measurement period) |

## Faults

| Symptom | Cause | Fix |
| --- | --- | --- |
| FAULTN low before `Init()` | RESN low, or latched power-on flags | `Init()` releases RESN and clears the power-on flags; then read `GetAllFaults()` |
| A fault returns right after `ClearFaults()` | The cause is still there | Read `GetAllFaults()`; the flag names the cause |
| `open_load` on a fitted coil | Open-load threshold too high for the current, or the load is wired to the wrong LOAD pin | Lower `open_load_threshold`; check the pin table |
| `overcurrent` on a channel | A short, or a load below the coil resistance it was sized for | Measure the load; the protection trips between 2.05 A and 6 A |
| `REG_ECC_ERR` comes back after every clear | Known on some parts; `Init()` reports it and continues | No action if the channels work |

## Debugging tips

- Route `Log()` in your port to a console: the driver logs every mode change, each failed attempt and the decoded
  critical-fault byte.
- `PrintAllFaults()` prints every fault flag; `IsFault(true)` does it when FAULTN is low.
- `ReadRegister(CentralReg::ICVID)` is the cheapest end-to-end read; `ReadAllSupplyVoltages()` the cheapest
  22-bit one.
