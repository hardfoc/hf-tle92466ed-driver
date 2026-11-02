# SPI protocol

The TLE92466ED's 32-bit frames, the CRC, the one-frame reply pipeline, and what the driver does to make every
access trustworthy.

The frame definitions are in section 5.2.3 of the [datasheet](datasheet/tle92466ed-datasheet.pdf); the driver's
types are `tle92466ed::SPIFrame`, `ReplyMode`, `SPIStatus` and `CriticalFaultFlags` in
[`inc/tle92466ed_spi_interface.hpp`](../inc/tle92466ed_spi_interface.hpp).

## Frames

Every frame is 32 bits, MSB first, inside one CSN low period. A frame of any other length is ignored. The host
sends one of two request frames:

<div class="hf-registers" markdown>

| Bits | Write frame (R/W = 1) | Read frame (R/W = 0) |
| --- | --- | --- |
| `31:24` | `CRC` | `CRC` |
| `23:17` | `ADDRESS`, 7 bits | don't care (the driver sends 0) |
| `16` | `R/W` = 1 | `R/W` = 0 |
| `15:0` | `DATA` | `ADDRESS`, 16 bits |

</div>

The asymmetry matters: a write can address only `0x00` to `0x7F`, which holds every writable register (the central
configuration and the six channel banks). The status and feedback registers at `0x200` and above are read-only by
construction.

The chip answers with one of three reply frames, chosen by the register that was read:

<div class="hf-registers" markdown>

| Bits | 16-bit reply (`REPLY_16BIT`, 00B) | 22-bit reply (`REPLY_22BIT`, 01B) | Critical fault (`CRITICAL_FAULT`, 10B) |
| --- | --- | --- | --- |
| `31:24` | `CRC` | `CRC` | don't care |
| `23:22` | `00` | `01` | `10` |
| `21:17` | `STATUS` | `DATA[21:17]` | don't care |
| `16` | `R/W` of the previous request | `DATA[16]` | don't care |
| `15:8` | `DATA[15:8]` | `DATA[15:8]` | don't care |
| `7:0` | `DATA[7:0]` | `DATA[7:0]` | `1V5` `2V5` `BG` `CLK_TOO_SLOW` `CLK_TOO_FAST` `DIG_CLK_TOO_SLOW` `DIG_CLK_TOO_FAST` `WD_REF_CLK` |

</div>

Feedback registers (`FB_STAT`, `FB_VOLTAGE1/2` and the channel feedback bank at `+0x200`) answer with 22-bit
replies; every other register with 16-bit replies.

## Status field

The status in a 16-bit reply describes the **previous** request frame:

| `STATUS` | `SPIStatus` | Meaning |
| --- | --- | --- |
| `00000` | `NO_ERROR` | Accepted |
| `00001` | `SPI_FRAME_ERROR` | Malformed |
| `00010` | `CRC_ERROR` | CRC mismatch |
| `00011` | `WRITE_RO_REG` | Wrote a read-only register |
| `00100`-`00110` | `INTERNAL_BUS_FAULT` | Internal bus fault |

The lowest code has the highest priority. `Write()` turns any non-zero status into `CommError::TransferError`
(`DriverError::SPIFrameError`).

## CRC

Every frame carries a CRC-8 SAE J1850 byte in bits `[31:24]`:

| Parameter | Value |
| --- | --- |
| Polynomial | x<sup>8</sup> + x<sup>4</sup> + x<sup>3</sup> + x<sup>2</sup> + 1 (`0x1D`) |
| Initial value | `0xFF` |
| Final XOR | `0xFF` |
| Input order | frame `[7:0]`, then `[15:8]`, then `[23:16]` (the reverse of the wire order) |
| Check value | `0x4B` for the ASCII string `123456789` |

`CalculateCrc8J1850()` and `CalculateFrameCrc()` implement it; the host tests check the standard check value and
the byte order. The chip checks incoming CRCs only while `CRC_EN` in `GLOBAL_CONFIG` is set. The driver always
sends a correct CRC, clears `CRC_EN` in `Init()` and sets it again when you pass
`GlobalConfig::crc_enabled = true` to `ConfigureGlobal()`. Reply CRCs are checked on every read whatever
`CRC_EN` says.

## The reply pipeline

The reply to a frame comes out of SO during the **next** frame. A read therefore needs a second frame to collect
its answer, and that answer depends on what the bus did before. The driver makes each access independent of
history by sending three frames:

| Frame | MOSI | MISO carries |
| --- | --- | --- |
| 1, flush | read `0x0000` | The reply still pending from earlier traffic: discarded |
| 2, command | the read or write | The reply to frame 1: discarded |
| 3, dummy | read `0x0000` | The reply to frame 2: kept |

The dummies read address 0 rather than repeating the command: on the bench a repeated read made the part answer
with a reserved reply mode. `ReadMulti()` uses the same idea for a burst: flush, N commands, dummy, N + 2 frames
for N reads.

## How a reply is validated

A reply is accepted only when all of these hold:

1. MISO was driven (the word is not `0x00000000` or `0xFFFFFFFF`), otherwise `CommError::NoReply`.
2. The reply is not a critical-fault frame (reply mode `10B`). Its bits `[31:24]` are don't care in the datasheet,
   not a CRC, so the driver decides on the reply mode first and never CRC-checks a fault frame.
3. The CRC matches.
4. The reply mode is the width the register is defined to answer with (`ExpectedReplyFor()`), so a stale 16-bit
   reply cannot pass as a 22-bit feedback value.

`Read()` makes two attempts before it gives up with `CRCError` (frames arrived, none were well formed) or
`NoReply`. A critical-fault reply is not retried: it means the chip cannot serve any access, and the read fails
with `CommError::BusError` (`DriverError::FaultDetected`), whatever the frame's top byte holds. `Write()` and
`ReadMulti()` treat it the same way. `LastCriticalFaultFlags()` and `SawCriticalFault()` keep the fault byte for the
caller.

## Critical fault frames

The chip answers every access with a critical-fault frame while it is in the Critical Fault state: an internal
supply (1V5, 2V5, ADC bandgap) or the clock is out of range. Bits 7 to 5 are "OK" flags (1 = good); bits 4 to 0 are
fault flags (1 = fault).

| Bit | Flag | 1 means |
| --- | --- | --- |
| 7 | `1V5` | 1.5 V internal supply OK |
| 6 | `2V5` | 2.5 V internal supply OK |
| 5 | `BG` | ADC bandgap OK |
| 4 | `CLK_TOO_SLOW` | Clock too slow for operation |
| 3 | `CLK_TOO_FAST` | Clock too fast for operation |
| 2 | `DIG_CLK_TOO_SLOW` | Digital clock too slow (clock watchdog enabled) |
| 1 | `DIG_CLK_TOO_FAST` | Digital clock too fast (clock watchdog enabled) |
| 0 | `WD_REF_CLK` | Clock watchdog reference clock missing |

A fault byte of `0x00` (all three supplies "not OK") almost always means VDD is missing, not an SPI problem.

## Chip select and shared buses

The datasheet ends every access with a rising CSN edge, so each 32-bit frame needs its own CSN pulse, with at
least 600 ns of CSN high between frames. What must not happen between the frames of one access is another
device's transaction: the reply of the TLE92466ED's command would be clocked out during someone else's frame and
lost. On a shared bus, hold the bus (a mutex, `spi_device_acquire_bus()` on ESP-IDF) for the whole
`TransferMulti()` call. [Porting](porting.md#transfer-rules) has the rules for the port.

Next: [Current control](current-control.md).
