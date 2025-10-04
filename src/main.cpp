#include <Arduino.h>
#include <Wire.h>
#include <DS3231-RTC.h>
#include "config.h"
#include <Clock.h>

// Hardware instances
DS3231 rtc;
Clock hybridClock;

void setup() {
    Serial.begin(115200);
    Serial.println("=== Hybrid Clock Starting ===");
    
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
        hybridClock.enableHourChangeAnimation(true);
    #endif
    
    // Enable micro-calibration every 4 hours
    hybridClock.enableMicroCalibration(true, 4);
    
    #ifdef ENABLE_PATTERN_SYSTEM
        #ifdef ENABLE_HOURLY_PATTERN_ROTATION
            hybridClock.enableHourlyPatternRotation(true);
        #else
            // Default to breathing rings pattern
            hybridClock.setDisplayPattern(ClockDisplay::BREATHING_RINGS);
        #endif
    #endif
    
    // Initialize clock with external RTC
    hybridClock.begin(&rtc);
    
    Serial.println("=== Setup Complete ===");
}

void loop() {
    // Simple update call handles everything
    hybridClock.update();
}
