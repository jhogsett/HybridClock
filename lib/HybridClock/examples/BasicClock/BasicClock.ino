/**
 * BasicClock.ino
 * 
 * A simple example of using the HybridClock library to create a hybrid
 * analog-digital clock with stepper motor and NeoPixel LED displays.
 * 
 * This example enables Serial debugging to show library output.
 * To disable Serial debugging (saves ~2KB Flash), comment out the
 * HYBRIDCLOCK_ENABLE_SERIAL line below.
 * 
 * Hardware Required:
 * - Arduino Nano (or compatible)
 * - DS3231 RTC module
 * - 28BYJ-48 stepper motor with ULN2003 driver
 * - Hall effect sensor for motor position detection
 * - Two NeoPixel LED rings (24 LEDs for hours, 12 LEDs for minutes)
 * 
 * Wiring:
 * - RTC: SDA to A4, SCL to A5
 * - Motor: IN1-IN4 to pins 14-17 (A0-A3)
 * - Sensor: Digital pin 2
 * - NeoPixels: Data to pin 6
 */

// Enable Serial debugging output from the library
#define HYBRIDCLOCK_ENABLE_SERIAL

#include <Wire.h>
#include <DS3231-RTC.h>
#include <HybridClock.h>

// Hardware configuration
#define STEPS_PER_REVOLUTION  2048      // Steps for 28BYJ-48 motor
#define FIRST_MOTOR_PIN       14        // A0 (pins A0-A3 used sequentially)
#define SENSOR_PIN            2         // Hall effect sensor
#define NEOPIXEL_PIN          6         // NeoPixel data pin
#define HOUR_LEDS             24        // LEDs in hour ring
#define MINUTE_LEDS           12        // LEDs in minute ring
#define DEFAULT_BRIGHTNESS    63        // LED brightness (0-255)
#define MOTOR_SPEED           11        // Motor speed (higher = faster)

// RTC instance
DS3231 rtc;

// Clock instance with verbose logging enabled
Clock hybridClock(
    STEPS_PER_REVOLUTION,
    FIRST_MOTOR_PIN,
    SENSOR_PIN,
    NEOPIXEL_PIN,
    HOUR_LEDS,
    MINUTE_LEDS,
    DEFAULT_BRIGHTNESS,
    MOTOR_SPEED,
    50,     // RTC check delay (ms)
    true    // Verbose logging enabled
);

void setup() {
    Serial.begin(115200);
    Serial.println(F("=== Hybrid Clock Starting ==="));
    
    // Initialize I2C for RTC
    Wire.begin();
    
    // IMPORTANT: Centering Adjustment & Calibration
    // 
    // FIRST TIME SETUP / ARM DETACHED:
    // If the motor arm becomes detached or this is first time setup:
    // 1. Upload this code and let it run through calibration
    // 2. Watch for the GREEN LED (calibration success indicator)
    // 3. IMMEDIATELY power off the device when you see the green LED
    // 4. Manually attach the arm pointing exactly at 12:00 (:00 position)
    // 5. Power back on - the arm should now be correctly positioned
    //
    // FINE-TUNING CENTERING:
    // After calibration, if the motor stops slightly off from 12 o'clock:
    // - Positive values move the hand clockwise
    // - Negative values move the hand counter-clockwise
    // - Each unit is approximately 1 motor step (~0.18 degrees)
    // - Start with 0 and adjust based on your hardware (typical range: -10 to +10)
    hybridClock.setCenteringAdjustment(9);  // Example: 9 works for one specific device
    
    // Optional: Enable quiet hours (dimmed from 10 PM to 6 AM)
    hybridClock.enableQuietHours(true, 22, 6, 30);  // 30% brightness
    
    // Optional: Enable pattern rotation every hour
    hybridClock.getPatternManager().enableAutoRotation(true);
    hybridClock.getPatternManager().setRotationInterval(1);
    
    // Optional: Enable micro-calibration every 4 hours
    hybridClock.enableMicroCalibration(true, 4);
    
    // Initialize clock with RTC
    hybridClock.begin(&rtc);
    
    Serial.println(F("=== Setup Complete ==="));
}

void loop() {
    // Update clock - handles motor, display, and animations
    hybridClock.update();
}
