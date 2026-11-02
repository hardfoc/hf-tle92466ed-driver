# TLE92466ED Test Framework Documentation

## Overview

The **TLE92466ED Test Framework** (`TLE92466ED_TestFramework.hpp`) provides professional
testing infrastructure for ESP32-based examples, adapted from the HardFOC Internal
Interface Wrap test framework.

## Purpose

- **Structured Testing**: Organize tests into logical sections
- **Automatic Tracking**: Pass/fail counting and execution timing
- **Visual Feedback**: GPIO14 progress indicator
- **Task Isolation**: FreeRTOS task-based execution
- **CI/CD Ready**: Structured output for automated testing

## Framework Components

### 1. GPIO14 Test Progression Indicator

#### Purpose
Provides **hardware-level visual feedback** showing test progression on oscilloscope,
logic analyzer, or LED.

#### Functions

```cpp
bool init_test_progress_indicator() noexcept;
```text
**Initialize GPIO14** as output for test progression indicator
- Configures GPIO14 as push-pull output
- Sets initial state to LOW
- Returns `true` on success

```cpp
void flip_test_progress_indicator() noexcept;
```text
**Toggle GPIO14** state to indicate test completion
- Alternates between HIGH and LOW
- Called automatically after each test
- 50ms delay for visual effect

```cpp
void output_section_indicator(uint8_t blink_count = 5) noexcept;
```text
**Blink GPIO14** to indicate section boundaries
- Default: 5 blinks (50ms ON, 50ms OFF each)
- Custom blink count for different sections
- Marks state as LOW after completion

```cpp
void cleanup_test_progress_indicator() noexcept;
```text
**Cleanup and reset** GPIO14
- Sets pin to LOW
- Resets pin configuration
- Called before system restart

#### Signal Patterns

```text
Section Start:  ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
                └─┘ └─┘ └─┘ └─┘ └─┘  (5 blinks, 50ms each)

Test Complete:  ┌─────────────┐       (Toggle HIGH)
                └─────────────┘       
                
Next Test:      ────────────────      (Toggle LOW)
```text

#### Hardware Setup

```text
ESP32-C6 GPIO14 ──┬── LED Anode
                  │   └── LED Cathode ── 220Ω ── GND
                  │
                  └── Oscilloscope/Logic Analyzer Probe
```text

---

### 2. Test Result Tracking

#### TestResults Structure

```cpp
struct TestResults {
    int total_tests;              // Total number of tests run
    int passed_tests;             // Number of tests passed
    int failed_tests;             // Number of tests failed
    uint64_t total_execution_time_us;  // Total execution time (μs)
    
    void add_result(bool passed, uint64_t execution_time) noexcept;
    double get_success_percentage() const noexcept;
    double get_total_time_ms() const noexcept;
    void print_summary(const char* tag) const noexcept;
};
```text

#### Global Instance

```cpp
static TestResults g_test_results;
```text

Automatically accumulates results from all tests. Accessed via macros.

#### Methods

**`add_result(passed, execution_time)`**
- Records test result
- Updates pass/fail counters
- Accumulates execution time
- Called automatically by test macros

**`get_success_percentage()`**
- Returns success rate (0.0 to 100.0)
- Handles division by zero

**`get_total_time_ms()`**
- Returns total execution time in milliseconds
- Converts from microseconds

**`print_summary(tag)`**
- Prints professional formatted summary:
```text
╔══════════════════════════════════════════════════════════════════╗
║                    TEST RESULTS SUMMARY                          ║
╠══════════════════════════════════════════════════════════════════╣
║  Total Tests:      7                                             ║
║  Passed:           7                                             ║
║  Failed:           0                                             ║
║  Success Rate:     100.00%                                       ║
║  Total Time:       234.56 ms                                     ║
╚══════════════════════════════════════════════════════════════════╝
```text

---

### 3. Test Execution Macros

#### RUN_TEST(test_func)

**Inline test execution** (no separate task)

```cpp
RUN_TEST(test_hal_initialization);
```text

