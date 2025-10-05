#include <Arduino.h>
#include <Wire.h>
#include <DS3231-RTC.h>
#include "config.h"
#include <Clock.h>

// Hardware instances
DS3231 rtc;

// Instantiate Clock with all configuration from config.h
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
    #ifdef ENABLE_VERBOSE_LOGGING
        true                 // verboseLogging
    #else
        false
    #endif
);

void setup() {
    Serial.begin(115200);
    Serial.println(F("=== Hybrid Clock Starting ==="));
    
    // Initialize I2C for RTC
    Wire.begin();
    
    // Configure clock
    #ifdef BLACK_DEVICE
        hybridClock.setCenteringAdjustment(9);
    #elif defined(WHITE_DEVICE)
        hybridClock.setCenteringAdjustment(3);
    #endif
    
    #ifdef ENABLE_QUIET_HOURS
        hybridClock.enableQuietHours(true, QUIET_HOURS_START, QUIET_HOURS_END, QUIET_BRIGHTNESS_PERCENT);
    #endif
    
    #ifdef TEST_HOUR_CHANGE_ON_STARTUP
        hybridClock.enableTestAnimationOnStartup(true);
    #endif
    
    // Enable micro-calibration every 4 hours
    hybridClock.enableMicroCalibration(true, 4);
    
    #ifdef ENABLE_PATTERN_SYSTEM
        #ifdef ENABLE_HOURLY_PATTERN_ROTATION
            // Enable automatic pattern rotation using PatternManager
            hybridClock.getPatternManager().enableAutoRotation(true);
            hybridClock.getPatternManager().setRotationInterval(1);  // Rotate every hour
            
            // Enable first 4 patterns for rotation (default behavior)
            // Or customize: hybridClock.getPatternManager().enableAllPatterns();
        #else
            // Default to breathing rings pattern
            hybridClock.setDisplayPattern(ClockDisplay::BREATHING_RINGS);
        #endif
        
        // Optional: Configure animation manager
        // hybridClock.getAnimationManager().setAnimation(AnimationManager::WINDMILL);
        // hybridClock.getAnimationManager().enableRandomSelection(true);
    #endif
    
    // Initialize clock with external RTC
    hybridClock.begin(&rtc);
    
    Serial.println(F("=== Setup Complete ==="));
}

void loop() {
    // Simple update call handles everything
    hybridClock.update();
}
