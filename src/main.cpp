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
    PATTERN_COUNT = 4
};

// For hourly rotation, we'll use just the first two patterns initially
#define HOURLY_PATTERN_COUNT 2

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
void displayQuarterHourEffect();
void updatePatternSystem();
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
    pixels.setBrightness(DEFAULT_BRIGHTNESS);
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
    
    // Get current time for initial display
    bool h12Flag = false;
    bool pm = false;
    int initialMinute = rtc.getMinute();
    int initialHour = rtc.getHour(h12Flag, pm);
    lastMinute = initialMinute;
    lastHour = initialHour;
    
    // Show proper colorful display immediately after calibration
    pixels.clear();
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, 8), 0, HOUR_LEDS);
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, 127), HOUR_LEDS, MINUTE_LEDS);
    
    // Show hour display
    int hour12 = initialHour % 12 + 1;
    for (int i = 1; i < 12; i++) {
        if (i < hour12) {
            pixels.setPixelColor(i * 2, pixels.Color(255, 255, 255));
        }
    }
    if (hour12 == 1) {
        pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    }
    pixels.show();
    
    // Now move to current minute position (with nice display already showing)
    float targetPosition = initialMinute * (STEPS_PER_REVOLUTION / 60.0);
    float difference = targetPosition - handPosition;
    
    if (abs(difference) > 0.5) {
        resumeMotor(); // Power up motor before movement
        stepperMotor.step((int)difference);
        handPosition = targetPosition;
        pauseMotor(); // Power down motor after movement
    }
    
    // Ensure motor is powered down when idle
    pauseMotor();
    
    Serial.println("=== Ready ===");
    
#ifdef TEST_HOUR_CHANGE_ON_STARTUP
    // TEST: Trigger hour change animation after 3 seconds
    delay(3000);
    Serial.println("Testing hour change animation...");
    
    // Simulate hour change by temporarily setting lastHour to different value
    int testHour = (lastHour + 1) % 24;
    
    // Brief flash animation for new hour (same code as in loop)
    int hour12 = testHour % 12;
    int hourLED = (hour12 == 0) ? 0 : hour12 * 2;
    
    Serial.print("Flashing LED ");
    Serial.print(hourLED);
    Serial.print(" for hour ");
    Serial.println(testHour);
    
    for (int i = 0; i < 3; i++) {
        pixels.setPixelColor(hourLED, pixels.Color(255, 255, 255));
        pixels.show();
        delay(200);
        pixels.setPixelColor(hourLED, pixels.Color(0, 0, 0));
        pixels.show();
        delay(200);
    }
    
    Serial.println("Hour change animation test complete");
#endif
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
            stepperMotor.step((int)difference);
            handPosition = targetPosition;
            if (handPosition >= STEPS_PER_REVOLUTION) handPosition -= STEPS_PER_REVOLUTION;
            if (handPosition < 0) handPosition += STEPS_PER_REVOLUTION;
            pauseMotor(); // Power down motor after movement
        }
        
        lastMinute = minute;
    }
    
    // Check for hour change and show animation
    if (hour != lastHour && lastHour != -1) {
        // Brief flash animation for new hour
        int hour12 = hour % 12;
        int hourLED = (hour12 == 0) ? 0 : hour12 * 2;
        
        for (int i = 0; i < 3; i++) {
            pixels.setPixelColor(hourLED, pixels.Color(255, 255, 255));
            pixels.show();
            delay(200);
            pixels.setPixelColor(hourLED, pixels.Color(0, 0, 0));
            pixels.show();
            delay(200);
        }
        lastHour = hour;
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
            pixels.setPixelColor(i * 2, pixels.Color(255, 255, 255));
        }
    }
    
    // Special case for 12 o'clock - only light LED 0
    if (hour12 == 1) {
        pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    }
    
    pixels.show();
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
    
    uint8_t brightness1 = 5 + (breathCycle * 45); // 5-50 brightness range (outer ring)
    uint8_t brightness2 = 30 + (breathCycle2 * 90); // 30-120 brightness range (inner ring)
    
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
    
    for (int i = 0; i < HOUR_LEDS; i++) {
        float distance = min(i, HOUR_LEDS - i); // Distance from position 0 (12 o'clock)
        float ripple = sin(ripplePhase - distance * 0.8) * 0.5 + 0.5;
        uint8_t brightness = 10 + (ripple * 40);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(currentHue, 255, brightness));
    }
    
    // Inner ring with complementary color and offset ripple
    for (int i = 0; i < MINUTE_LEDS; i++) {
        float distance = min(i, MINUTE_LEDS - i);
        float ripple = sin(ripplePhase - distance * 1.2 + 1.0) * 0.5 + 0.5;
        uint8_t brightness = 50 + (ripple * 80);
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
    
    for (int i = 0; i < HOUR_LEDS; i++) {
        float angle = (i * 2.0 * PI / HOUR_LEDS) + spiralPhase1;
        uint32_t hue = currentHue + (sin(angle) * 16384L); // Hue varies with position
        uint8_t brightness = 20 + (cos(angle) * 30 + 30);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    for (int i = 0; i < MINUTE_LEDS; i++) {
        float angle = (i * 2.0 * PI / MINUTE_LEDS) + spiralPhase2;
        uint32_t hue = (currentHue + 32768L) + (sin(angle) * 16384L);
        uint8_t brightness = 60 + (cos(angle) * 40 + 40);
        pixels.setPixelColor(HOUR_LEDS + i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    currentHue += HUE_STEP / 3; // Slower hue progression for spiral
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
    float bloomIntensity = sin(progress * PI) * 200; // Peak at middle of effect
    
    for (int i = 0; i < TOTAL_LEDS; i++) {
        uint8_t brightness = 20 + bloomIntensity;
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
        default:
            displayDefaultComplement();
            break;
    }
}
#endif