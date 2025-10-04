#include "Clock.h"

Clock::Clock(int stepsPerRev, int motorPin1, int motorPin2, int motorPin3, int motorPin4,
             int sensorPin, int neopixelPin, int hourLeds, int minuteLeds,
             uint8_t brightness, int motorSpeed)
    : clockMotor(stepsPerRev, motorPin1, motorPin2, motorPin3, motorPin4, sensorPin, motorSpeed)
    , clockDisplay(neopixelPin, hourLeds, minuteLeds, brightness)
    , externalRTC(nullptr)
    , usingExternalRTC(false)
    , centeringAdjustment(CENTERING_ADJUSTMENT)
    , slowDelay(SLOW_DELAY)
    , quietHoursEnabled(false)
    , quietHoursStart(QUIET_HOURS_START)
    , quietHoursEnd(QUIET_HOURS_END)
    , quietBrightnessPercent(QUIET_BRIGHTNESS_PERCENT)
    , defaultBrightness(brightness)
    , hourChangeAnimationEnabled(true)
    , microCalibrationEnabled(false)
    , microCalibrationInterval(4)
    , hourlyPatternRotation(false)
    , displayPattern(ClockDisplay::DEFAULT_COMPLEMENT)
    , calibrated(false)
    , lastHourForAnimation(-1)
    , lastHourForPattern(-1) {
}

void Clock::begin(DS3231* rtcPtr) {
    Serial.println("=== Clock System Starting ===");
    
    // Store RTC reference
    if (rtcPtr != nullptr) {
        externalRTC = rtcPtr;
        usingExternalRTC = true;
        Serial.println("Clock: Using external RTC instance");
    } else {
        usingExternalRTC = false;
        Serial.println("Clock: Using internal RTC instance");
    }
    
    // Initialize components
    clockTime.begin();
    clockMotor.begin();
    clockDisplay.begin();
    
    // Perform calibration
    performCalibration();
    
    // Get initial time
    clockTime.update();
    int initialMinute = clockTime.getMinute();
    int initialHour = clockTime.getHour();
    
    Serial.print("Clock: Initial time - ");
    Serial.print(initialHour);
    Serial.print(":");
    Serial.println(initialMinute);
    
    // Set initial brightness based on quiet hours
    if (quietHoursEnabled) {
        updateQuietHoursBrightness();
    }
    
    // Move to current minute position
    clockMotor.moveToMinute(initialMinute);
    
    Serial.println("=== Clock System Ready ===");
}

void Clock::performCalibration() {
    Serial.println("Clock: Starting calibration...");
    
    // Show calibration indicator
    clockDisplay.clear();
    clockDisplay.fill(clockDisplay.getPixels().Color(10, 10, 10));
    clockDisplay.show();
    
    // Calibrate motor
    calibrated = clockMotor.calibrate(centeringAdjustment, slowDelay);
    
    if (calibrated) {
        // Show success
        clockDisplay.clear();
        clockDisplay.getPixels().setPixelColor(0, clockDisplay.getPixels().Color(0, 255, 0));
        clockDisplay.show();
        delay(2000);
        Serial.println("Clock: Calibration successful");
    } else {
        // Show error
        clockDisplay.clear();
        clockDisplay.getPixels().setPixelColor(0, clockDisplay.getPixels().Color(255, 0, 0));
        clockDisplay.show();
        delay(2000);
        Serial.println("Clock: Calibration failed");
    }
}

void Clock::enableQuietHours(bool enable, int start, int end, int percent) {
    quietHoursEnabled = enable;
    quietHoursStart = start;
    quietHoursEnd = end;
    quietBrightnessPercent = percent;
    
    if (enable) {
        Serial.print("Clock: Quiet hours enabled (");
        Serial.print(start);
        Serial.print(":00 - ");
        Serial.print(end);
        Serial.print(":00, ");
        Serial.print(percent);
        Serial.println("% brightness)");
    } else {
        Serial.println("Clock: Quiet hours disabled");
    }
}

