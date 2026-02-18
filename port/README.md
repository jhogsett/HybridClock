# HybridClock - Minimal Portable Version

This directory contains the minimal HybridClock library files for easy integration into other projects.

## What's Included

**Core Library Files:**
- `Clock.h` / `Clock.cpp` - Main clock orchestrator
- `ClockTime.h` / `ClockTime.cpp` - RTC time management
- `ClockMotor.h` / `ClockMotor.cpp` - Stepper motor control
- `ClockDisplay.h` / `ClockDisplay.cpp` - NeoPixel LED display
- `AnimationManager.h` / `AnimationManager.cpp` - Hour-change animations (minimal stubs)
- `PatternManager.h` / `PatternManager.cpp` - LED pattern management
- `HybridClock.h` - Convenience header (includes all components)

**Example:**
- `example_minimal.ino` - Minimal working example

## Features in Minimal Version

✅ **Included:**
- Motor calibration on startup
- Periodic micro-calibration (every 4 hours)
- Stepper motor control for minute hand
- LED display with default complementary hue pattern
- Time tracking with DS3231 RTC

❌ **Not Included (compiled out):**
- Hour-change animations
- Extra LED patterns (Breathing Rings, Ripple Effect, etc.)
- Serial debugging output
- Pattern rotation

## How to Use in Arduino IDE

### Option 1: As a Library

1. Create a folder in your Arduino libraries directory:
   - Windows: `Documents\Arduino\libraries\HybridClock\`
   - Mac: `~/Documents/Arduino/libraries/HybridClock/`
   - Linux: `~/Arduino/libraries/HybridClock/`

2. Copy all `.h` and `.cpp` files to that folder

3. Restart Arduino IDE

4. In your sketch:
   ```cpp
   #include <Wire.h>
   #include <DS3231-RTC.h>
   #include <HybridClock.h>
   
   DS3231 rtc;
   Clock hybridClock(2048, 14, 2, 6, 24, 12, 63, 11);
   
   void setup() {
       Wire.begin();
       hybridClock.setCenteringAdjustment(9);  // Adjust for your device
       hybridClock.enableMicroCalibration(true, 4);
       hybridClock.begin(&rtc);
   }
   
   void loop() {
       hybridClock.update();
   }
   ```

### Option 2: In Your Project Folder

1. Copy all `.h` and `.cpp` files into your project folder (same directory as your `.ino` file)

2. Arduino IDE will automatically compile them

3. Use the same code as above

## How to Use in PlatformIO

1. Create a `lib/HybridClock/` directory in your PlatformIO project

2. Copy all `.h` and `.cpp` files to `lib/HybridClock/`

3. In your `platformio.ini`:
   ```ini
   lib_deps = 
       hasenradball/DS3231-RTC@^1.1.0
       adafruit/Adafruit NeoPixel@^1.11.0
       arduino-libraries/Stepper@^1.1.3
   ```

4. In your `main.cpp`, use the same code as above

## Hardware Requirements

- Arduino Nano (ATmega328) or Nano Every (ATmega4809)
- DS3231 RTC module
- 28BYJ-48 stepper motor with ULN2003 driver
- Hall effect sensor for position detection
- 2 NeoPixel LED rings (24 LEDs for hours, 12 LEDs for minutes)

## Wiring

- **RTC:** SDA → A4, SCL → A5
- **Motor:** IN1-IN4 → pins 14-17 (A0-A3)
- **Sensor:** Digital pin 2
- **NeoPixels:** Data → pin 6

## Memory Usage

**Arduino Nano ATmega328:**
- RAM: 18.7% (382 bytes / 2048 bytes)
- Flash: 38.8% (11,912 bytes / 30,720 bytes)

**Arduino Nano Every ATmega4809:**
- RAM: 7.9% (486 bytes / 6144 bytes)
- Flash: 25.4% (12,378 bytes / 48,640 bytes)

## Optional Features

If you need more features, you can enable them by adding these defines BEFORE including the library:

```cpp
// Enable Serial debugging output (~2KB Flash)
#define HYBRIDCLOCK_ENABLE_SERIAL

// Enable hour-change animations (~600 bytes Flash)
#define HYBRIDCLOCK_ENABLE_ANIMATIONS

// Enable extra LED patterns (~600 bytes Flash)
#define HYBRIDCLOCK_ENABLE_EXTRA_PATTERNS

#include <HybridClock.h>
```

## Configuration

### Centering Adjustment

After calibration, fine-tune the motor position:

```cpp
// Positive values: move hand clockwise
// Negative values: move hand counter-clockwise
// Each unit ≈ 1 motor step (~0.18 degrees)
hybridClock.setCenteringAdjustment(9);
```

### Micro-Calibration

Enable periodic recalibration for accuracy:

```cpp
// Recalibrate every 4 hours
hybridClock.enableMicroCalibration(true, 4);
```

### Quiet Hours (Optional)

Automatically dim LEDs during specified hours:

```cpp
// 10 PM to 6 AM, 30% brightness
hybridClock.enableQuietHours(true, 22, 6, 30);
```

## License

See main project LICENSE file.

## Dependencies

- **DS3231-RTC** - Real-time clock module
- **Adafruit NeoPixel** - LED strip control
- **Stepper** - Arduino built-in stepper library
