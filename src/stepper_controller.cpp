#include "stepper_controller.h"
#include <Arduino.h>
#include <math.h>

StepperController::StepperController(Stepper& stepperMotor, int hallSensorPin, int centering)
    : stepper(stepperMotor), sensorPin(hallSensorPin), currentPosition(0.0), 
      centeringAdjustment(centering), isCalibrated(false) {
    for (int i = 0; i < 4; i++) {
        motorPins[i] = LOW;
    }
}

void StepperController::begin() {
    pinMode(sensorPin, INPUT_PULLUP);
    stepper.setSpeed(MOTOR_SPEED);
}

bool StepperController::calibrate() {
    Serial.println("Starting calibration...");
    
    currentPosition = 0.0;
    isCalibrated = false;
    
    // If already on sensor, move away first
    if (digitalRead(sensorPin) == FOUND) {
        for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
            if (digitalRead(sensorPin) == NOTFOUND) break;
            stepMotor(FORE);
        }
    }
    
    // Find sensor activation zone
    int edgeSteps1 = findSensorEdge(FORE);
    if (edgeSteps1 < 0) return false;
    
    int edgeSteps2 = findSensorEdge(BACK);
    if (edgeSteps2 < 0) return false;
    
    Serial.print("Forward edge steps: ");
    Serial.println(edgeSteps1);
    Serial.print("Backward edge steps: ");
    Serial.println(edgeSteps2);
    
    // Calculate center of sensor zone
    int centerSteps = (edgeSteps1 + edgeSteps2) / 2;
    
    // Move to sensor center
    stepMotor(centerSteps);
    
    // Apply centering adjustment
    stepMotor(-(centerSteps / 2) - centeringAdjustment);
    
    currentPosition = 0.0;
    isCalibrated = true;
    
    Serial.println("Calibration complete");
    return true;
}

int StepperController::findSensorEdge(int direction) {
    // First, move until sensor is found
    for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
        if (digitalRead(sensorPin) == FOUND) break;
        stepMotor(direction);
        if (i == STEPS_PER_REVOLUTION - 1) return -1; // Timeout
    }
    
    // Then count steps until sensor is lost
    int steps = 0;
    for (int i = 0; i < STEPS_PER_REVOLUTION * 2; i++) {
        if (digitalRead(sensorPin) == NOTFOUND) break;
        stepMotor(direction);
        steps++;
        delay(SLOW_DELAY);
        if (i == STEPS_PER_REVOLUTION * 2 - 1) return -1; // Timeout
    }
    
    return steps;
}

void StepperController::moveToPosition(float targetPosition) {
    if (!isCalibrated) {
        Serial.println("Error: Stepper not calibrated");
        return;
    }
    
    resumeMotor();
    
    targetPosition = normalizePosition(targetPosition);
    float difference = targetPosition - currentPosition;
    
    // Choose shortest path (considering wrap-around)
    if (difference > STEPS_PER_REVOLUTION / 2) {
        difference -= STEPS_PER_REVOLUTION;
    } else if (difference < -STEPS_PER_REVOLUTION / 2) {
        difference += STEPS_PER_REVOLUTION;
    }
    
    if (abs(difference) > 0.5) { // Only move if significant difference
        Serial.print("Moving ");
        Serial.print((int)difference);
        Serial.println(" steps");
        stepMotor((int)difference);
        currentPosition = normalizePosition(currentPosition + difference);
        Serial.print("New position: ");
        Serial.println(currentPosition);
    } else {
        Serial.println("No movement needed (difference too small)");
    }
    
    pauseMotor();
}

void StepperController::moveToMinute(int minute) {
    float position = minute * (STEPS_PER_REVOLUTION / 60.0);
    Serial.print("Moving to minute ");
    Serial.print(minute);
    Serial.print(" (position ");
    Serial.print(position);
    Serial.print(" from current ");
    Serial.print(currentPosition);
    Serial.println(")");
    moveToPosition(position);
}

void StepperController::moveToSecond(int minute, int second) {
    float totalSeconds = minute * 60 + second;
    float position = totalSeconds * (STEPS_PER_REVOLUTION / 3600.0);
    moveToPosition(position);
}

void StepperController::homePosition() {
    moveToPosition(0.0);
}

void StepperController::smoothMoveTo(float targetPosition, int steps) {
    if (!isCalibrated) return;
    
    float startPosition = currentPosition;
    float totalDistance = targetPosition - startPosition;
    
    // Handle wrap-around
    if (totalDistance > STEPS_PER_REVOLUTION / 2) {
        totalDistance -= STEPS_PER_REVOLUTION;
    } else if (totalDistance < -STEPS_PER_REVOLUTION / 2) {
        totalDistance += STEPS_PER_REVOLUTION;
    }
    
    resumeMotor();
    
    for (int i = 1; i <= steps; i++) {
        float progress = (float)i / steps;
        float intermediatePosition = startPosition + (totalDistance * progress);
        float stepDifference = intermediatePosition - currentPosition;
        
        if (abs(stepDifference) >= 0.5) {
            stepMotor((int)stepDifference);
            currentPosition = normalizePosition(currentPosition + stepDifference);
        }
        
        delay(50); // Smooth movement delay
    }
    
    pauseMotor();
}

void StepperController::sweepTest() {
    Serial.println("Starting sweep test...");
    
    resumeMotor();
    
    // Sweep 180 degrees forward
    for (int i = 0; i < STEPS_PER_REVOLUTION / 2; i += 10) {
        stepMotor(10);
        delay(100);
    }
    
    delay(1000);
    
    // Sweep back to start
    for (int i = 0; i < STEPS_PER_REVOLUTION / 2; i += 10) {
        stepMotor(-10);
        delay(100);
    }
    
    pauseMotor();
    Serial.println("Sweep test complete");
}

bool StepperController::verifyCalibration() {
    if (!isCalibrated) return false;
    
    homePosition();
    
    bool sensorActive = (digitalRead(sensorPin) == FOUND);
    Serial.print("Calibration verification - Sensor active: ");
    Serial.println(sensorActive ? "YES" : "NO");
    
    return sensorActive;
}

void StepperController::pauseMotor() {
    for (int i = 0; i < 4; i++) {
        motorPins[i] = digitalRead(FIRST_MOTOR_PIN + i);
        digitalWrite(FIRST_MOTOR_PIN + i, LOW);
    }
}

void StepperController::resumeMotor() {
    for (int i = 0; i < 4; i++) {
        digitalWrite(FIRST_MOTOR_PIN + i, motorPins[i]);
    }
    delay(SETTLE_TIME);
}

void StepperController::stepMotor(int steps) {
    stepper.step(steps);
}

float StepperController::normalizePosition(float position) {
    while (position >= STEPS_PER_REVOLUTION) {
        position -= STEPS_PER_REVOLUTION;
    }
    while (position < 0) {
        position += STEPS_PER_REVOLUTION;
    }
    return position;
}