**Features**:
- Executes test in current task context
- Prints formatted header
- Measures execution time (microseconds)
- Records result in `g_test_results`
- Flips GPIO14 on completion
- 100ms delay after test

**Use Case**: Simple tests that don't need large stack or isolation

**Output**:
```text
╔══════════════════════════════════════════════════════════════════╗
║ Running: test_hal_initialization                                 
╚══════════════════════════════════════════════════════════════════╝
[SUCCESS] PASSED: test_hal_initialization (12.34 ms)
```text

---

#### RUN_TEST_IN_TASK(name, func, stack_size_bytes, priority)

**FreeRTOS task-based test execution**

```cpp
RUN_TEST_IN_TASK("hal_init", test_hal_initialization, 8192, 5);
```text

**Parameters**:
- `name`: Test name string (for logging)
- `func`: Test function pointer (`bool (*)() noexcept`)
- `stack_size_bytes`: Task stack size (e.g., `8192` = 8KB)
- `priority`: FreeRTOS task priority (`1-5`, typical: `5`)

**Features**:
- Executes test in **isolated FreeRTOS task**
- Custom stack size per test
- Automatic semaphore synchronization
- **30-second timeout** protection
- Fallback to inline execution on task creation failure
- Waits for test completion before continuing
- 100ms delay between tests

**Use Case**: Complex tests needing large stack or isolation

**Task Management**:
```cpp
struct TestTaskContext {
    const char* test_name;
    bool (*test_func)() noexcept;
    TestResults* results;
    const char* tag;
    SemaphoreHandle_t completion_semaphore;
};

void test_task_trampoline(void* param);
```text

**Output**:
```text
╔══════════════════════════════════════════════════════════════════╗
║ Running (task): hal_init                                         
╚══════════════════════════════════════════════════════════════════╝
[SUCCESS] PASSED (task): hal_init (12.34 ms)
Test task completed: hal_init
```text

**Timeout Handling**:
```cpp
if (xSemaphoreTake(semaphore, pdMS_TO_TICKS(30000)) == pdTRUE) {
    // Test completed
} else {
    ESP_LOGW(TAG, "Test task timeout: %s", name);
    // Still records result, continues to next test
}
```text

---

#### RUN_TEST_SECTION_IF_ENABLED(enabled, section_name, ...)

**Conditional test section execution**

```cpp
RUN_TEST_SECTION_IF_ENABLED(
    ENABLE_INITIALIZATION_TESTS, "INITIALIZATION TESTS",
    RUN_TEST_IN_TASK("hal_init", test_hal_initialization, 8192, 5);
    RUN_TEST_IN_TASK("driver_init", test_driver_initialization, 8192, 5);
);
```text

**Parameters**:
- `enabled`: Compile-time flag (`0` or `1`)
- `section_name`: Display name for section
- `...`: Test calls (variadic)

**Features**:
- **Compile-time** section enable/disable
- Professional section header
- **5-blink pattern** on GPIO14 at section start
- Wraps multiple test calls

**Output**:
```text
╔══════════════════════════════════════════════════════════════════╗
║ SECTION: INITIALIZATION TESTS                                    ║
╚══════════════════════════════════════════════════════════════════╝
[GPIO14: 5 blinks]
... tests ...
```text

---

#### RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(enabled, section_name, blink_count, ...)

**Conditional test section with custom blink pattern**

```cpp
RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
    ENABLE_ADVANCED_TESTS, "ADVANCED TESTS", 3,
    // Custom 3-blink pattern
    RUN_TEST_IN_TASK("advanced_test", test_advanced, 16384, 5);
);
```text

**Parameters**:
- `enabled`: Compile-time flag
- `section_name`: Display name
- `blink_count`: Custom blink count for this section
- `...`: Test calls

**Use Case**: Differentiate sections by blink pattern on oscilloscope

---

### 4. Test Section Configuration

#### print_test_section_status(tag, module_name)

Prints configuration information at test start:

```cpp
print_test_section_status(TAG, "TLE92466ED");
```text

