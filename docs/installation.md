# Installation

Add the driver to a CMake or ESP-IDF project. It is a header-only C++20 library with no dependencies.

## Requirements

- A C++20 compiler: GCC 10+, Clang 12+ or a current MSVC. On C++23 the driver uses `std::expected`; on C++20 it
  uses the polyfill in [`inc/tle92466ed_expected.hpp`](../inc/tle92466ed_expected.hpp).
- A little-endian target: the frame CRC is computed over the frame's bytes in memory, and a `static_assert`
  stops the build on a big-endian one.
- No RTOS, heap or exceptions: every call is `noexcept` and returns a `tle92466ed::DriverResult<T>`.

## Get the source

A git submodule keeps the driver pinned to a version you chose:

```bash
git submodule add https://github.com/hardfoc/hf-tle92466ed-driver.git external/hf-tle92466ed-driver
git submodule update --init external/hf-tle92466ed-driver
```

Only `inc/`, `src/`, `cmake/` and `CMakeLists.txt` are needed to build. The examples have their own submodule
(`examples/esp32/scripts`); you do not need it to use the driver.

## CMake

```cmake
add_subdirectory(external/hf-tle92466ed-driver)
target_link_libraries(my_app PRIVATE hf_tle92466ed)   # or the alias hf::tle92466ed
```

The `hf_tle92466ed` target is an `INTERFACE` library: it carries the include directories (including the
generated version header) and requires C++20 (`cxx_std_20`), so your target does not need to set either.

| Option | Default | Effect |
| --- | --- | --- |
| `HF_TLE92466ED_ENABLE_WARNINGS` | `OFF` | `-Wall -Wextra -Wpedantic` on targets that link the driver |
| `HF_TLE92466ED_BUILD_TESTS` | `OFF` | Builds the host unit tests in `tests/` ([Testing and CI](testing.md)) |

Use the driver from source (submodule or `add_subdirectory`). The install rules do not yet copy
`src/tle92466ed.ipp`, which the header includes, so an installed copy does not build.

## ESP-IDF

The ESP-IDF component wrapper is
[`examples/esp32/components/hf_tle92466ed/`](../examples/esp32/components/hf_tle92466ed/CMakeLists.txt). Copy that
folder into your project's `components/` (or point `EXTRA_COMPONENT_DIRS` at it) and set the
`HF_TLE92466ED_ROOT` cache variable to the driver checkout; the wrapper stops with a clear message when the path is
wrong. Your component then declares `REQUIRES hf_tle92466ed`.

## How the build is organised

| Layer | File | Role |
| --- | --- | --- |
| Settings | [`cmake/hf_tle92466ed_build_settings.cmake`](../cmake/hf_tle92466ed_build_settings.cmake) | Include directories, version: the single source of truth |
| Library | [`CMakeLists.txt`](../CMakeLists.txt) | Interface library, install and package export |
| ESP-IDF | [`examples/esp32/components/hf_tle92466ed/CMakeLists.txt`](../examples/esp32/components/hf_tle92466ed/CMakeLists.txt) | IDF component wrapper over the same settings |

The implementation lives in [`src/tle92466ed.ipp`](../src/tle92466ed.ipp), which `tle92466ed.hpp` includes; do
not compile it on its own. The version is set once in the settings file and written into `tle92466ed_version.h`
at configure time. Read it at run time:

```cpp
const char* v = tle92466ed::Driver<MySpi>::GetDriverVersion();   // "1.0.0"
```

## Next

[Quickstart](quickstart.md) wires the chip and drives a channel; [Porting](porting.md) writes the SPI interface for
your MCU.
