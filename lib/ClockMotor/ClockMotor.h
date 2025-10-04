#ifndef CLOCK_MOTOR_H
#define CLOCK_MOTOR_H

#include <Arduino.h>
#include <Stepper.h>

/**
 * ClockMotor - Manages stepper motor control and calibration
 * 
 * Handles stepper motor movement, position tracking, calibration,
 * and power management for clock hand positioning.
 */
class ClockMotor {
public:
    ClockMotor(int stepsPerRev, int pin1, int pin2, int pin3, int pin4, 
               int sensorPin, int motorSpeed = 11);
    
    // Initialize motor and pins
    void begin();
    
    // Calibration
    bool calibrate(int centeringAdjustment = 0, int slowDelay = 0);
    void microCalibrate(int centeringAdjustment = 0, int slowDelay = 0);
    
    // Movement
    void moveToMinute(int minute);
    void moveSteps(int steps);
    
    // Power management
    void powerOn();
    void powerOff();
    bool isPoweredOn() const { return motorPowered; }
    
    // Position tracking
    float getPosition() const { return handPosition; }
    void setPosition(float pos) { handPosition = pos; }
    
    // Motor settings
    void setSpeed(int speed) { stepper.setSpeed(speed); }
    int getStepsPerRevolution() const { return stepsPerRevolution; }
    
private:
    Stepper stepper;
    int sensorPin;
    int stepsPerRevolution;
    int firstMotorPin;
    
    float handPosition;
    bool motorPowered;
    bool motorPins[4];
};

#endif // CLOCK_MOTOR_H
