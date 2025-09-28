#include <Arduino.h>
#include "config.h"
#include <Adafruit_NeoPixel.h>
#include <Stepper.h>
#include <Wire.h>
#include <DS3231-RTC.h>

enum DisplayPattern {
    PATTERN_DEFAULT_COMPLEMENT = 0,
    PATTERN_BREATHING_RINGS = 1,
    PATTERN_RIPPLE_EFFECT = 2,
    PATTERN_SLOW_SPIRAL = 3,
    PATTERN_GENTLE_WAVES = 4,
    PATTERN_COLOR_DRIFT = 5,
    PATTERN_COUNT = 6
};

// For hourly rotation, we'll use the first four patterns initially
#define HOURLY_PATTERN_COUNT 4

struct PatternState {
    DisplayPattern currentPattern;
    uint32_t patternStartTime;
    uint32_t lastPatternChange;
    bool quarterHourActive;
    uint32_t quarterHourStartTime;
    int lastHour; // Track hour changes for pattern switching
} patternState = {PATTERN_DEFAULT_COMPLEMENT, 0, 0, false, 0, -1};

// Hardware instances
DS3231 rtc;
Stepper stepperMotor(STEPS_PER_REVOLUTION, FIRST_MOTOR_PIN, FIRST_MOTOR_PIN+1, FIRST_MOTOR_PIN+2, FIRST_MOTOR_PIN+3);
Adafruit_NeoPixel pixels(TOTAL_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// State variables
int lastMinute = -1;
int lastHour = -1;
int lastSecond = -1;
bool isCalibrated = false;
float handPosition = 0.0;
uint32_t currentHue = 0;

#ifdef ENABLE_PATTERN_SYSTEM
// Pattern management system

// Pattern display functions
void displayDefaultComplement();
void displayBreathingRings();
void displayRippleEffect();
void displaySlowSpiral();
void displayGentleWaves();
void displayColorDrift();
void displayQuarterHourEffect();
void updatePatternSystem();
#endif

// Hour change animation
void showWindmillHourChange(int newHour);

// Quiet micro-calibration function
void performQuietMicroCalibration();

#ifdef ENABLE_QUIET_HOURS
// Quiet hours brightness management
bool isQuietHours(int hour);
void updateQuietHoursBrightness();
#endif

// Motor power management functions
bool motorPins[4] = {LOW, LOW, LOW, LOW};

void pauseMotor() {
    // Save current motor pin states
    for (int i = 0; i < 4; i++) {
        motorPins[i] = digitalRead(FIRST_MOTOR_PIN + i);
        digitalWrite(FIRST_MOTOR_PIN + i, LOW);
    }
}

void resumeMotor() {
    // Restore motor pin states
    for (int i = 0; i < 4; i++) {
        digitalWrite(FIRST_MOTOR_PIN + i, motorPins[i]);
    }
    delay(SETTLE_TIME); // Allow motor to settle
}

void setup() {
    Serial.begin(115200);
    Serial.println("=== Hybrid Clock Starting ===");
    
    Wire.begin();
    pixels.begin();
    pixels.setBrightness(DEFAULT_BRIGHTNESS); // Start with default, will be updated by quiet hours check
    pixels.clear();
    pixels.show();
    
    stepperMotor.setSpeed(MOTOR_SPEED);
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    
    // Calibration
    Serial.println("Calibrating...");
    pixels.fill(pixels.Color(10, 10, 10));
    pixels.show();
    
    // Proper calibration procedure - find sensor center with device adjustment
    handPosition = 0.0;
    int cal_steps1 = 0;
    int cal_steps2 = 0;
    
    // If already on the sensor, roll forward until it's not found
    if (digitalRead(SENSOR_PIN) == FOUND) {
        for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
            if (digitalRead(SENSOR_PIN) == NOTFOUND)
                break;
            stepperMotor.step(1);
        }
    }
    
    // Roll forward until the sensor is found
    for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
        if (digitalRead(SENSOR_PIN) == FOUND)
            break;
        stepperMotor.step(1);
    }
    
    // Roll forward slowly until the sensor is not found and count the steps
    for (int i = 0; i < 2 * STEPS_PER_REVOLUTION; i++) {
        if (digitalRead(SENSOR_PIN) == NOTFOUND)
            break;
        stepperMotor.step(1);
        cal_steps1++;
        delay(SLOW_DELAY);
    }
    
    Serial.print("Fwd Steps: ");
    Serial.println(cal_steps1);
    
    // Roll back until the sensor is found
    for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
        if (digitalRead(SENSOR_PIN) == FOUND)
            break;
        stepperMotor.step(-1);
        delay(SLOW_DELAY);
    }
    
    // Roll back slowly until the sensor is not found and count the steps
    for (int i = 0; i < 2 * STEPS_PER_REVOLUTION; i++) {
        if (digitalRead(SENSOR_PIN) == NOTFOUND)
            break;
        stepperMotor.step(-1);
        cal_steps2++;
        delay(SLOW_DELAY);
    }
    
    Serial.print("Bak Steps: ");
    Serial.println(cal_steps2);
    
    int cal_steps = (cal_steps1 + cal_steps2) / 2;
    Serial.print("Center Steps: ");
    Serial.println(cal_steps);
    
    // Roll forward slowly the average of the counted steps
    for (int i = 0; i < cal_steps; i++) {
        stepperMotor.step(1);
        delay(SLOW_DELAY);
    }
    
    // Roll back slowly half the number of counted steps plus centering adjustment
    for (int i = 0; i < (cal_steps / 2) + CENTERING_ADJUSTMENT; i++) {
        stepperMotor.step(-1);
        delay(SLOW_DELAY);
    }
    
    isCalibrated = true;
    handPosition = 0.0;
    
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green for success
    pixels.show();
    delay(2000);
    
    // Get current time for initial setup
    bool h12Flag = false;
    bool pm = false;
    int initialMinute = rtc.getMinute();
    int initialHour = rtc.getHour(h12Flag, pm);
    lastMinute = initialMinute;
    lastHour = initialHour;
    
    Serial.println("=== Ready ===");
    
    // Set initial brightness based on current time