**Output**:
```text
╔══════════════════════════════════════════════════════════════════╗
║ TLE92466ED TEST CONFIGURATION                                    
╠══════════════════════════════════════════════════════════════════╣
║ Test sections will execute based on compile-time configuration  ║
║ GPIO14 test progression indicator: ENABLED                       ║
╚══════════════════════════════════════════════════════════════════╝
```text

---

## Usage Guide

### Basic Setup

#### 1. Include Framework

```cpp
#include "TLE92466ED_TestFramework.hpp"

static const char* TAG = "MyTest";
```text

#### 2. Define Test Sections

```cpp
#define ENABLE_BASIC_TESTS 1
#define ENABLE_ADVANCED_TESTS 1
#define ENABLE_STRESS_TESTS 0    // Disabled
```text

#### 3. Write Test Functions

```cpp
static bool test_my_feature() noexcept {
    ESP_LOGI(TAG, "Testing my feature...");
    
    // Test logic here
    bool result = some_operation();
    
    if (result) {
        ESP_LOGI(TAG, "✅ Feature works!");
        return true;
    } else {
        ESP_LOGE(TAG, "❌ Feature failed!");
        return false;
    }
}
```text

**Requirements**:
- Return type: `bool`
- `noexcept` specifier
- `true` = passed, `false` = failed

#### 4. Create app_main()

```cpp
extern "C" void app_main() {
    // Print header
    ESP_LOGI(TAG, "╔════════════════════════════════════╗");
    ESP_LOGI(TAG, "║        MY TEST SUITE               ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════╝");
    
    // Initialize GPIO14
    init_test_progress_indicator();
    
    // Print configuration
    print_test_section_status(TAG, "MY_MODULE");
    
    // Run test sections
    RUN_TEST_SECTION_IF_ENABLED(
        ENABLE_BASIC_TESTS, "BASIC TESTS",
        RUN_TEST_IN_TASK("my_feature", test_my_feature, 8192, 5);
    );
    
    // Print results
    g_test_results.print_summary(TAG);
    
    // Cleanup
    cleanup_test_progress_indicator();
    
    // Status message
    if (g_test_results.failed_tests == 0) {
        ESP_LOGI(TAG, "✅ ALL TESTS PASSED!");
    } else {
        ESP_LOGE(TAG, "❌ %d TESTS FAILED!", g_test_results.failed_tests);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10000));
    esp_restart();
}
```text

---

## Advanced Usage

### Custom Test Context

For tests that need shared resources:

```cpp
// Global resources
static MyDriver* g_driver = nullptr;

static bool test_with_driver() noexcept {
    if (!g_driver) {
        ESP_LOGE(TAG, "Driver not initialized");
        return false;
    }
    
    // Use driver
    return g_driver->doSomething();
}
```text

### Stack Size Guidelines

| Test Complexity | Stack Size | Notes |
|----------------|-----------|-------|
| Simple | 4096 (4KB) | Basic operations, minimal locals |
| Normal | 8192 (8KB) | Standard tests (default) |
| Complex | 12288 (12KB) | Multiple buffers, recursion |
| Heavy | 16384 (16KB) | Large objects, deep call chains |

### Priority Guidelines

| Priority | Use Case |
|----------|----------|
| 1-2 | Low priority background tests |
| 3-4 | Normal test priority |
| 5 | **Default**, standard tests |
| 6-10 | High priority, time-critical tests |

### Timeout Considerations

Default timeout: **30 seconds**

Modify in framework if needed:
```cpp
if (xSemaphoreTake(ctx.completion_semaphore, pdMS_TO_TICKS(60000))) {
    // 60-second timeout for slow tests
}
```text

---

## Best Practices

### 1. Test Organization

```cpp
// Group related tests
RUN_TEST_SECTION_IF_ENABLED(
    ENABLE_INITIALIZATION_TESTS, "INITIALIZATION",
    RUN_TEST_IN_TASK("hal", test_hal_init, 8192, 5);
    RUN_TEST_IN_TASK("driver", test_driver_init, 8192, 5);
);

RUN_TEST_SECTION_IF_ENABLED(
    ENABLE_OPERATION_TESTS, "OPERATIONS",
    RUN_TEST_IN_TASK("read", test_read, 8192, 5);
    RUN_TEST_IN_TASK("write", test_write, 8192, 5);
);
```text

