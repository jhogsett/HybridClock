#include "Clock.h"

// Uncomment to enable Serial debugging output from the library
// #define HYBRIDCLOCK_ENABLE_SERIAL

// Uncomment to enable hour-change animations (saves ~600 bytes Flash)
// #define HYBRIDCLOCK_ENABLE_ANIMATIONS

#ifdef HYBRIDCLOCK_ENABLE_SERIAL
  #define SERIAL_PRINT(x) Serial.print(x)
  #define SERIAL_PRINTLN(x) Serial.println(x)
#else
  #define SERIAL_PRINT(x)
  #define SERIAL_PRINTLN(x)
#endif

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
    SERIAL_PRINTLN(F("=== Clock System Starting ==="));
    
    // Store RTC reference
    if (rtcPtr != nullptr) {
        externalRTC = rtcPtr;
        usingExternalRTC = true;
        if (verboseLogging) SERIAL_PRINTLN(F("Clock: Using external RTC instance"));
    } else {
        usingExternalRTC = false;
        if (verboseLogging) SERIAL_PRINTLN(F("Clock: Using internal RTC instance"));
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
#ifdef HYBRIDCLOCK_ENABLE_ANIMATIONS
    int initialHour = clockTime.getHour();
#endif
    
#ifdef HYBRIDCLOCK_ENABLE_SERIAL
    if (verboseLogging) {
        SERIAL_PRINT(F("Clock: Initial time - "));
        SERIAL_PRINT(clockTime.getHour());
        SERIAL_PRINT(F(":"));
        SERIAL_PRINTLN(initialMinute);
    }
#endif
    
    // Set initial brightness based on quiet hours
    if (quietHoursEnabled) {
        updateQuietHoursBrightness();
    }
    
#ifdef HYBRIDCLOCK_ENABLE_ANIMATIONS
    // Show hour change animation on startup if enabled
    if (testAnimationOnStartup && hourChangeAnimationEnabled) {
        if (verboseLogging) SERIAL_PRINTLN(F("Clock: Testing hour change animation on startup..."));
        int testHour = (initialHour + 1) % 24;
        if (verboseLogging) {
            SERIAL_PRINT(F("Clock: Showing animation for hour "));
            SERIAL_PRINTLN(testHour);
        }
        animationManager.playHourChangeAnimation(clockDisplay, testHour);
        if (verboseLogging) SERIAL_PRINTLN(F("Clock: Hour change animation test complete"));
    }
#endif
    
    // Move to current minute position
    clockMotor.moveToMinute(initialMinute);
    
    SERIAL_PRINTLN(F("=== Clock System Ready ==="));
}

void Clock::performCalibration() {
    if (verboseLogging) SERIAL_PRINTLN(F("Clock: Starting calibration..."));
    
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
        if (verboseLogging) SERIAL_PRINTLN(F("Clock: Calibration successful"));
    } else {
        // Show failure
        clockDisplay.clear();
        clockDisplay.getPixels().setPixelColor(0, clockDisplay.getPixels().Color(255, 0, 0));
        clockDisplay.show();
        delay(2000);
        SERIAL_PRINTLN(F("Clock: Calibration FAILED!")); // CRITICAL - always show
    }
}

