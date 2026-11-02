"""The three layers of the driver: hardware-agnostic core, communication interface, platform port.

Reads: From top to bottom. Your application talks to one Driver<Comm> object per chip. The core owns the chip's
register map (from inc/tle92466ed_registers.hpp): the central configuration and status registers, and the bank of
channel registers that repeats for each of the six channels. The communication interface
(tle92466ed::SpiInterface in inc/tle92466ed_spi_interface.hpp) is a CRTP base: it builds 32-bit frames with the
CRC-8 SAE J1850 byte, runs the one-frame reply pipeline and validates every reply; the methods it dispatches to your
class are listed from the header. The platform port is the only code you write. Below it is the chip.
"""

import re
from pathlib import Path

from hardfoc_mkdocs_theme.figkit import Fig

ROOT = Path(__file__).resolve().parents[3]
REGS = "inc/tle92466ed_registers.hpp"
COMM = "inc/tle92466ed_spi_interface.hpp"


def _names(text: str, ns: str) -> list[tuple[str, int]]:
    body = re.search(rf"^namespace {ns} \{{(.*?)^\}}", text, re.S | re.M).group(1)
    return [(n, int(v, 16)) for n, v in re.findall(r"constexpr uint16_t (\w+) = (0x[0-9A-Fa-f]+);", body)]


def data():
    regs = (ROOT / REGS).read_text()
    central = _names(regs, "CentralReg")
    channel = _names(regs, "ChannelReg")
    comm = (ROOT / COMM).read_text()
    cls = re.search(r"class SpiInterface \{(.*?)^\};", comm, re.S | re.M).group(1)
    hooks = []
    for name in re.findall(r"static_cast<(?:const )?Derived\*>\(this\)->(\w+)\(", cls):
        if name not in hooks:
            hooks.append(name)
    return {
        "central": [n for n, v in central if v < 0x200],
        "status": [n for n, v in central if v >= 0x200],
        "channel": [n for n, _ in channel],
        "hooks": hooks,
        "sources": [f"{REGS}:CentralReg", f"{REGS}:ChannelReg", f"{COMM}:SpiInterface"],
    }


def build(d):
    f = Fig(
        "driver-layers",
        "Three layers, one of them yours",
        alt="Layer diagram: the application calls the hardware-agnostic TLE92466ED driver, which owns the central "
        "and per-channel register map, and uses the CRTP SPI interface that frames, checks and pipelines 32-bit "
        "transfers, which calls the platform port you write, which drives the SPI bus and the RESN, EN and FAULTN "
        "pins of the TLE92466ED.",
        width=1200,
        kicker="Architecture",
        subtitle="The driver and the interface are the same on every MCU. The port is one class that moves 32-bit "
        "frames and sets three pins.",
        profile="overview",
        legend=[("brand", "this library"), ("teal", "you write"), ("slate", "hardware")],
    )
    W = f.body_w
    app = f.card(0, 0, W, "Your application", ["valve control loop, state machine, diagnostics ..."], tone="slate")
    top = app.b + 40
    y = f.region_inner_top(top)
    rows = (
        ("Central configuration (0x00-0x3F)", d["central"]),
        ("Central status, read only (0x200-0x207)", d["status"]),
        ("Channel bank, six times (CH4 0x20, CH5 0x30, CH0 0x40 ... CH3 0x70; feedback at +0x200)", d["channel"]),
    )
    for label, names in rows:
        f.text(16, y + 12, label, size=f.k["small"], weight=700, color="brand.text", bg="brand.tint")
        h = f.chips(16, y + 22, W - 32, names, tone="brand")
        y = y + 22 + h + 18
    reg = f.region(0, top, W, y - top, "Hardware-agnostic core", tone="brand",
                   sub="tle92466ed::Driver<Comm>, inc/tle92466ed.hpp + src/tle92466ed.ipp")
    comm = f.card(0, reg.b + 40, W, "Communication interface",
                  ["tle92466ed::SpiInterface<Self>: CRTP, no virtual calls; 32-bit frames with CRC-8 SAE J1850,",
                   "flush / command / dummy pipeline, reply width and CRC checked, critical-fault frames decoded"],
                  tone="brand", accent=True)
    hooks = d["hooks"]
    half = (len(hooks) + 1) // 2
    port = f.card(0, comm.b + 40, W, "Platform port (your class)",
                  ["  ".join(h + "()" for h in hooks[:half]), "  ".join(h + "()" for h in hooks[half:])],
                  tone="teal", accent=True, mono=True)
    hw = f.card(0, port.b + 40, W, "TLE92466ED",
                ["SPI mode 1 up to 8 MHz, 32-bit frames  /  RESN, EN in, FAULTN open drain out"], tone="slate")
    for a, b in ((app, reg), (reg, comm), (comm, port), (port, hw)):
        f.connect(a, b, "bottom", "top", tone="rail")
    return f
