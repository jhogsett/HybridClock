#include "config.h"
#include <Adafruit_NeoPixel.h>
#include <Stepper.h>
#include <Wire.h>
#include <DS3231-RTC.h>

// Hardware instances
DS3231 rtc;
Stepper stepperMotor(STEPS_PER_REVOLUTION, FIRST_MOTOR_PIN, FIRST_MOTOR_PIN+1, FIRST_MOTOR_PIN+2, FIRST_MOTOR_PIN+3);
Adafruit_NeoPixel pixels(TOTAL_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// State variables
int lastMinute = -1;
int lastHour = -1;
bool isCalibrated = false;
float handPosition = 0.0;
uint32_t currentHue = 0;

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
    
    // Move to current minute position
    bool h12Flag = false;
    bool pm = false;
    int initialMinute = rtc.getMinute();
    float targetPosition = initialMinute * (STEPS_PER_REVOLUTION / 60.0);
    float difference = targetPosition - handPosition;
    
    if (abs(difference) > 0.5) {
        stepperMotor.step((int)difference);
        handPosition = targetPosition;
    }
    lastMinute = initialMinute;
    lastHour = rtc.getHour(h12Flag, pm);
    
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
    // Get current time
    bool h12Flag = false;
    bool pm = false;
    int hour = rtc.getHour(h12Flag, pm);
    int minute = rtc.getMinute();
    int second = rtc.getSecond();
    
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
            stepperMotor.step((int)difference);
            handPosition = targetPosition;
            if (handPosition >= STEPS_PER_REVOLUTION) handPosition -= STEPS_PER_REVOLUTION;
            if (handPosition < 0) handPosition += STEPS_PER_REVOLUTION;
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
    
    // Dynamic background pattern (complementary colors)
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, 8), 0, HOUR_LEDS);
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, 127), HOUR_LEDS, MINUTE_LEDS);
    
    // Advance hue slowly
    currentHue += 256; // Slower than original for subtlety
    currentHue %= 65536;
    
    // Hour display (override background for hour positions)
    int hour12 = hour % 12;
    if (hour12 == 0) {
        // 12 o'clock - LED 0
        pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    } else {
        // 1-11 o'clock - current hour LED
        int hourLED = hour12 * 2; // LEDs 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22
        pixels.setPixelColor(hourLED, pixels.Color(255, 255, 255));
    }
    
    pixels.show();
    
    delay(1000); // Update every second
}