void Clock::enableQuietHours(bool enable, int start, int end, int percent) {
    quietHoursEnabled = enable;
    quietHoursStart = start;
    quietHoursEnd = end;
    quietBrightnessPercent = percent;
    
    if (verboseLogging) {
        if (enable) {
            SERIAL_PRINT(F("Clock: Quiet hours enabled ("));
            SERIAL_PRINT(start);
            SERIAL_PRINT(F(":00 - "));
            SERIAL_PRINT(end);
            SERIAL_PRINT(F(":00, "));
            SERIAL_PRINT(percent);
            SERIAL_PRINTLN(F("% brightness)"));
        } else {
            SERIAL_PRINTLN(F("Clock: Quiet hours disabled"));
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
    
    int minute = clockTime.getMinute();
#ifdef HYBRIDCLOCK_ENABLE_ANIMATIONS
    int second = clockTime.getSecond();
#endif
    int hour = clockTime.getHour();
    
#ifdef HYBRIDCLOCK_ENABLE_ANIMATIONS
    // Hour change animation - trigger at exactly 59:58 for seamless timing
    bool hourTransitionHandled = false;
    if (hourChangeAnimationEnabled && minute == 59 && second == 58) {
        int nextHour = (hour + 1) % 24;
        if (nextHour != lastHourForAnimation) {
#ifdef HYBRIDCLOCK_ENABLE_SERIAL
            if (verboseLogging) {
                SERIAL_PRINT(F("Clock: Hour transition animation ("));
                SERIAL_PRINT(hour);
                SERIAL_PRINT(F(" -> "));
                SERIAL_PRINT(nextHour);
                SERIAL_PRINTLN(F(")"));
            }
#endif
            
            // Animation runs for exactly 2 seconds (59:58 -> 00:00)
            animationManager.playHourChangeAnimation(clockDisplay, nextHour);
            
            // Now we're at 00:00 - immediately move hand to home position
            clockMotor.moveToMinute(0);
            
            // Perform micro-calibration if enabled
            if (microCalibrationEnabled && nextHour % microCalibrationInterval == 0) {
#ifdef HYBRIDCLOCK_ENABLE_SERIAL
                if (verboseLogging) SERIAL_PRINTLN(F("Clock: Performing micro-calibration"));
#endif
                clockMotor.powerOn();
                clockMotor.microCalibrate(centeringAdjustment, slowDelay);
                
                // After micro-calibration, hand is already at position 0
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
            hourTransitionHandled = true;
        }
    }
#else
    // Without animations, just handle micro-calibration at hour changes
    bool hourTransitionHandled = false;
    if (clockTime.hasHourChanged()) {
        int currentHour = clockTime.getHour();
        
        // Perform micro-calibration if enabled at the appropriate hours
        if (microCalibrationEnabled && currentHour % microCalibrationInterval == 0 && minute == 0) {
#ifdef HYBRIDCLOCK_ENABLE_SERIAL
            if (verboseLogging) SERIAL_PRINTLN(F("Clock: Performing micro-calibration"));
#endif
            clockMotor.powerOn();
            clockMotor.microCalibrate(centeringAdjustment, slowDelay);
            clockMotor.moveToMinute(0);  // Return to current position
            clockMotor.powerOff();
        }
    }
#endif
    
    // Handle minute change (but skip if we just handled hour transition)
    if (!hourTransitionHandled && clockTime.hasMinuteChanged()) {
        handleMinuteChange();
    }
    
    // Handle hour change (but skip if we just handled hour transition)
    if (!hourTransitionHandled && clockTime.hasHourChanged()) {
        handleHourChange();
    }
    
    // Handle automatic pattern rotation
    if (patternManager.shouldRotate(hour)) {
        randomSeed(analogRead(A7) + hour);
        patternManager.selectRandomPattern();
        if (verboseLogging) {
#ifdef DEBUG_PATTERN_NAMES
            SERIAL_PRINT(F("Clock: Pattern changed to "));
            SERIAL_PRINTLN(patternManager.getPatternName(patternManager.getPattern()));
#else
            SERIAL_PRINT(F("Clock: Pattern changed to "));
            SERIAL_PRINTLN(patternManager.getPattern());
#endif
        }
    }
    
    // Update display
    updateDisplay();
}

void Clock::handleMinuteChange() {
    int minute = clockTime.getMinute();
    
    if (verboseLogging) {
        SERIAL_PRINT(F("Clock: Minute changed to "));
        SERIAL_PRINTLN(minute);
    }
    
    // Move hand to new position
    clockMotor.moveToMinute(minute);
}

void Clock::handleHourChange() {
#ifdef HYBRIDCLOCK_ENABLE_SERIAL
    if (verboseLogging) {
        int hour = clockTime.getHour();
        SERIAL_PRINT(F("Clock: Hour changed to "));
        SERIAL_PRINTLN(hour);
    }
#endif
    
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
            SERIAL_PRINT(F("Clock: Brightness changed to "));
            SERIAL_PRINT(targetBrightness);
            SERIAL_PRINT(F(" ("));
            SERIAL_PRINT(isQuiet ? F("QUIET") : F("ACTIVE"));
            SERIAL_PRINTLN(F(" mode)"));
        }
    }
}