#ifdef ENABLE_QUIET_HOURS
    updateQuietHoursBrightness();
#endif

#ifdef TEST_HOUR_CHANGE_ON_STARTUP
    // TEST: Trigger hour change animation immediately after calibration
    Serial.println("Testing hour change animation...");
    
    // Simulate hour change by temporarily setting lastHour to different value
    int testHour = (lastHour + 1) % 24;
    
    Serial.print("Testing windmill animation for hour ");
    Serial.println(testHour);
    
    // Use new windmill animation
    showWindmillHourChange(testHour);
    
    Serial.println("Windmill hour change animation test complete");
#endif

    // Move to current minute position
    float targetPosition = initialMinute * (STEPS_PER_REVOLUTION / 60.0);
    float difference = targetPosition - handPosition;
    
    if (abs(difference) > 0.5) {
        resumeMotor(); // Power up motor before movement
        int stepsToMove = (int)difference;
        stepperMotor.step(stepsToMove);
        handPosition += stepsToMove; // Update by actual steps moved
        pauseMotor(); // Power down motor after movement
    }
    
    // Ensure motor is powered down when idle
    pauseMotor();
}

void loop() {
    // Do something only once per second (original timing pattern)
    int second = rtc.getSecond();
    if (second == lastSecond) {
        delay(RTC_CHECK_DELAY);
        return;
    }
    lastSecond = second;
    
    // Get current time
    bool h12Flag = false;
    bool pm = false;
    int hour = rtc.getHour(h12Flag, pm);
    int minute = rtc.getMinute();
    
    // Move hand when minute changes
    if (minute != lastMinute) {
        float targetPosition = minute * (STEPS_PER_REVOLUTION / 60.0);
        float difference = targetPosition - handPosition;
        
        // Handle wrap-around
        if (difference > STEPS_PER_REVOLUTION / 2) {
            difference -= STEPS_PER_REVOLUTION;
        } else if (difference < -STEPS_PER_REVOLUTION / 2) {
            difference += STEPS_PER_REVOLUTION;
        }
        
        if (abs(difference) > 0.5) {
            resumeMotor(); // Power up motor before movement
            int stepsToMove = (int)difference;
            stepperMotor.step(stepsToMove);
            handPosition += stepsToMove; // Update by actual steps moved, not target
            
            // Normalize position
            if (handPosition >= STEPS_PER_REVOLUTION) handPosition -= STEPS_PER_REVOLUTION;
            if (handPosition < 0) handPosition += STEPS_PER_REVOLUTION;
            
            pauseMotor(); // Power down motor after movement
        }
        
        lastMinute = minute;
    }
    
    // Check for hour change and show animation during seconds 58-59 of last minute
    if (minute == 59 && (second == 58 || second == 57)) { // Start at second 57 to ensure 2-second animation completes at top of hour
        int nextHour = (hour + 1) % 24;
        if (nextHour != lastHour) { // Only if we haven't already shown this transition
            Serial.print("[");
            if (hour < 10) Serial.print("0");
            Serial.print(hour);
            Serial.print(":");
            if (minute < 10) Serial.print("0");
            Serial.print(minute);
            Serial.print(":");
            if (second < 10) Serial.print("0");
            Serial.print(second);
            Serial.print("] HOUR TRANSITION (windmill): ");
            Serial.print(lastHour);
            Serial.print(" -> ");
            Serial.println(nextHour);
            
            // Eye-catching rainbow animation for upcoming new hour
            showWindmillHourChange(nextHour);
            
            // Perform micro-calibration every 4 hours (at hours 0, 4, 8, 12, 16, 20)
            if (nextHour % 4 == 0) {
                // Power up motor for micro-calibration
                resumeMotor();
                
                // Perform quiet micro-calibration after hour change animation
                performQuietMicroCalibration();
                
                // Power down motor after micro-calibration
                pauseMotor();
            }
            
            // Check if brightness needs to be adjusted due to quiet hours transition
            // Use nextHour since we're transitioning TO that hour
#ifdef ENABLE_QUIET_HOURS
            bool h12Flag_brightness, pm_brightness;
            int currentHour_brightness = rtc.getHour(h12Flag_brightness, pm_brightness);
            
            uint8_t targetBrightness;
            if (isQuietHours(nextHour)) {  // Use nextHour, not currentHour_brightness
                targetBrightness = (DEFAULT_BRIGHTNESS * QUIET_BRIGHTNESS_PERCENT) / 100;
            } else {
                targetBrightness = DEFAULT_BRIGHTNESS;
            }
            
            // Only change brightness if it's different from current setting
            if (pixels.getBrightness() != targetBrightness) {
                pixels.setBrightness(targetBrightness);
                
                Serial.print("[");
                if (hour < 10) Serial.print("0");
                Serial.print(hour);
                Serial.print(":");
                if (minute < 10) Serial.print("0");
                Serial.print(minute);
                Serial.print(":");
                if (second < 10) Serial.print("0");
                Serial.print(second);
                Serial.print("] BRIGHTNESS CHANGE (windmill): ");
                Serial.print(pixels.getBrightness());
                Serial.print(" -> ");
                Serial.print(targetBrightness);
                Serial.print(" (");
                Serial.print(isQuietHours(nextHour) ? "QUIET" : "ACTIVE");
                Serial.print(" mode for hour ");
                Serial.print(nextHour);
                Serial.println(")");
            }
#endif
            
            lastHour = nextHour; // Update to prevent re-triggering
        }
    }
    
    // Update lastHour when actual hour changes (for normal operation)
    if (hour != lastHour && minute != 59) {
        Serial.print("[");
        if (hour < 10) Serial.print("0");
        Serial.print(hour);
        Serial.print(":");
        if (minute < 10) Serial.print("0");
        Serial.print(minute);
        Serial.print(":");
        if (second < 10) Serial.print("0");
        Serial.print(second);
        Serial.print("] HOUR TRANSITION (normal): ");
        Serial.print(lastHour);
        Serial.print(" -> ");
        Serial.println(hour);
        lastHour = hour;
        
        // Check if brightness needs to be adjusted due to quiet hours transition
#ifdef ENABLE_QUIET_HOURS
        updateQuietHoursBrightness();
#endif
    }
    
    // Update LED display
    pixels.clear();
    
    #ifdef ENABLE_PATTERN_SYSTEM
    // Use pattern system for dynamic displays
    updatePatternSystem();
    #else
    // Original complementary color pattern
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, 8), 0, HOUR_LEDS);
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, 127), HOUR_LEDS, MINUTE_LEDS);
    
    // Advance hue every second (original timing)
    currentHue += HUE_STEP;
    currentHue %= MAX_HUE;
    #endif
    
    // Hour display (override background for hour positions) - original pattern
    int hour12 = hour % 12 + 1; // 12am/pm = 1, 1am/pm = 2, etc.
    
    // Light up LEDs for all hours from 1 through current hour (except 12)
    for (int i = 1; i < 12; i++) {
        if (i < hour12) {
            pixels.setPixelColor(i * 2, pixels.Color(HOUR_COLOR_R, HOUR_COLOR_G, HOUR_COLOR_B));
        }
    }
    
    // Special case for 12 o'clock - only light LED 0
    if (hour12 == 1) {
        pixels.setPixelColor(0, pixels.Color(HOUR_COLOR_R, HOUR_COLOR_G, HOUR_COLOR_B));
    }
    
    pixels.show();
}