### 2. Clear Logging

```cpp
static bool test_feature() noexcept {
    ESP_LOGI(TAG, "Starting feature test...");
    ESP_LOGI(TAG, "Step 1: Initialize");
    ESP_LOGI(TAG, "Step 2: Execute");
    ESP_LOGI(TAG, "Step 3: Verify");
    
    ESP_LOGI(TAG, "✅ Feature test passed");
    return true;
}
```text

### 3. Error Reporting

```cpp
static bool test_operation() noexcept {
    if (auto result = do_operation(); !result) {
        ESP_LOGE(TAG, "❌ Operation failed: %s", result.error());
        return false;
    }
    return true;
}
```text

### 4. Resource Cleanup

```cpp
extern "C" void app_main() {
    // Setup
    init_test_progress_indicator();
    auto* resource = create_resource();
    
    // Tests
    RUN_TEST_SECTION_IF_ENABLED(...);
    
    // Cleanup (ALWAYS!)
    delete resource;
    cleanup_test_progress_indicator();
    g_test_results.print_summary(TAG);
}
```text

---

## Performance Impact

### Memory Overhead

| Component | Flash | RAM |
|-----------|-------|-----|
| Framework Code | ~8KB | ~2KB |
| Test Results | 0 | 32 bytes |
| Per Test Context | 0 | ~64 bytes (temporary) |
| GPIO14 State | 0 | 3 bytes |
| **Total** | **~8KB** | **~2KB + 64B per active task** |

### Timing Overhead

| Operation | Time |
|-----------|------|
| Test Header Print | ~5ms |
| GPIO14 Toggle | ~50μs |
| GPIO14 Blink (5x) | ~500ms |
| Semaphore Operations | ~10μs |
| Result Recording | ~1μs |
| **Per Test Overhead** | **~5-10ms** |

---

## Debugging

### Enable Framework Debug Output

```cpp
#define TEST_FRAMEWORK_DEBUG 1

// In framework functions:
#ifdef TEST_FRAMEWORK_DEBUG
    ESP_LOGI("TestFramework", "Debug info...");
#endif
```text

### Monitor GPIO14

```bash
# Logic analyzer configuration
Sample Rate: 1MHz
Duration: 10s
Trigger: Rising edge on GPIO14
```text

### Analyze Test Timing

```cpp
// Parse output for timing data
grep "PASSED" output.log | awk '{print $NF}'
```text

---

## Integration with CI/CD

### Structured Output Parsing

```bash
#!/bin/bash

# Extract results
TOTAL=$(grep "Total Tests:" output.log | awk '{print $3}')
PASSED=$(grep "Passed:" output.log | awk '{print $2}')
FAILED=$(grep "Failed:" output.log | awk '{print $2}')
SUCCESS_RATE=$(grep "Success Rate:" output.log | awk '{print $3}')

# Exit code
if [ "$FAILED" -gt 0 ]; then
    echo "TESTS FAILED: $FAILED/$TOTAL"
    exit 1
else
    echo "ALL TESTS PASSED: $PASSED/$TOTAL ($SUCCESS_RATE)"
    exit 0
fi
```text

### GitHub Actions Example

```yaml
- name: Run Tests
  run: |
    ./scripts/flash_app.sh basic_usage Debug
    ./scripts/monitor_app.sh basic_usage > output.log
    
- name: Parse Results
  run: |
    if grep "\[FAILED\]" output.log; then
      echo "Tests failed!"
      exit 1
    fi
```text

---

## References

- [BasicUsageExample.cpp](../main/BasicUsageExample.cpp) - Example implementation
- [HardFOC Internal TestFramework.h](
  ../../../../internal/hf-internal-interface-wrap/examples/esp32/main/TestFramework.h) -
  Original framework

---

**Framework Version**: 1.0.0  
**Based on**: HardFOC Internal Interface Wrap TestFramework.h  
**Platform**: ESP32 (ESP-IDF v5.0+)  
**Status**: ✅ Production Ready
