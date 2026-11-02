---
hide:
  - navigation
  - toc
---

<div class="hf-hero" markdown>
<span class="hf-kicker">HardFOC driver</span>

# TLE92466ED Driver

<p>A C++20 driver for the Infineon TLE92466ED six-channel low-side solenoid driver. It brings the chip up,
configures current control, PWM and dither per channel, regulates solenoid and valve currents and reads back
feedback and diagnostics, on any MCU with an SPI bus.</p>

[Quickstart](quickstart.md){ .md-button .md-button--primary }
[API guide](api-guide.md){ .md-button }
[Source on GitHub](https://github.com/hardfoc/hf-tle92466ed-driver){ .md-button }
</div>

The driver has no platform code. You give it a small class that moves 32-bit SPI frames and sets three pins; it
does the rest: frame encoding with the CRC byte, the one-frame reply pipeline, reply validation, current scaling
and fault decoding.

<div class="hf-chip" markdown>
<div class="hf-chip__head">
<span class="hf-chip__kicker">Six-channel low-side solenoid driver</span>
<span class="hf-chip__name">TLE92466ED</span>
<span class="hf-chip__vendor">Infineon</span>
</div>

Outputs
:   Six low-side channels, integrated MOSFETs (R<sub>DS(on)</sub> 115 mΩ) and shunts (140 mΩ)

Current control
:   15-bit setpoint, 0 to 1.5 A per channel (1.8 A with dither), 2.7 A in a parallel pair

Host interface
:   SPI mode 1 up to 8 MHz, 32-bit frames with CRC-8 SAE J1850, SPI watchdog

Supplies
:   VDD 4.5 V to 5.5 V, VBAT 6 V to 18 V (4 V to 38 V extended), VIO 3.0 V to 5.5 V

Control pins
:   RESN, EN, DRV0, DRV1, CLK (inputs), FAULTN (open-drain output)

Package
:   PG-DSO-36, T<sub>J</sub> −40 °C to 150 °C (175 °C extended), AEC-Q100 Grade 0

[Datasheet](datasheet/tle92466ed-datasheet.pdf) ·
[Evaluation kit user manual](datasheet/tle92466ed-eval-kit-user-manual.pdf)
</div>

## What you get

- **One object per chip.** `tle92466ed::Driver<YourPort>` covers the whole chip: modes, channel setup, current
  setpoints, PWM, dither, feedback, diagnostics and raw registers. See the [API guide](api-guide.md).
- **Current in milliamperes.** `SetCurrentSetpoint(Channel::CH0, 500)` writes the 15-bit `TARGET` with the
  datasheet's scaling, single or parallel. See [Current control](current-control.md).
- **A careful SPI transport.** Every access is self-synchronising (flush, command, dummy), every reply is checked
  for CRC and width, and critical-fault frames are decoded. See [SPI protocol](spi-protocol.md).
- **Diagnostics you can act on.** Supplies, temperature, open load, short to ground and over-current per channel,
  in one `FaultReport`. See [Diagnostics and faults](diagnostics.md).
- **Zero-overhead portability.** The port is a CRTP class: no virtual calls, no heap, no exceptions, no RTOS
  dependency. See [Porting](porting.md).
- **Tested without hardware.** Host unit tests run the driver against a register model of the chip's SPI. See
  [Testing and CI](testing.md).

## Where to go next

<div class="grid cards" markdown>

-   :material-rocket-launch-outline: **[Quickstart](quickstart.md)**

    ---

    Wire the chip, write the port, regulate a coil current in about 15 lines.

-   :material-chip: **[Hardware](hardware.md)**

    ---

    Pins, supplies, bus settings and timing, the evaluation kit.

-   :material-layers-outline: **[Architecture](architecture.md)**

    ---

    The three layers and what each one owns.

-   :material-code-braces: **[API guide](api-guide.md)**

    ---

    Configure, drive, read and handle faults, task by task.

-   :material-swap-horizontal: **[Porting](porting.md)**

    ---

    Implement the SPI interface for your MCU.

-   :material-book-open-variant: **[API reference](api-reference.md)**

    ---

    Every class and member, generated from the headers.

</div>
