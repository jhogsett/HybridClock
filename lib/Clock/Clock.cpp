#include "Clock.h"

Clock::Clock(int stepsPerRev, int firstMotorPin,
             int sensorPin, int neopixelPin, int hourLeds, int minuteLeds,
             uint8_t brightness, int motorSpeed, int rtcCheckDelay, bool verboseLogging)
    : clockMotor(stepsPerRev, firstMotorPin, firstMotorPin+1, firstMotorPin+2, firstMotorPin+3, sensorPin, motorSpeed, verboseLogging)
    , clockDisplay(neopixelPin, hourLeds, minuteLeds, brightness)
    , externalRTC(nullptr)
    , usingExternalRTC(false)
    , centeringAdjustment(0)
    , slowDelay(0)
    , rtcCheckDelay(rtcCheckDelay)
    , verboseLogging(verboseLogging)
    , quietHoursEnabled(false)
    , quietHoursStart(0)
    , quietHoursEnd(0)
    , quietBrightnessPercent(50)
    , defaultBrightness(brightness)
    , hourChangeAnimationEnabled(true)
    , testAnimationOnStartup(false)
    , microCalibrationEnabled(false)
    , microCalibrationInterval(4)
    , calibrated(false)
    , lastHourForAnimation(-1) {
}

void Clock::begin(DS3231* rtcPtr) {
    Serial.println(F("=== Clock System Starting ==="));
    
    // Store RTC reference
    if (rtcPtr != nullptr) {
        externalRTC = rtcPtr;
        usingExternalRTC = true;
        if (verboseLogging) Serial.println(F("Clock: Using external RTC instance"));
    } else {
        usingExternalRTC = false;
        if (verboseLogging) Serial.println(F("Clock: Using internal RTC instance"));
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
    
    if (verboseLogging) {
        Serial.print(F("Clock: Initial time - "));
        Serial.print(initialHour);
        Serial.print(F(":"));
        Serial.println(initialMinute);
    }
    
    // Set initial brightness based on quiet hours
    if (quietHoursEnabled) {
        updateQuietHoursBrightness();
    }
    
    // Show hour change animation on startup if enabled
    if (testAnimationOnStartup && hourChangeAnimationEnabled) {
        if (verboseLogging) Serial.println(F("Clock: Testing hour change animation on startup..."));
        int testHour = (initialHour + 1) % 24;
        if (verboseLogging) {
            Serial.print(F("Clock: Showing animation for hour "));
            Serial.println(testHour);
        }
        animationManager.playHourChangeAnimation(clockDisplay, testHour);
        if (verboseLogging) Serial.println(F("Clock: Hour change animation test complete"));
    }
    
    // Move to current minute position
    clockMotor.moveToMinute(initialMinute);
    
    Serial.println(F("=== Clock System Ready ==="));
}

void Clock::performCalibration() {
    if (verboseLogging) Serial.println(F("Clock: Starting calibration..."));
    
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
        if (verboseLogging) Serial.println(F("Clock: Calibration successful"));
    } else {
        // Show failure
        clockDisplay.clear();
        clockDisplay.getPixels().setPixelColor(0, clockDisplay.getPixels().Color(255, 0, 0));
        clockDisplay.show();
        delay(2000);
        Serial.println(F("Clock: Calibration FAILED!")); // CRITICAL - always show
    }
}

void Clock::enableQuietHours(bool enable, int start, int end, int percent) {
    quietHoursEnabled = enable;
    quietHoursStart = start;
    quietHoursEnd = end;
    quietBrightnessPercent = percent;
    
    if (verboseLogging) {
        if (enable) {
            Serial.print(F("Clock: Quiet hours enabled ("));
            Serial.print(start);
            Serial.print(F(":00 - "));
            Serial.print(end);
            Serial.print(F(":00, "));
            Serial.print(percent);
            Serial.println(F("% brightness)"));
        } else {
            Serial.println(F("Clock: Quiet hours disabled"));
        }
    }
}

