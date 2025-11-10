/**
 * main_minimal.cpp
 * 
 * Minimal HybridClock implementation with only essential features:
 * - Calibration and periodic micro-calibration
 * - LED and stepper motor control for displaying time
 * - Simple default complementary hue pattern on LED rings
 * 
 * No hour-change animations, no pattern rotation, no extra features.
 * Perfect for embedding in another project.
 * 
 * To use this instead of main.cpp, rename or swap the files.
 */

// Minimal build - no optional features enabled
// #define HYBRIDCLOCK_ENABLE_SERIAL         // Disabled to save Flash
// #define HYBRIDCLOCK_ENABLE_ANIMATIONS      // Disabled to save Flash
// #define HYBRIDCLOCK_ENABLE_EXTRA_PATTERNS  // Disabled to save Flash

#include <Arduino.h>
#include <Wire.h>
#include <DS3231-RTC.h>
#include "config.h"
#include <Clock.h>

// Hardware instances
DS3231 rtc;

// Instantiate Clock with minimal configuration
Clock hybridClock(
    STEPS_PER_REVOLUTION,    // stepsPerRev
    FIRST_MOTOR_PIN,         // firstMotorPin (pins +1, +2, +3 are automatic)
    SENSOR_PIN,              // sensorPin
    NEOPIXEL_PIN,            // neopixelPin
    HOUR_LEDS,               // hourLeds
    MINUTE_LEDS,             // minuteLeds
    DEFAULT_BRIGHTNESS,      // brightness
    MOTOR_SPEED,             // motorSpeed
    RTC_CHECK_DELAY,         // rtcCheckDelay
    false                    // verboseLogging (disabled for minimal build)
);

void setup() {
    // Initialize I2C for RTC
    Wire.begin();
    
    // Configure centering adjustment for your device
    #ifdef BLACK_DEVICE
        hybridClock.setCenteringAdjustment(9);
    #elif defined(WHITE_DEVICE)
        hybridClock.setCenteringAdjustment(3);
    #endif
    
    // Enable periodic micro-calibration every 4 hours
    hybridClock.enableMicroCalibration(true, 4);
    
    // Disable hour-change animation for minimal build
    hybridClock.enableHourChangeAnimation(false);
    
    // Set simple default complementary pattern (no rotation)
    hybridClock.setDisplayPattern(ClockDisplay::DEFAULT_COMPLEMENT);
    
    // Initialize clock with external RTC
    hybridClock.begin(&rtc);
}

void loop() {
    // Simple update call handles everything
    hybridClock.update();
}