// Eye-catching windmill hour change animation
void showWindmillHourChange(int newHour) {
    // DRAMATIC VERSION (commented out - save for alarm/alert use)
    /*
    // Save current pattern state to restore after animation
    pixels.clear();
    
    // Create coordinated windmill rotation effect (about 2 seconds total)
    int rotationSteps = 24; // Number of rotation steps
    int stepDelay = 80;     // Milliseconds per step (24 * 80 = ~1.9 seconds)
    
    for (int step = 0; step < rotationSteps; step++) {
        pixels.clear();
        
        // Outer ring windmill arm (bright white/yellow)
        int outerPos = (step * 2) % HOUR_LEDS; // 2 LEDs per step for faster rotation
        pixels.setPixelColor(outerPos, pixels.Color(255, 255, 200)); // Bright warm white
        pixels.setPixelColor((outerPos + 1) % HOUR_LEDS, pixels.Color(200, 200, 150)); // Trailing glow
        
        // Opposite arm on outer ring for windmill effect
        int outerOpposite = (outerPos + HOUR_LEDS/2) % HOUR_LEDS;
        pixels.setPixelColor(outerOpposite, pixels.Color(255, 255, 200));
        pixels.setPixelColor((outerOpposite + 1) % HOUR_LEDS, pixels.Color(200, 200, 150));
        
        // Inner ring windmill arm (complementary color, offset rotation)
        int innerPos = (step * 3) % MINUTE_LEDS; // 3 LEDs per step for different speed
        pixels.setPixelColor(HOUR_LEDS + innerPos, pixels.Color(255, 150, 50)); // Bright orange
        pixels.setPixelColor(HOUR_LEDS + ((innerPos + 1) % MINUTE_LEDS), pixels.Color(200, 100, 30)); // Trailing glow
        
        // Opposite arm on inner ring
        int innerOpposite = (innerPos + MINUTE_LEDS/2) % MINUTE_LEDS;
        pixels.setPixelColor(HOUR_LEDS + innerOpposite, pixels.Color(255, 150, 50));
        pixels.setPixelColor(HOUR_LEDS + ((innerOpposite + 1) % MINUTE_LEDS), pixels.Color(200, 100, 30));
        
        pixels.show();
        delay(stepDelay);
    }
    */
    
    // SUBTLE VERSION: Rotating rainbow color field (one complete rotation in ~2 seconds)
    int rotationSteps = 48;  // More steps for smooth rotation but faster than before
    int stepDelay = 42;      // 48 * 42 = ~2 seconds (back to reasonable speed)
    
    for (int step = 0; step < rotationSteps; step++) {
        pixels.clear();
        
        // Create a rainbow that wraps around the ring and rotates CLOCKWISE
        uint32_t rotationOffset = (step * 65535L / rotationSteps); // Rotation offset
        
        // Outer ring: rainbow color field that rotates clockwise
        for (int i = 0; i < HOUR_LEDS; i++) {
            // Each LED gets a different hue based on its position around the ring
            uint32_t positionHue = (i * 65535L / HOUR_LEDS); // Rainbow spread across ring
            uint32_t hue = (positionHue - rotationOffset + 65536L) % 65536L; // Rotate clockwise (SUBTRACT offset)
            uint8_t brightness = 35; // Bright for special hour change animation
            pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
        }
        
        // Inner ring: synchronized rainbow rotating clockwise at EXACTLY half speed (visible 1:2 ratio)
        for (int i = 0; i < MINUTE_LEDS; i++) {
            // Each LED gets a different hue based on its position around the ring
            uint32_t positionHue = (i * 65535L / MINUTE_LEDS); // Rainbow spread across ring
            uint32_t hue = (positionHue - (rotationOffset / 2) + 65536L) % 65536L; // Clockwise at half speed (SUBTRACT offset)
            uint8_t brightness = 80; // Brighter for inner ring visibility
            pixels.setPixelColor(HOUR_LEDS + i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
        }
        
        pixels.show();
        delay(stepDelay);
    }
    
    // Animation complete - no blank moment or final flash
}

// Quiet micro-calibration function - duplicates proven calibration algorithm
// Assumes magnet is already near home position, performs extent sensing and centering
// Leaves LEDs completely untouched for silent operation
void performQuietMicroCalibration() {
    // Save current LED state (do not modify LEDs during this process)
    // The display continues showing whatever pattern was active
    
    int cal_steps1 = 0;
    int cal_steps2 = 0;
    
    // SAFETY CHECK: If magnet is not found within a small range, silently exit
    // This prevents unattractive full-sweep behavior if hand is not actually at home
    bool magnetFound = false;
    int searchSteps = 0;
    int totalSearchSteps = 0;
    
    // Quick search for magnet - if not found nearby, bail out quietly
    if (digitalRead(SENSOR_PIN) == FOUND) {
        magnetFound = true;
    } else {
        // Search forward a bit
        for (int i = 0; i < STEPS_PER_REVOLUTION / 12; i++) { // Search only 1/12 revolution (~5 minutes worth)
            stepperMotor.step(1);
            searchSteps++;
            totalSearchSteps++;
            if (digitalRead(SENSOR_PIN) == FOUND) {
                magnetFound = true;
                break;
            }
        }
        
        // If still not found, search backward from starting position
        if (!magnetFound) {
            // Return to starting position first
            for (int i = 0; i < searchSteps; i++) {
                stepperMotor.step(-1);
                totalSearchSteps++;
            }
            searchSteps = 0; // Reset for backward search
            
            // Search backward
            for (int i = 0; i < STEPS_PER_REVOLUTION / 12; i++) {
                stepperMotor.step(-1);
                searchSteps++;
                totalSearchSteps++;
                if (digitalRead(SENSOR_PIN) == FOUND) {
                    magnetFound = true;
                    break;
                }
            }
        }
    }
    
    // If magnet not found nearby, silently exit and restore position
    if (!magnetFound) {
        // Restore original position - return to where we started
        // If we're currently backward from start, go forward to return
        if (searchSteps > 0) { // We ended on a backward search
            for (int i = 0; i < searchSteps; i++) {
                stepperMotor.step(1); // Return forward to starting position
            }
        }
        return; // Silent exit - no calibration performed
    }
    
    // MICRO-CALIBRATION ALGORITHM - starts from current position (assumes on or near magnet)
    // Skip the "exit and re-enter" logic since we want to measure from current position
    
    // Roll forward slowly until the sensor is not found and count the steps
    for (int i = 0; i < 2 * STEPS_PER_REVOLUTION; i++) {
        if (digitalRead(SENSOR_PIN) == NOTFOUND) {
            break;
        }
        stepperMotor.step(1);
        cal_steps1++;
        delay(SLOW_DELAY);
    }
    
    // Roll back until the sensor is found
    for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
        stepperMotor.step(-1);
        if (digitalRead(SENSOR_PIN) == FOUND) {
            break;
        }
    }
    
    // Roll back slowly until the sensor is not found and count the steps
    for (int i = 0; i < 2 * STEPS_PER_REVOLUTION; i++) {
        if (digitalRead(SENSOR_PIN) == NOTFOUND) {
            break;
        }
        stepperMotor.step(-1);
        cal_steps2++;
        delay(SLOW_DELAY);
    }
    
    Serial.print("Bak Steps: ");
    Serial.println(cal_steps2);
    
    int cal_steps = (cal_steps1 + cal_steps2) / 2;
    Serial.print("Center Steps: ");
    Serial.println(cal_steps);
    
    // Roll forward slowly the average of the counted steps
    for (int i = 0; i < cal_steps; i++) {
        stepperMotor.step(1);
        delay(SLOW_DELAY);
    }
    
    // Roll back slowly half the number of counted steps plus centering adjustment
    for (int i = 0; i < (cal_steps / 2) + CENTERING_ADJUSTMENT; i++) {
        stepperMotor.step(-1);
        delay(SLOW_DELAY);
    }
    
    // Update position tracking to reflect we're now at home (0.0)
    handPosition = 0.0;
    
    // LEDs remain completely untouched - display continues as normal
}