void Clock::update() {
    // Update time from RTC
    if (!clockTime.update()) {
        // Second hasn't changed, nothing to do
        delay(rtcCheckDelay);
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
    
    // Debug output
    if (verboseLogging) {
        static uint32_t lastDebugTime = 0;
        if (minute == 59 && millis() - lastDebugTime > 5000) {
            Serial.print(F("Clock: Checking animation - minute=59, second="));
            Serial.print(second);
            Serial.print(F(", enabled="));
            Serial.print(hourChangeAnimationEnabled);
            Serial.print(F(", lastHourForAnimation="));
            Serial.println(lastHourForAnimation);
            lastDebugTime = millis();
        }
    }
    
    if (hourChangeAnimationEnabled && minute == 59 && (second == 57 || second == 58)) {
        int nextHour = (hour + 1) % 24;
        if (nextHour != lastHourForAnimation) {
            if (verboseLogging) {
                Serial.print(F("Clock: Hour transition animation ("));
                Serial.print(hour);
                Serial.print(F(" -> "));
                Serial.print(nextHour);
                Serial.println(F(")"));
            }
            
            animationManager.playHourChangeAnimation(clockDisplay, nextHour);
            
            // Perform micro-calibration if enabled
            if (microCalibrationEnabled && nextHour % microCalibrationInterval == 0) {
                if (verboseLogging) Serial.println(F("Clock: Performing micro-calibration"));
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
                // Check if next hour is within quiet hours
                bool isQuiet;
                if (quietHoursStart > quietHoursEnd) {
                    isQuiet = (nextHour >= quietHoursStart || nextHour < quietHoursEnd);
                } else {
                    isQuiet = (nextHour >= quietHoursStart && nextHour < quietHoursEnd);
                }
                clockDisplay.setQuietMode(isQuiet);
                updateQuietHoursBrightness();
            }
            
            lastHourForAnimation = nextHour;
        }
    }
    
    // Handle automatic pattern rotation
    if (patternManager.shouldRotate(hour)) {
        randomSeed(analogRead(A7) + hour);
        patternManager.selectRandomPattern();
        if (verboseLogging) {
            Serial.print(F("Clock: Pattern changed to "));
            Serial.println(patternManager.getPatternName(patternManager.getPattern()));
        }
    }
    
    // Update display
    updateDisplay();
}

void Clock::handleMinuteChange() {
    int minute = clockTime.getMinute();
    
    if (verboseLogging) {
        Serial.print(F("Clock: Minute changed to "));
        Serial.println(minute);
    }
    
    // Move hand to new position
    clockMotor.moveToMinute(minute);
}

void Clock::handleHourChange() {
    int hour = clockTime.getHour();
    
    if (verboseLogging) {
        Serial.print(F("Clock: Hour changed to "));
        Serial.println(hour);
    }
    
    // Update brightness if quiet hours changed
    if (quietHoursEnabled) {
        updateQuietHoursBrightness();
    }
}

void Clock::updateDisplay() {
    clockDisplay.clear();
    
    // Display current pattern from pattern manager
    clockDisplay.displayPattern(patternManager.getPattern());
    
    // Overlay hour indicators
    clockDisplay.showHourIndicators(clockTime.getHour12());
    
    clockDisplay.show();
}

void Clock::updateQuietHoursBrightness() {
    int currentHour = clockTime.getHour();
    
    // Check if current hour is within quiet hours
    bool isQuiet;
    if (quietHoursStart > quietHoursEnd) {
        // Quiet hours cross midnight
        isQuiet = (currentHour >= quietHoursStart || currentHour < quietHoursEnd);
    } else {
        // Quiet hours within same day
        isQuiet = (currentHour >= quietHoursStart && currentHour < quietHoursEnd);
    }
    
    clockDisplay.setQuietMode(isQuiet);
    
    uint8_t targetBrightness;
    if (isQuiet) {
        targetBrightness = (defaultBrightness * quietBrightnessPercent) / 100;
    } else {
        targetBrightness = defaultBrightness;
    }
    
    if (clockDisplay.getBrightness() != targetBrightness) {
        clockDisplay.setBrightness(targetBrightness);
        if (verboseLogging) {
            Serial.print(F("Clock: Brightness changed to "));
            Serial.print(targetBrightness);
            Serial.print(F(" ("));
            Serial.print(isQuiet ? F("QUIET") : F("ACTIVE"));
            Serial.println(F(" mode)"));
        }
    }
}
