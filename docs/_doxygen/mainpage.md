# TLE92466ED Driver API reference {#mainpage}

The classes, functions and register definitions of the HardFOC TLE92466ED driver, generated from the headers.

Start with tle92466ed::Driver, the driver and every channel, diagnostic and register call, and
tle92466ed::SpiInterface, the CRTP interface your platform port implements. Register addresses and bit fields are
in tle92466ed_registers.hpp (tle92466ed::CentralReg, tle92466ed::ChannelReg); configuration structures are
tle92466ed::GlobalConfig and tle92466ed::ChannelConfig.

The guides, with examples that compile, are on the documentation site: <a href="../">TLE92466ED Driver
documentation</a>.