#ifdef ENABLE_PATTERN_SYSTEM
// Pattern implementation functions

void displayDefaultComplement() {
    // Original complementary hue pattern
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, 8), 0, HOUR_LEDS);
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, 127), HOUR_LEDS, MINUTE_LEDS);
    
    // Advance hue every second (original timing)
    currentHue += HUE_STEP;
    currentHue %= MAX_HUE;
}

void displayBreathingRings() {
    // Gentle breathing effect - rings pulse in and out of phase
    uint32_t time = millis();
    float breathCycle = sin((time / 1000.0) * 0.5) * 0.5 + 0.5; // 4-second breathing cycle
    float breathCycle2 = sin((time / 1000.0) * 0.3 + 1.5) * 0.5 + 0.5; // Offset breathing
    
    // Adjust brightness based on quiet hours
    uint8_t outerMinBrightness = 4;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 60;
    uint8_t innerMaxBrightness = 127;
    
#ifdef ENABLE_QUIET_HOURS
    bool h12Flag, pm;
    int currentHour = rtc.getHour(h12Flag, pm);
    if (isQuietHours(currentHour)) {
        // In quiet mode, increase minimum brightness to ensure visibility
        outerMinBrightness = 10;  // More than double for visibility
        outerMaxBrightness = 16;  // Keep proportional
        innerMinBrightness = 70;  // Slight increase for inner ring
        innerMaxBrightness = 127; // Keep same max
        
        // Only log once per minute to avoid spam
        static uint32_t lastBreathingLog = 0;
        if (millis() - lastBreathingLog > 60000) {
            lastBreathingLog = millis();
            Serial.println("PATTERN: Breathing Rings - QUIET mode brightness adjustment applied");
        }
    }
#endif
    
    uint8_t brightness1 = outerMinBrightness + (breathCycle * (outerMaxBrightness - outerMinBrightness));
    uint8_t brightness2 = innerMinBrightness + (breathCycle2 * (innerMaxBrightness - innerMinBrightness));
    
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, brightness1), 0, HOUR_LEDS);
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, brightness2), HOUR_LEDS, MINUTE_LEDS);
    
    // Slower hue advancement for breathing pattern
    currentHue += HUE_STEP / 2;
    currentHue %= MAX_HUE;
}

