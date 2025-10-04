#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <DS3231-RTC.h>
#include <ClockTime.h>
#include <ClockMotor.h>
#include <ClockDisplay.h>
#include <ClockConfig.h>

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
    Clock(int stepsPerRev = STEPS_PER_REVOLUTION,
          int motorPin1 = FIRST_MOTOR_PIN,
          int motorPin2 = FIRST_MOTOR_PIN + 1,
          int motorPin3 = FIRST_MOTOR_PIN + 2,
          int motorPin4 = FIRST_MOTOR_PIN + 3,
          int sensorPin = SENSOR_PIN,
          int neopixelPin = NEOPIXEL_PIN,
          int hourLeds = HOUR_LEDS,
          int minuteLeds = MINUTE_LEDS,
          uint8_t brightness = DEFAULT_BRIGHTNESS,
          int motorSpeed = MOTOR_SPEED);
    
    // Initialize clock system with external RTC
    // If rtcPtr is nullptr, will use internal RTC instance
    void begin(DS3231* rtcPtr = nullptr);
    
    // Update clock - call this in loop()
    void update();
    
    // Access to components
    ClockTime& getTime() { return clockTime; }
    ClockMotor& getMotor() { return clockMotor; }
    ClockDisplay& getDisplay() { return clockDisplay; }
    
    // Configuration
    void setCenteringAdjustment(int adjustment) { centeringAdjustment = adjustment; }
    void setSlowDelay(int delay) { slowDelay = delay; }
    void enableQuietHours(bool enable, int start = QUIET_HOURS_START, int end = QUIET_HOURS_END, int percent = QUIET_BRIGHTNESS_PERCENT);
    void enableHourChangeAnimation(bool enable) { hourChangeAnimationEnabled = enable; }
    void enableMicroCalibration(bool enable, int everyNHours = 4) { 
        microCalibrationEnabled = enable;
        microCalibrationInterval = everyNHours;
    }
    void setDisplayPattern(ClockDisplay::Pattern pattern) { displayPattern = pattern; }
    void enableHourlyPatternRotation(bool enable) { hourlyPatternRotation = enable; }
    
    // Status
    bool isCalibrated() const { return calibrated; }
    
private:
    ClockTime clockTime;
    ClockMotor clockMotor;
    ClockDisplay clockDisplay;
    
    DS3231* externalRTC;
    bool usingExternalRTC;
    
    // Configuration
    int centeringAdjustment;
    int slowDelay;
    bool quietHoursEnabled;
    int quietHoursStart;
    int quietHoursEnd;
    int quietBrightnessPercent;
    uint8_t defaultBrightness;
    bool hourChangeAnimationEnabled;
    bool microCalibrationEnabled;
    int microCalibrationInterval;
    bool hourlyPatternRotation;
    ClockDisplay::Pattern displayPattern;
    
    // State
    bool calibrated;
    int lastHourForAnimation;
    int lastHourForPattern;
    
    // Helper methods
    void performCalibration();
    void handleMinuteChange();
    void handleHourChange();
    void updateDisplay();
    void updateQuietHoursBrightness();
};

#endif // CLOCK_H
