#include "ClockMotor.h"

ClockMotor::ClockMotor(int stepsPerRev, int pin1, int pin2, int pin3, int pin4, 
                       int sensorPin, int motorSpeed)
    : stepper(stepsPerRev, pin1, pin2, pin3, pin4)
    , sensorPin(sensorPin)
    , stepsPerRevolution(stepsPerRev)
    , firstMotorPin(pin1)
    , handPosition(0.0)
    , motorPowered(false) {
    stepper.setSpeed(motorSpeed);
}

void ClockMotor::begin() {
    pinMode(sensorPin, INPUT_PULLUP);
    
    // Initialize motor pins array
    for (int i = 0; i < 4; i++) {
        motorPins[i] = LOW;
    }
    
    powerOff(); // Start with motor powered off
}

void ClockMotor::powerOn() {
    if (!motorPowered) {
        // Restore motor pin states
        for (int i = 0; i < 4; i++) {
            digitalWrite(firstMotorPin + i, motorPins[i]);
        }
        motorPowered = true;
        delay(100); // Settle time
    }
}

void ClockMotor::powerOff() {
    if (motorPowered || !motorPowered) { // Always save state
        // Save current motor pin states
        for (int i = 0; i < 4; i++) {
            motorPins[i] = digitalRead(firstMotorPin + i);
            digitalWrite(firstMotorPin + i, LOW);
        }
        motorPowered = false;
    }
}

bool ClockMotor::calibrate(int centeringAdjustment, int slowDelay) {
    Serial.println("ClockMotor: Starting calibration...");
    
    handPosition = 0.0;
    int cal_steps1 = 0;
    int cal_steps2 = 0;
    
    // If already on the sensor, roll forward until it's not found
    // FOUND = LOW = 0, NOTFOUND = HIGH = 1
    if (digitalRead(sensorPin) == LOW) {
        for (int i = 0; i < stepsPerRevolution; i++) {
            if (digitalRead(sensorPin) == HIGH)
                break;
            stepper.step(1);
        }
    }
    
    // Roll forward until the sensor is found
    for (int i = 0; i < stepsPerRevolution; i++) {
        if (digitalRead(sensorPin) == LOW)
            break;
        stepper.step(1);
    }
    
    // Roll forward slowly until the sensor is not found and count the steps
    for (int i = 0; i < 2 * stepsPerRevolution; i++) {
        if (digitalRead(sensorPin) == HIGH)
            break;
        stepper.step(1);
        cal_steps1++;
        if (slowDelay > 0) delay(slowDelay);
    }
    
    Serial.print("ClockMotor: Fwd Steps: ");
    Serial.println(cal_steps1);
    
    // Roll back until the sensor is found
    for (int i = 0; i < stepsPerRevolution; i++) {
        if (digitalRead(sensorPin) == LOW)
            break;
        stepper.step(-1);
        if (slowDelay > 0) delay(slowDelay);
    }
    
    // Roll back slowly until the sensor is not found and count the steps
    for (int i = 0; i < 2 * stepsPerRevolution; i++) {
        if (digitalRead(sensorPin) == HIGH)
            break;
        stepper.step(-1);
        cal_steps2++;
        if (slowDelay > 0) delay(slowDelay);
    }
    
    Serial.print("ClockMotor: Bak Steps: ");
    Serial.println(cal_steps2);
    
    int cal_steps = (cal_steps1 + cal_steps2) / 2;
    Serial.print("ClockMotor: Center Steps: ");
    Serial.println(cal_steps);
    
    // Roll forward slowly the average of the counted steps
    for (int i = 0; i < cal_steps; i++) {
        stepper.step(1);
        if (slowDelay > 0) delay(slowDelay);
    }
    
    // Roll back slowly half the number of counted steps plus centering adjustment
    for (int i = 0; i < (cal_steps / 2) + centeringAdjustment; i++) {
        stepper.step(-1);
        if (slowDelay > 0) delay(slowDelay);
    }
    
    handPosition = 0.0;
    Serial.println("ClockMotor: Calibration complete");
    
    return true;
}

void ClockMotor::microCalibrate(int centeringAdjustment, int slowDelay) {
    Serial.println("ClockMotor: Starting micro-calibration...");
    
    int cal_steps1 = 0;
    int cal_steps2 = 0;
    
    // Quick search for magnet
    bool magnetFound = false;
    int searchSteps = 0;
    
    if (digitalRead(sensorPin) == LOW) {
        magnetFound = true;
    } else {
        // Search forward
        for (int i = 0; i < stepsPerRevolution / 12; i++) {
            stepper.step(1);
            searchSteps++;
            if (digitalRead(sensorPin) == LOW) {
                magnetFound = true;
                break;
            }
        }
        
        // Search backward if not found
        if (!magnetFound) {
            for (int i = 0; i < searchSteps; i++) {
                stepper.step(-1);
            }
            searchSteps = 0;
            
            for (int i = 0; i < stepsPerRevolution / 12; i++) {
                stepper.step(-1);
                searchSteps++;
                if (digitalRead(sensorPin) == LOW) {
                    magnetFound = true;
                    break;
                }
            }
        }
    }
    
    // If magnet not found, restore position and exit
    if (!magnetFound) {
        if (searchSteps > 0) {
            for (int i = 0; i < searchSteps; i++) {
                stepper.step(1);
            }
        }
        Serial.println("ClockMotor: Micro-calibration skipped (magnet not found)");
        return;
    }
    
    // Perform micro-calibration algorithm
    for (int i = 0; i < 2 * stepsPerRevolution; i++) {
        if (digitalRead(sensorPin) == HIGH)
            break;
        stepper.step(1);
        cal_steps1++;
        if (slowDelay > 0) delay(slowDelay);
    }
    
    for (int i = 0; i < stepsPerRevolution; i++) {
        stepper.step(-1);
        if (digitalRead(sensorPin) == LOW)
            break;
    }
    
    for (int i = 0; i < 2 * stepsPerRevolution; i++) {
        if (digitalRead(sensorPin) == HIGH)
            break;
        stepper.step(-1);
        cal_steps2++;
        if (slowDelay > 0) delay(slowDelay);
    }
    
    int cal_steps = (cal_steps1 + cal_steps2) / 2;
    
    for (int i = 0; i < cal_steps; i++) {
        stepper.step(1);
        if (slowDelay > 0) delay(slowDelay);
    }
    
    for (int i = 0; i < (cal_steps / 2) + centeringAdjustment; i++) {
        stepper.step(-1);
        if (slowDelay > 0) delay(slowDelay);
    }
    
    handPosition = 0.0;
    Serial.println("ClockMotor: Micro-calibration complete");
}

void ClockMotor::moveToMinute(int minute) {
    float targetPosition = minute * (stepsPerRevolution / 60.0);
    float difference = targetPosition - handPosition;
    
    // Handle wrap-around
    if (difference > stepsPerRevolution / 2) {
        difference -= stepsPerRevolution;
    } else if (difference < -stepsPerRevolution / 2) {
        difference += stepsPerRevolution;
    }
    
    if (abs(difference) > 0.5) {
        moveSteps((int)difference);
    }
}

void ClockMotor::moveSteps(int steps) {
    bool wasPowered = motorPowered;
    
    if (!motorPowered) {
        powerOn();
    }
    
    stepper.step(steps);
    handPosition += steps;
    
    // Normalize position
    if (handPosition >= stepsPerRevolution) handPosition -= stepsPerRevolution;
    if (handPosition < 0) handPosition += stepsPerRevolution;
    
    if (!wasPowered) {
        powerOff();
    }
}
