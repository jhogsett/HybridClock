# Verbose Logging Feature

## Overview
The Clock library now supports configurable verbose logging to help save Flash memory on resource-constrained devices like the Arduino Nano ATmega328.

## How It Works

### Runtime Control
The verbose logging is controlled via a boolean parameter passed to the Clock constructor:

```cpp
Clock hybridClock(
    STEPS_PER_REV,
    FIRST_MOTOR_PIN,
    SENSOR_PIN,
    NEOPIXEL_PIN,
    HOUR_LEDS,
    MINUTE_LEDS,
    BRIGHTNESS,
    MOTOR_SPEED,
    RTC_CHECK_DELAY,
    true  // verboseLogging - set to false to reduce output
);
```

### Configuration
In `config.h`, you can enable/disable verbose logging with:

```cpp
#define ENABLE_VERBOSE_LOGGING  // Comment this out to disable
```

In `main.cpp`, this is used as:

```cpp
Clock hybridClock(
    // ... other parameters ...
    #ifdef ENABLE_VERBOSE_LOGGING
        true   // Verbose logging ON
    #else
        false  // Verbose logging OFF
    #endif
);
```

## Message Categories

### Critical Messages (Always Shown)
These messages are always displayed regardless of the verboseLogging setting:
- `=== Clock System Starting ===` - System initialization
- `=== Clock Ready ===` - System ready
- `Clock: Calibration FAILED!` - Calibration errors
- Any error conditions

### Verbose Messages (Conditional)
These messages are only shown when `verboseLogging = true`:
- `Clock: Using external/internal RTC instance`
- `Clock: Initial time - HH:MM`
- `Clock: Testing hour change animation on startup...`
- `Clock: Minute changed to XX`
- `Clock: Hour changed to XX`
- `Clock: Brightness changed to XXX (QUIET/ACTIVE mode)`
- `Clock: Pattern changed to X`
- `Clock: Hour transition animation (X -> Y)`
- `Clock: Performing micro-calibration`
- `ClockMotor: Starting calibration...`
- `ClockMotor: Fwd Steps: XXX`
- `ClockMotor: Bak Steps: XXX`
- `ClockMotor: Center Steps: XXX`
- `ClockMotor: Calibration complete`
- `ClockMotor: Starting micro-calibration...`
- `ClockMotor: Micro-calibration complete`
- `ClockMotor: Micro-calibration skipped (magnet not found)`
- Debug output at minute 59

## Memory Considerations

### Current Implementation
The current implementation uses runtime checks (`if (verboseLogging)`). This approach:
- ✅ Allows dynamic control of logging
- ✅ Clean, readable code
- ⚠️ Keeps string literals in Flash memory even when disabled
- Memory savings: Minimal (strings still compiled in)

### For Maximum Memory Savings
If you need to squeeze out every byte of Flash memory, you can modify the library to use preprocessor directives instead:

```cpp
// Instead of:
if (verboseLogging) Serial.println("Message");

// Use:
#ifdef VERBOSE_LOGGING
Serial.println("Message");
#endif
```

This would require:
1. Defining `VERBOSE_LOGGING` as a compile-time flag in `platformio.ini`:
   ```ini
   build_flags = -DVERBOSE_LOGGING
   ```
2. Removing the runtime `verboseLogging` parameter
3. Replacing all `if (verboseLogging)` guards with `#ifdef VERBOSE_LOGGING`

**Estimated savings**: 200-300 bytes of Flash memory

## Usage Examples

### Normal Operation (Verbose)
```cpp
#define ENABLE_VERBOSE_LOGGING  // In config.h

// Output will show all messages:
// === Clock System Starting ===
// Clock: Using external RTC instance
// Clock: Initial time - 14:35
// ClockMotor: Starting calibration...
// ClockMotor: Fwd Steps: 42
// ClockMotor: Bak Steps: 40
// ClockMotor: Center Steps: 41
// ClockMotor: Calibration complete
// === Clock Ready ===
```

### Quiet Operation (Critical Only)
```cpp
//#define ENABLE_VERBOSE_LOGGING  // Commented out in config.h

// Output will only show:
// === Clock System Starting ===
// === Clock Ready ===
// (and any errors if they occur)
```

## Current Status
- ✅ Verbose logging parameter added to Clock class
- ✅ Verbose logging parameter added to ClockMotor class
- ✅ All verbose messages properly guarded
- ✅ Critical messages always displayed
- ✅ Configuration option in config.h
- ✅ Builds successfully
- ✅ Compatible with portable library design

## Recommendation
For development and debugging, keep `ENABLE_VERBOSE_LOGGING` defined. For production deployments where memory is tight, comment it out to reduce Serial output and improve clarity.
