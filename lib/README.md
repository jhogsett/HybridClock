# HybridClock Portable Libraries

This directory contains portable, reusable clock libraries that can be integrated into other projects.

## Libraries

### ClockTime
Manages RTC (DS3231) time reading and change detection.

**Features:**
- Simple interface for reading hour, minute, second
- Automatic change detection (second, minute, hour)
- 12-hour format support
- Previous value tracking

**Usage:**
```cpp
#include <ClockTime.h>

ClockTime clockTime;

void setup() {
    clockTime.begin();
}

void loop() {
    if (clockTime.update()) {
        // Second has changed
        if (clockTime.hasMinuteChanged()) {
            int minute = clockTime.getMinute();
            // Handle minute change
        }
    }
}
```

### ClockMotor
Handles stepper motor control, calibration, and position tracking.

**Features:**
- Full calibration with hall effect sensor
- Micro-calibration for drift correction
- Power management (on/off control)
- Position tracking with wrap-around handling
- Move to specific minute position

**Usage:**
```cpp
#include <ClockMotor.h>

ClockMotor motor(STEPS_PER_REVOLUTION, PIN1, PIN2, PIN3, PIN4, SENSOR_PIN);

void setup() {
    motor.begin();
    motor.calibrate(CENTERING_ADJUSTMENT, SLOW_DELAY);
    motor.powerOff(); // Save power when idle
}

void loop() {
    motor.moveToMinute(currentMinute);
}
```

### ClockDisplay
Manages NeoPixel LED display with multiple patterns.

**Features:**
- 6 different display patterns
- Hour indicator overlay
- Windmill hour change animation
- Quarter-hour celebration effects
- Quiet mode brightness adjustment
- Automatic hue cycling

**Patterns:**
- Default Complement - Original complementary hue pattern
- Breathing Rings - Gentle pulsing effect
- Ripple Effect - Ripples from 12 o'clock
- Slow Spiral - Spiraling colors at different speeds
- Gentle Waves - Wave-like movement
- Color Drift - Smooth color transitions

**Usage:**
```cpp
#include <ClockDisplay.h>

ClockDisplay display(NEOPIXEL_PIN, HOUR_LEDS, MINUTE_LEDS);

void setup() {
    display.begin();
    display.setCurrentPattern(ClockDisplay::BREATHING_RINGS);
}

void loop() {
    display.clear();
    display.displayPattern(display.getCurrentPattern());
    display.showHourIndicators(hour12);
    display.show();
}
```

### ClockConfig
Portable configuration with sensible defaults.

**Features:**
- Hardware pin definitions
- Motor configuration
- LED configuration
- Quiet hours helpers
- All values can be overridden

**Usage:**
```cpp
#include <ClockConfig.h>

// Use defaults or override in your config.h before including

// Check quiet hours
if (isQuietHours(currentHour)) {
    uint8_t brightness = getQuietBrightness(DEFAULT_BRIGHTNESS);
    display.setBrightness(brightness);
}
```

## Integration

To use these libraries in another project:

1. Copy the `lib/Clock*` folders to your project's `lib/` directory
2. Include the headers you need:
   ```cpp
   #include <ClockTime.h>
   #include <ClockMotor.h>
   #include <ClockDisplay.h>
   #include <ClockConfig.h>
   ```
3. Override configuration values in your project's `config.h` if needed
4. Instantiate and use the classes

## Dependencies

- **ClockTime**: Wire.h, DS3231-RTC.h
- **ClockMotor**: Stepper.h
- **ClockDisplay**: Adafruit_NeoPixel.h
- **ClockConfig**: None (header only)

## Example Integration

See the main `src/main.cpp` for a complete working example of all libraries integrated together.
