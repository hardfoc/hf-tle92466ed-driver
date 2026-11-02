# Examples

Complete ESP32-S3 applications that exercise the driver on a TLE92466ED, from an API test without loads to a
proportional-valve cycle.

They run on an ESP32-S3 wired as in [Hardware](hardware.md#evaluation-kit); the pins are in
[`esp32_tle92466ed_test_config.hpp`](../examples/esp32/main/esp32_tle92466ed_test_config.hpp) and the port in
[`esp32_tle92466ed_bus.hpp`](../examples/esp32/main/esp32_tle92466ed_bus.hpp). CI builds the apps marked "yes" on
each pull request; the others need their load and are built locally.

## The apps

<!-- generated: examples -->
| App | What it shows | Source | CI |
| --- | --- | --- | --- |
| `driver_integration_test` :material-star:{ title="featured" } | Driver API test suite against a connected TLE92466ED; no loads needed | [`driver_integration_test.cpp`](../examples/esp32/main/driver_integration_test.cpp) | yes |
| `lm_pro_cycle_test` :material-star:{ title="featured" } | Parker LM-Pro 24 V proportional valve on CH5: full-flow current ramp and diagnostics | [`lm_pro_cycle_test.cpp`](../examples/esp32/main/lm_pro_cycle_test.cpp) | no |
| `solenoid_control_test` :material-star:{ title="featured" } | Two solenoids, one on a single channel and one on a parallel pair, setpoint from an ADC input | [`solenoid_control_test.cpp`](../examples/esp32/main/solenoid_control_test.cpp) | no |
| `resistive_load_bench` :material-star:{ title="featured" } | 300 Ω load on CH0: setpoint sweep checked against the averaged feedback | [`resistive_load_bench.cpp`](../examples/esp32/main/resistive_load_bench.cpp) | no |
| `diagnostics_and_bist` | SFF_BIST, PIN_STAT, FAULT_MASK and the supply-monitor self-test | [`diagnostics_and_bist.cpp`](../examples/esp32/main/diagnostics_and_bist.cpp) | yes |

Build and flash one (from `examples/esp32`, ESP-IDF environment loaded):

```bash
./scripts/build_app.sh driver_integration_test Release
./scripts/flash_app.sh flash_monitor driver_integration_test Release
```

Source of this table: [`examples/esp32/app_config.yml`](../examples/esp32/app_config.yml).
<!-- /generated -->

## Set up once

```bash
git submodule update --init examples/esp32/scripts   # the HardFOC ESP-IDF build scripts
cd examples/esp32
. $IDF_PATH/export.sh                                 # ESP-IDF v5.5
./scripts/build_app.sh list                           # every app and build type
```

The scripts read [`app_config.yml`](../examples/esp32/app_config.yml) (the same file this page's table comes
from) and build each app as its own firmware image.

## What to read in an example

| You want to | Read |
| --- | --- |
| See every driver call once, with pass / fail output | `driver_integration_test.cpp` |
| Drive a real proportional valve with a current ramp | `lm_pro_cycle_test.cpp` |
| Run a parallel pair and set the current from a potentiometer | `solenoid_control_test.cpp` |
| Check current accuracy against a known resistor | `resistive_load_bench.cpp` (`ReadChannelFeedback()`) |
| Run the self-tests and read the pin and mask registers | `diagnostics_and_bist.cpp` |

The [example folder's README](../examples/esp32/README.md) and the per-app guides in
[`examples/esp32/docs/`](../examples/esp32/docs/README.md) have the wiring, the expected serial output and the
test framework.