void displayRippleEffect() {
    // Gentle ripple emanating from 12 o'clock position
    uint32_t time = millis();
    float ripplePhase = (time / 200.0); // Ripple speed
    
    // Adjust brightness based on quiet hours
    uint8_t outerMinBrightness = 4;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 50;
    uint8_t innerMaxBrightness = 130;
    
#ifdef ENABLE_QUIET_HOURS
    bool h12Flag, pm;
    int currentHour = rtc.getHour(h12Flag, pm);
    if (isQuietHours(currentHour)) {
        // In quiet mode, increase minimum brightness to ensure visibility
        outerMinBrightness = 10;  // More than double for visibility
        outerMaxBrightness = 16;  // Keep proportional
        innerMinBrightness = 60;  // Slight increase for inner ring
        innerMaxBrightness = 130; // Keep same max
        
        // Only log once per minute to avoid spam
        static uint32_t lastRippleLog = 0;
        if (millis() - lastRippleLog > 60000) {
            lastRippleLog = millis();
            Serial.println("PATTERN: Ripple Effect - QUIET mode brightness adjustment applied");
        }
    }
#endif
    
    for (int i = 0; i < HOUR_LEDS; i++) {
        float distance = min(i, HOUR_LEDS - i); // Distance from position 0 (12 o'clock)
        float ripple = sin(ripplePhase - distance * 0.8) * 0.5 + 0.5;
        uint8_t brightness = outerMinBrightness + (ripple * (outerMaxBrightness - outerMinBrightness));
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(currentHue, 255, brightness));
    }
    
    // Inner ring with complementary color and offset ripple
    for (int i = 0; i < MINUTE_LEDS; i++) {
        float distance = min(i, MINUTE_LEDS - i);
        float ripple = sin(ripplePhase - distance * 1.2 + 1.0) * 0.5 + 0.5;
        uint8_t brightness = innerMinBrightness + (ripple * (innerMaxBrightness - innerMinBrightness));
        pixels.setPixelColor(HOUR_LEDS + i, Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, brightness));
    }
    
    currentHue += HUE_STEP;
    currentHue %= MAX_HUE;
}

