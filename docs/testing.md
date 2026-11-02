# Testing and CI

How the driver is checked: host unit tests against a register model of the chip, compiled documentation, ESP32
builds and static analysis, on every pull request.

## Host unit tests

The tests build the real driver against a model of the TLE92466ED's SPI behaviour: 32-bit frames with the CRC
checked on every request, the reply to each frame clocked out during the next one, 16-bit and 22-bit reply widths
per register, and the status and critical-fault replies. Faults are injected per test (idle MISO, wrong reply
width, critical-fault state, write status).

```bash
cmake -S . -B build-host -DHF_TLE92466ED_BUILD_TESTS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

| File | Covers |
| --- | --- |
| [`tests/test_frames.cpp`](../tests/test_frames.cpp) | CRC-8 SAE J1850 check value and byte order, write / read / reply frame layout, critical-fault flags |
| [`tests/test_transport.cpp`](../tests/test_transport.cpp) | Flush / command / dummy sequence, reply-width validation, `NoReply`, write status, CRC errors, `ReadMulti()`, critical faults |
| [`tests/test_driver.cpp`](../tests/test_driver.cpp) | `Init()` (RESN pulse, EN, `ICVID`), wrong device, modes, setpoint encoding single and parallel, argument errors |
| [`tests/mock_tle92466ed.hpp`](../tests/mock_tle92466ed.hpp) | The register model |
| [`tests/check_install.cmake`](../tests/check_install.cmake) | `cmake --install` into a scratch prefix, then a `find_package(hf_tle92466ed)` consumer builds the driver from it |

## Documentation snippets

Every C++ block on this site is included from [`tests/docs/`](../tests/docs/my_spi.hpp), which the host build
compiles (target `hf_tle92466ed_doc_snippets`) with the driver instantiated over the documented port. A change to
the API that breaks a page breaks the build first.

## CI workflows

| Workflow | What it checks |
| --- | --- |
| Host unit tests | GCC and Clang build and run the tests and the doc snippets |
| ESP32 examples build | Every app with `ci_enabled: true` in `examples/esp32/app_config.yml`, for each listed ESP-IDF version and build type |
| C++ lint, C++ analysis | clang-format, clang-tidy and cppcheck |
| Docs | Strict site build, figure and generated-table drift, Doxygen, link check, Lighthouse budget |
| Markdown lint, YAML lint, link check | Repository text files |

The workflows call the shared, versioned workflows of
[hf-general-ci-tools](https://github.com/hardfoc/hf-general-ci-tools) and
[hf-espidf-ci-tools](https://github.com/hardfoc/hf-espidf-ci-tools).

## On hardware

The [ESP32 examples](examples.md) are the hardware test suite: `driver_integration_test` and
`diagnostics_and_bist` need only the chip, the others need their load. Each prints a pass / fail summary over the
serial monitor.
