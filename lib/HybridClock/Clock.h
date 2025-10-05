#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <DS3231-RTC.h>
#include <ClockTime.h>
#include <ClockMotor.h>
#include <ClockDisplay.h>
#include <AnimationManager.h>
#include <PatternManager.h>

/**
 * Clock - Unified clock system
 * 
 * Orchestrates ClockTime, ClockMotor, and ClockDisplay to create
 * a complete clock system with a simple API.
 * 
 * Usage:
 *   Clock clock;
 *   
 *   void setup() {
 *     clock.begin(&myRTC);
 *   }
 *   
 *   void loop() {
 *     clock.update();
 *   }
 */
class Clock {
public:
    Clock(int stepsPerRev,
          int firstMotorPin,  // Other 3 pins are sequential: pin+1, pin+2, pin+3
          int sensorPin,
          int neopixelPin,
          int hourLeds,
          int minuteLeds,
          uint8_t brightness,
          int motorSpeed,
          int rtcCheckDelay = 50,
          bool verboseLogging = false);
    
    // Initialize clock system with external RTC
    // If rtcPtr is nullptr, will use internal RTC instance
    void begin(DS3231* rtcPtr = nullptr);
    
    // Update clock - call this in loop()
    void update();
    
    // Access to components
    ClockTime& getTime() { return clockTime; }
    ClockMotor& getMotor() { return clockMotor; }
    ClockDisplay& getDisplay() { return clockDisplay; }
    AnimationManager& getAnimationManager() { return animationManager; }
    PatternManager& getPatternManager() { return patternManager; }
    
    // Configuration
    void setCenteringAdjustment(int adjustment) { centeringAdjustment = adjustment; }
    void setSlowDelay(int delay) { slowDelay = delay; }
    void enableQuietHours(bool enable, int start, int end, int percent);
    void enableHourChangeAnimation(bool enable) { hourChangeAnimationEnabled = enable; }
    void enableTestAnimationOnStartup(bool enable) { testAnimationOnStartup = enable; }
    void enableMicroCalibration(bool enable, int everyNHours = 4) { 
        microCalibrationEnabled = enable;
        microCalibrationInterval = everyNHours;
    }
    
    // Deprecated: Use getPatternManager() instead
    void setDisplayPattern(ClockDisplay::Pattern pattern) { patternManager.setPattern(pattern); }
    void enableHourlyPatternRotation(bool enable) { patternManager.enableAutoRotation(enable); }
    
    // Status
    bool isCalibrated() const { return calibrated; }
    
private:
    ClockTime clockTime;
    ClockMotor clockMotor;
    ClockDisplay clockDisplay;
    AnimationManager animationManager;
    PatternManager patternManager;
    
    DS3231* externalRTC;
    bool usingExternalRTC;
    
    // Configuration
    int centeringAdjustment;
    int slowDelay;
    int rtcCheckDelay;
    bool verboseLogging;
    bool quietHoursEnabled;
    int quietHoursStart;
    int quietHoursEnd;
    int quietBrightnessPercent;
    uint8_t defaultBrightness;
    bool hourChangeAnimationEnabled;
    bool testAnimationOnStartup;
    bool microCalibrationEnabled;
    int microCalibrationInterval;
    
    // State
    bool calibrated;
    int lastHourForAnimation;
    
    // Helper methods
    void performCalibration();
    void handleMinuteChange();
    void handleHourChange();
    void updateDisplay();
    void updateQuietHoursBrightness();
};

#endif // CLOCK_H
