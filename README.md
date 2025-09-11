# HybridClock
## Advanced LED & Stepper Motor Dial Clock

A sophisticated Arduino-based timepiece combining analog and digital display elements with dynamic color patterns and intelligent calibration.

### Features
- **Hybrid Display**: Analog minute hand via precision stepper motor + digital hour display via NeoPixel LEDs
- **Auto-Calibration**: Hall effect sensor-based positioning with intelligent edge detection
- **Dynamic Visuals**: Complementary color cycling, hour change animations, and multiple display modes
- **Multiple Modes**: Normal time, rainbow, temperature display, date display, and calibration modes
- **Smart Power Management**: Motor power-down between movements to reduce heat and power consumption
- **Robust Design**: Error handling, calibration verification, and automatic midnight recalibration

### Hardware Requirements
- Arduino Nano (or compatible)
- DS3231 RTC Module (I2C connection: A4/A5)
- 28BYJ-48 Stepper Motor with ULN2003 Driver (pins A0-A3)
- 24-LED NeoPixel Ring (outer, hour display)
- 12-LED NeoPixel Ring (inner, background/effects)
- A3144 Hall Effect Sensor (pin 2, active LOW)
- 5V Power Supply (recommended: 2A+)

### Quick Start
1. Upload code using PlatformIO
2. Power on - automatic calibration will begin
3. Green LED at position 0 indicates successful calibration
4. Clock begins normal operation

### Display Modes
- **Normal**: Time display with color-cycling background
- **Rainbow**: Full spectrum color cycling
- **Temperature**: Color-coded temperature display (requires sensor)
- **Date**: Month/day visualization
- **Calibration**: Diagnostic display for setup

### Advanced Features
- Smooth stepper movements with configurable speed
- Edge detection for precise calibration
- Multiple device profiles (BLACK_DEVICE/WHITE_DEVICE)
- Serial debugging and diagnostics
- Modular, object-oriented code structure