void Clock::update() {
    // Update time from RTC
    if (!clockTime.update()) {
        // Second hasn't changed, nothing to do
        delay(RTC_CHECK_DELAY);
        return;
    }
    
    // Handle minute change
    if (clockTime.hasMinuteChanged()) {
        handleMinuteChange();
    }
    
    // Handle hour change
    if (clockTime.hasHourChanged()) {
        handleHourChange();
    }
    
    // Check for hour change animation trigger (at seconds 57-58)
    int minute = clockTime.getMinute();
    int second = clockTime.getSecond();
    int hour = clockTime.getHour();
    
    if (hourChangeAnimationEnabled && minute == 59 && (second == 57 || second == 58)) {
        int nextHour = (hour + 1) % 24;
        if (nextHour != lastHourForAnimation) {
            Serial.print("Clock: Hour transition animation (");
            Serial.print(hour);
            Serial.print(" -> ");
            Serial.print(nextHour);
            Serial.println(")");
            
            clockDisplay.showWindmillHourChange(nextHour);
            
            // Perform micro-calibration if enabled
            if (microCalibrationEnabled && nextHour % microCalibrationInterval == 0) {
                Serial.println("Clock: Performing micro-calibration");
                clockMotor.powerOn();
                clockMotor.microCalibrate(centeringAdjustment, slowDelay);
                
                // After micro-calibration, hand is at position 0 (12 o'clock)
                // Move it back to current minute position (which should be 59 or 0)
                int currentMinute = clockTime.getMinute();
                clockMotor.moveToMinute(currentMinute);
                
                clockMotor.powerOff();
            }
            
            // Update brightness for next hour
            if (quietHoursEnabled) {
                clockDisplay.setQuietMode(isQuietHours(nextHour, quietHoursStart, quietHoursEnd));
                updateQuietHoursBrightness();
            }
            
            lastHourForAnimation = nextHour;
        }
    }
    
    // Handle hourly pattern rotation
    if (hourlyPatternRotation) {
        if (hour != lastHourForPattern) {
            if (lastHourForPattern != -1) {
                // Select random pattern (0-3 for first four patterns)
                randomSeed(analogRead(A7) + hour);
                displayPattern = (ClockDisplay::Pattern)random(4);
                Serial.print("Clock: Pattern changed to ");
                Serial.println(displayPattern);
            }
            lastHourForPattern = hour;
        }
    }
    
    // Update display
    updateDisplay();
}

void Clock::handleMinuteChange() {
    int minute = clockTime.getMinute();
    
    Serial.print("Clock: Minute changed to ");
    Serial.println(minute);
    
    // Move hand to new position
    clockMotor.moveToMinute(minute);
}

void Clock::handleHourChange() {
    int hour = clockTime.getHour();
    
    Serial.print("Clock: Hour changed to ");
    Serial.println(hour);
    
    // Update brightness if quiet hours changed
    if (quietHoursEnabled) {
        updateQuietHoursBrightness();
    }
}

void Clock::updateDisplay() {
    clockDisplay.clear();
    
    // Display current pattern
    clockDisplay.displayPattern(displayPattern);
    
    // Overlay hour indicators
    clockDisplay.showHourIndicators(clockTime.getHour12());
    
    clockDisplay.show();
}

void Clock::updateQuietHoursBrightness() {
    int currentHour = clockTime.getHour();
    bool isQuiet = isQuietHours(currentHour, quietHoursStart, quietHoursEnd);
    
    clockDisplay.setQuietMode(isQuiet);
    
    uint8_t targetBrightness;
    if (isQuiet) {
        targetBrightness = getQuietBrightness(defaultBrightness, quietBrightnessPercent);
    } else {
        targetBrightness = defaultBrightness;
    }
    
    if (clockDisplay.getBrightness() != targetBrightness) {
        clockDisplay.setBrightness(targetBrightness);
        Serial.print("Clock: Brightness changed to ");
        Serial.print(targetBrightness);
        Serial.print(" (");
        Serial.print(isQuiet ? "QUIET" : "ACTIVE");
        Serial.println(" mode)");
    }
}