void displaySlowSpiral() {
    // Colors slowly spiral around rings at different speeds
    uint32_t time = millis();
    float spiralPhase1 = (time / 3000.0); // Outer ring: 3 second rotation
    float spiralPhase2 = (time / 2000.0); // Inner ring: 2 second rotation (different speed)
    
    // Adjust minimum brightness based on quiet hours
    uint8_t outerMinBrightness = 4;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 60;
    uint8_t innerMaxBrightness = 100;
    
#ifdef ENABLE_QUIET_HOURS
    bool h12Flag, pm;
    int currentHour = rtc.getHour(h12Flag, pm);
    if (isQuietHours(currentHour)) {
        // In quiet mode, increase minimum brightness to ensure visibility
        outerMinBrightness = 10;  // More than double the minimum for visibility
        outerMaxBrightness = 16;  // Keep proportional
        innerMinBrightness = 70;  // Slight increase for inner ring
        innerMaxBrightness = 100; // Keep same max
        
        // Only log once per minute to avoid spam
        static uint32_t lastSpiralLog = 0;
        if (millis() - lastSpiralLog > 60000) {
            lastSpiralLog = millis();
            Serial.println("PATTERN: Slow Spiral - QUIET mode brightness adjustment applied");
        }
    }
#endif
    
    for (int i = 0; i < HOUR_LEDS; i++) {
        float angle = (i * 2.0 * PI / HOUR_LEDS) - spiralPhase1; // SUBTRACT for clockwise rotation
        uint32_t hue = currentHue + (sin(angle) * 16384L); // Hue varies with position
        uint8_t brightness = outerMinBrightness + (cos(angle) * 0.5 + 0.5) * (outerMaxBrightness - outerMinBrightness);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    for (int i = 0; i < MINUTE_LEDS; i++) {
        float angle = (i * 2.0 * PI / MINUTE_LEDS) - spiralPhase2; // SUBTRACT for clockwise rotation
        uint32_t hue = (currentHue + 32768L) + (sin(angle) * 16384L);
        uint8_t brightness = innerMinBrightness + (cos(angle) * 0.4 + 0.4) * (innerMaxBrightness - innerMinBrightness);
        pixels.setPixelColor(HOUR_LEDS + i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    currentHue += HUE_STEP / 3; // Slower hue progression for spiral
    currentHue %= MAX_HUE;
}

void displayGentleWaves() {
    // Soft wave-like movement across the rings
    uint32_t time = millis();
    float wavePhase = (time / 2500.0); // 2.5 second wave cycle
    
    // Adjust minimum brightness based on quiet hours
    uint8_t outerMinBrightness = 6;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 70;
    uint8_t innerMaxBrightness = 127;
    
#ifdef ENABLE_QUIET_HOURS
    bool h12Flag, pm;
    int currentHour = rtc.getHour(h12Flag, pm);
    if (isQuietHours(currentHour)) {
        // In quiet mode, increase minimum brightness to ensure visibility
        outerMinBrightness = 12;  // Double the minimum for visibility
        outerMaxBrightness = 16;  // Keep proportional
        innerMinBrightness = 80;  // Slight increase for inner ring
        innerMaxBrightness = 127; // Keep same max
        
        // Only log once per minute to avoid spam
        static uint32_t lastWavesLog = 0;
        if (millis() - lastWavesLog > 60000) {
            lastWavesLog = millis();
            Serial.println("PATTERN: Gentle Waves - QUIET mode brightness adjustment applied");
        }
    }
#endif
    
    for (int i = 0; i < HOUR_LEDS; i++) {
        float position = (float)i / HOUR_LEDS * 2.0 * PI;
        float wave = sin(position + wavePhase) * 0.5 + 0.5; // Smoother wave between 0.0-1.0
        uint8_t brightness = outerMinBrightness + (wave * (outerMaxBrightness - outerMinBrightness));
        // Smooth the brightness transitions
        brightness = constrain(brightness, outerMinBrightness, outerMaxBrightness);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(currentHue, 255, brightness));
    }
    
    // Inner ring with more apparent complementary color and stronger wave
    for (int i = 0; i < MINUTE_LEDS; i++) {
        float position = (float)i / MINUTE_LEDS * 2.0 * PI;
        float wave = sin(position + wavePhase + PI) * 0.4 + 0.6; // Opposite phase, more variation
        uint8_t brightness = innerMinBrightness + (wave * (innerMaxBrightness - innerMinBrightness));
        brightness = constrain(brightness, innerMinBrightness, innerMaxBrightness);
        pixels.setPixelColor(HOUR_LEDS + i, Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, brightness));
    }
    
    // Very slow hue progression for gentle effect
    currentHue += HUE_STEP / 4;
    currentHue %= MAX_HUE;
}

void displayColorDrift() {
    // Slow, smooth color transitions with subtle brightness variations
    uint32_t time = millis();
    float driftPhase = (time / 8000.0); // 8 second color drift cycle
    
    // Adjust minimum brightness based on quiet hours
    uint8_t outerMinBrightness = 6;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 100;
    uint8_t innerMaxBrightness = 127;
    
#ifdef ENABLE_QUIET_HOURS
    bool h12Flag, pm;
    int currentHour = rtc.getHour(h12Flag, pm);
    if (isQuietHours(currentHour)) {
        // In quiet mode, increase minimum brightness to ensure visibility
        outerMinBrightness = 12;  // Double the minimum for visibility
        outerMaxBrightness = 16;  // Keep proportional
        innerMinBrightness = 110; // Slight increase for inner ring
        innerMaxBrightness = 127; // Keep same max
        
        // Only log once per minute to avoid spam
        static uint32_t lastDriftLog = 0;
        if (millis() - lastDriftLog > 60000) {
            lastDriftLog = millis();
            Serial.println("PATTERN: Color Drift - QUIET mode brightness adjustment applied");
        }
    }
#endif
    
    // Create smooth color drift across the outer ring
    for (int i = 0; i < HOUR_LEDS; i++) {
        float position = (float)i / HOUR_LEDS;
        uint32_t hue = currentHue + (sin(driftPhase + position * PI) * 8192L); // Gentle hue variation
        uint8_t brightness = outerMinBrightness + (sin(driftPhase * 2 + position * 4) * 0.5 + 0.5) * (outerMaxBrightness - outerMinBrightness);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    // Inner ring with different drift pattern
    for (int i = 0; i < MINUTE_LEDS; i++) {
        float position = (float)i / MINUTE_LEDS;
        uint32_t hue = (currentHue + 32768L) + (cos(driftPhase * 0.7 + position * PI * 1.5) * 12288L);
        uint8_t brightness = innerMinBrightness + (cos(driftPhase * 1.5 + position * 3) * 0.4 + 0.4) * (innerMaxBrightness - innerMinBrightness);
        pixels.setPixelColor(HOUR_LEDS + i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    // Extremely slow hue progression for smooth transitions
    currentHue += HUE_STEP / 6;
    currentHue %= MAX_HUE;
}

void displayQuarterHourEffect() {
    // Brief celebration effect for quarter hours
    uint32_t elapsed = millis() - patternState.quarterHourStartTime;
    float progress = elapsed / (QUARTER_HOUR_EFFECT_DURATION * 1000.0);
    
    if (progress >= 1.0) {
        patternState.quarterHourActive = false;
        return;
    }
    
    // Gentle bloom effect that spreads outward
    float bloomIntensity = sin(progress * PI) * 200; // Bright bloom for special quarter-hour celebration
    
    for (int i = 0; i < TOTAL_LEDS; i++) {
        uint8_t brightness = 20 + bloomIntensity; // Can be bright - this is a special animation
        brightness = constrain(brightness, 20, 220);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(currentHue, 180, brightness));
    }
    
    Serial.print("Quarter hour effect: ");
    Serial.print(progress * 100);
    Serial.println("%");
}

void updatePatternSystem() {
    uint32_t currentTime = millis();
    
    // Check for hour changes for pattern switching
    #ifdef ENABLE_HOURLY_PATTERN_ROTATION
    bool h12Flag, pm;
    int currentHour = rtc.getHour(h12Flag, pm);
    if (currentHour != patternState.lastHour) {
        if (patternState.lastHour != -1) { // Skip first initialization
            // Select random pattern from available hourly patterns
            randomSeed(analogRead(RANDOM_SEED_PIN) + currentHour); // Seed with hour for some consistency
            patternState.currentPattern = (DisplayPattern)random(HOURLY_PATTERN_COUNT);
            
            Serial.print("Hour changed to ");
            Serial.print(currentHour);
            Serial.print(", switching to pattern: ");
            Serial.println(patternState.currentPattern);
        }
        patternState.lastHour = currentHour;
    }
    #endif
    
    // Check for quarter-hour effects
    #ifdef ENABLE_QUARTER_HOUR_EFFECTS
    int minute = rtc.getMinute();
    if ((minute == 15 || minute == 30 || minute == 45) && !patternState.quarterHourActive) {
        if (currentTime - patternState.quarterHourStartTime > 60000) { // Don't repeat within 1 minute
            patternState.quarterHourActive = true;
            patternState.quarterHourStartTime = currentTime;
            Serial.print("Quarter hour effect started at minute: ");
            Serial.println(minute);
        }
    }
    #endif
    
    // Handle active quarter-hour effect
    if (patternState.quarterHourActive) {
        displayQuarterHourEffect();
        return; // Override other patterns during quarter-hour effect
    }
    
    // Original pattern rotation system (disabled when hourly is enabled)
    #if defined(ENABLE_PATTERN_ROTATION) && !defined(ENABLE_HOURLY_PATTERN_ROTATION)
    if (currentTime - patternState.lastPatternChange > (PATTERN_CHANGE_INTERVAL * 1000)) {
        patternState.currentPattern = (DisplayPattern)((patternState.currentPattern + 1) % PATTERN_COUNT);
        patternState.lastPatternChange = currentTime;
        patternState.patternStartTime = currentTime;
        
        Serial.print("Switching to pattern: ");
        Serial.println(patternState.currentPattern);
    }
    #endif
    
    // Force specific test patterns
    #ifdef TEST_BREATHING_RINGS
    patternState.currentPattern = PATTERN_BREATHING_RINGS;
    #elif defined(TEST_RIPPLE_EFFECT)
    patternState.currentPattern = PATTERN_RIPPLE_EFFECT;
    #elif defined(TEST_SLOW_SPIRAL)
    patternState.currentPattern = PATTERN_SLOW_SPIRAL;
    #elif defined(TEST_GENTLE_WAVES)
    patternState.currentPattern = PATTERN_GENTLE_WAVES;
    #elif defined(TEST_COLOR_DRIFT)
    patternState.currentPattern = PATTERN_COLOR_DRIFT;
    #endif
    
    // Execute current pattern
    switch (patternState.currentPattern) {
        case PATTERN_BREATHING_RINGS:
            displayBreathingRings();
            break;
        case PATTERN_RIPPLE_EFFECT:
            displayRippleEffect();
            break;
        case PATTERN_SLOW_SPIRAL:
            displaySlowSpiral();
            break;
        case PATTERN_GENTLE_WAVES:
            displayGentleWaves();
            break;
        case PATTERN_COLOR_DRIFT:
            displayColorDrift();
            break;
        default:
            displayDefaultComplement();
            break;
    }
    
    // Report current pattern every minute for debugging
    static uint32_t lastPatternReport = 0;
    if (currentTime - lastPatternReport > 60000) {
        lastPatternReport = currentTime;
        
        // Get current time for timestamp
        bool h12Flag, pm;
        int currentHour = rtc.getHour(h12Flag, pm);
        int currentMinute = rtc.getMinute();
        
        Serial.print("[");
        if (currentHour < 10) Serial.print("0");
        Serial.print(currentHour);
        Serial.print(":");
        if (currentMinute < 10) Serial.print("0");
        Serial.print(currentMinute);
        Serial.print("] CURRENT PATTERN: ");
        Serial.print(patternState.currentPattern);
        Serial.print(" (");
        switch (patternState.currentPattern) {
            case PATTERN_BREATHING_RINGS: Serial.print("Breathing Rings"); break;
            case PATTERN_RIPPLE_EFFECT: Serial.print("Ripple Effect"); break;
            case PATTERN_SLOW_SPIRAL: Serial.print("Slow Spiral"); break;
            case PATTERN_GENTLE_WAVES: Serial.print("Gentle Waves"); break;
            case PATTERN_COLOR_DRIFT: Serial.print("Color Drift"); break;
            default: Serial.print("Default Complement"); break;
        }
        Serial.print("), Global brightness: ");
        Serial.println(pixels.getBrightness());
    }
}
#endif

#ifdef ENABLE_QUIET_HOURS
// Check if current hour is within quiet hours
bool isQuietHours(int hour) {
    // Handle wraparound case where quiet hours cross midnight
    if (QUIET_HOURS_START > QUIET_HOURS_END) {
        // Quiet hours cross midnight (e.g., 22:00 to 6:00)
        return (hour >= QUIET_HOURS_START || hour < QUIET_HOURS_END);
    } else {
        // Quiet hours within same day (e.g., 1:00 to 5:00)
        return (hour >= QUIET_HOURS_START && hour < QUIET_HOURS_END);
    }
}

// Update NeoPixel brightness based on current time
void updateQuietHoursBrightness() {
    bool h12Flag, pm;
    int currentHour = rtc.getHour(h12Flag, pm);
    
    uint8_t targetBrightness;
    if (isQuietHours(currentHour)) {
        targetBrightness = (DEFAULT_BRIGHTNESS * QUIET_BRIGHTNESS_PERCENT) / 100;
    } else {
        targetBrightness = DEFAULT_BRIGHTNESS;
    }
    
    // Only change brightness if it's different from current setting
    if (pixels.getBrightness() != targetBrightness) {
        pixels.setBrightness(targetBrightness);
        
        // Get current time for timestamp
        bool h12Flag, pm;
        int timestampHour = rtc.getHour(h12Flag, pm);
        int timestampMinute = rtc.getMinute();
        
        Serial.print("[");
        if (timestampHour < 10) Serial.print("0");
        Serial.print(timestampHour);
        Serial.print(":");
        if (timestampMinute < 10) Serial.print("0");
        Serial.print(timestampMinute);
        Serial.print("] BRIGHTNESS CHANGE: ");
        Serial.print(pixels.getBrightness());
        Serial.print(" -> ");
        Serial.print(targetBrightness);
        Serial.print(" (");
        Serial.print(isQuietHours(currentHour) ? "QUIET" : "ACTIVE");
        Serial.print(" mode at ");
        Serial.print(currentHour);
        Serial.println(":00)");
    } else {
        Serial.print("[");
        bool h12Flag, pm;
        int timestampHour = rtc.getHour(h12Flag, pm);
        int timestampMinute = rtc.getMinute();
        if (timestampHour < 10) Serial.print("0");
        Serial.print(timestampHour);
        Serial.print(":");
        if (timestampMinute < 10) Serial.print("0");
        Serial.print(timestampMinute);
        Serial.print("] BRIGHTNESS CHECK: Already at ");
        Serial.print(targetBrightness);
        Serial.print(" (");
        Serial.print(isQuietHours(currentHour) ? "QUIET" : "ACTIVE");
        Serial.print(" mode at ");
        Serial.print(currentHour);
        Serial.println(":00)");
    }
}
#endif