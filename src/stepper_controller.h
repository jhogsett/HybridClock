#ifndef STEPPER_CONTROLLER_H
#define STEPPER_CONTROLLER_H

#include <Stepper.h>
#include "config.h"

class StepperController {
private:
    Stepper& stepper;
    int sensorPin;
    float currentPosition;
    int centeringAdjustment;
    bool isCalibrated;
    
public:
    StepperController(Stepper& stepperMotor, int hallSensorPin, int centering);
    
    void begin();
    bool calibrate();
    void moveToPosition(float targetPosition);
    void moveToMinute(int minute);
    void moveToSecond(int minute, int second);
    void homePosition();
    
    // Advanced movements
    void sweepTest();
    void smoothMoveTo(float targetPosition, int steps = 10);
    bool verifyCalibration();
    
    float getCurrentPosition() const { return currentPosition; }
    bool getCalibrationStatus() const { return isCalibrated; }
    
private:
    void pauseMotor();
    void resumeMotor();
    int findSensorEdge(int direction);
    void stepMotor(int steps);
    float normalizePosition(float position);
    bool motorPins[4];
};

#endif