# HybridClock Library

A hybrid analog-digital clock library for Arduino that combines a stepper motor-driven analog clock mechanism with NeoPixel LED ring displays.

## Features

- **Stepper Motor Control**: Precise minute hand positioning using 28BYJ-48 stepper motor
- **NeoPixel LED Display**: Dual LED rings for hours and minutes with multiple patterns
- **RTC Integration**: Accurate timekeeping with DS3231 real-time clock
- **Hour Change Animations**: Colorful windmill animation at hour transitions
- **Background Patterns**: 6 different LED patterns with automatic rotation
- **Quiet Hours**: Automatic brightness reduction during specified hours
- **Auto-Calibration**: Self-calibrating motor positioning with hall effect sensor
- **Micro-Calibration**: Periodic recalibration to maintain accuracy
- **Memory Efficient**: Optimized for ATmega328 (Arduino Nano) with only 27.6% RAM usage

## Hardware Requirements

- Arduino Nano or compatible (ATmega328)
- DS3231 RTC module
- 28BYJ-48 stepper motor with ULN2003 driver
- Hall effect sensor for motor position detection
- Two NeoPixel LED rings (typically 24 LEDs for hours, 12 LEDs for minutes)

## Installation

### Arduino IDE
1. Download this library
2. Copy the `HybridClock` folder to your Arduino libraries directory
   - Windows: `Documents/Arduino/libraries/`
   - Mac: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
3. Restart Arduino IDE
4. Open example: **File → Examples → HybridClock → BasicClock**

### PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps =
    HybridClock
```

## Quick Start

```cpp
#include <Wire.h>
#include <DS3231-RTC.h>
#include <HybridClock.h>

DS3231 rtc;

Clock hybridClock(
    2048,   // Steps per revolution (28BYJ-48)
    14,     // First motor pin (A0, pins A0-A3 used)
    2,      // Sensor pin
    6,      // NeoPixel pin
    24,     // Hour LEDs
    12,     // Minute LEDs
    63,     // Brightness
    11      // Motor speed
);

void setup() {
    Wire.begin();
    
    // Set centering adjustment if needed (calibrate your specific hardware)
    hybridClock.setCenteringAdjustment(9);  // Adjust based on your device
    
    hybridClock.begin(&rtc);
}

void loop() {
    hybridClock.update();
}
```

## Configuration Options

### Centering Adjustment

After calibration, the motor hand may not stop exactly at 12 o'clock. Fine-tune the position:

```cpp
// Positive values: move hand clockwise
// Negative values: move hand counter-clockwise
// Each unit ≈ 1 motor step (~0.18 degrees)
// Typical range: -10 to +10
hybridClock.setCenteringAdjustment(9);  // Adjust for your specific hardware
```

**If the motor arm becomes detached:**
1. Upload code and let calibration run
2. Watch for the **GREEN LED** (calibration success indicator)
3. **Immediately power off** when you see the green LED
4. Manually attach the arm pointing exactly at **12:00** (:00 position)
5. Power back on - the arm should now be correctly positioned

**Fine-tuning centering adjustment:**
1. Upload code with centering adjustment set to `0`
2. Observe where the hand stops after calibration
3. If it's slightly clockwise of 12:00, use a negative value
4. If it's slightly counter-clockwise of 12:00, use a positive value
5. Adjust and re-upload until hand is perfectly centered at 12:00

### Quiet Hours
Automatically dim LEDs during specified hours:
```cpp
hybridClock.enableQuietHours(true, 22, 6, 30);  // 10 PM to 6 AM, 30% brightness
```

### Pattern Rotation
Enable automatic LED pattern changes:
```cpp
hybridClock.getPatternManager().enableAutoRotation(true);
hybridClock.getPatternManager().setRotationInterval(1);  // Every hour
```

### Micro-Calibration
Periodic recalibration for accuracy:
```cpp
hybridClock.enableMicroCalibration(true, 4);  // Every 4 hours
```

### Custom Patterns
Configure which patterns are active:
```cpp
auto& pm = hybridClock.getPatternManager();
pm.disableAllPatterns();
pm.enablePattern(ClockDisplay::BREATHING_RINGS, true);
pm.enablePattern(ClockDisplay::RIPPLE_EFFECT, true);
```

## Serial Debugging

Serial output is **disabled by default** to save Flash memory (~2KB savings).

To enable Serial debugging, add this **before** including the library:
```cpp
#define HYBRIDCLOCK_ENABLE_SERIAL
#include <HybridClock.h>
```

Then enable verbose logging:
```cpp
Clock hybridClock(
    2048, 14, 2, 6, 24, 12, 63, 11,
    50,    // RTC check delay
    true   // Verbose logging
);
```

## Memory Usage

Optimized for resource-constrained devices:

**Arduino Nano (ATmega328: 2KB RAM, 30KB Flash)**
- RAM: 27.6% (565 bytes) - Safe operation with plenty of headroom
- Flash: 56.1% (17.2KB) - With Serial disabled
- Flash: 62.6% (19.2KB) - With Serial enabled

## API Reference

### Clock Class

#### Constructor
```cpp
Clock(int stepsPerRev, int firstMotorPin, int sensorPin, int neopixelPin,
      int hourLeds, int minuteLeds, uint8_t brightness, int motorSpeed,
      int rtcCheckDelay = 50, bool verboseLogging = false)
```

#### Methods
- `void begin(DS3231* rtcPtr = nullptr)` - Initialize clock system
- `void update()` - Update clock (call in loop())
- `void enableQuietHours(bool enable, int start, int end, int percent)`
- `void enableMicroCalibration(bool enable, int everyNHours = 4)`
- `void setCenteringAdjustment(int adjustment)` - Fine-tune motor positioning
- `AnimationManager& getAnimationManager()` - Access animation manager
- `PatternManager& getPatternManager()` - Access pattern manager

### LED Patterns

- `DEFAULT_COMPLEMENT` - Colored hour markers with white minutes
- `BREATHING_RINGS` - Pulsing rainbow effect
- `RIPPLE_EFFECT` - Expanding color ripples
- `SLOW_SPIRAL` - Rotating rainbow spiral
- `GENTLE_WAVES` - Wave-like color transitions
- `COLOR_DRIFT` - Slowly changing colors

## Examples

See `examples/BasicClock/BasicClock.ino` for a complete working example.

## License

See LICENSE file for details.

## Author

jhogsett

## Dependencies

- DS3231-RTC
- Adafruit NeoPixel
- Stepper (Arduino built-in)
- Wire (Arduino built-in)
