#include "clock_display.h"

ClockDisplay::ClockDisplay(Adafruit_NeoPixel& pixelStrip) 
    : pixels(pixelStrip), currentHue(0), lastHour(-1), animationEnabled(true) {
}

void ClockDisplay::begin() {
    pixels.begin();
    pixels.setBrightness(DEFAULT_BRIGHTNESS);
    clearAll();
    pixels.show();
}

void ClockDisplay::setBrightness(uint8_t brightness) {
    pixels.setBrightness(brightness);
    pixels.show();
}

void ClockDisplay::updateTime(int hour, int minute, int second) {
    // Update background pattern first
    updateBackgroundPattern();
    
    // Update hour display
    updateHourDisplay(hour);
    
    // Check for hour change animation
    if (animationEnabled && hour != lastHour && lastHour != -1) {
        showHourChange(hour);
    }
    lastHour = hour;
    
    pixels.show();
}

void ClockDisplay::updateHourDisplay(int hour) {
    int hour12 = (hour % 12);
    
    // According to original spec:
    // - 12:00 hour (0) is shown by LED 0 being lit WHITE  
    // - Remaining hours 1:00-11:00 are shown by LEDs being lit WHITE up to current hour
    
    if (hour12 == 0) {
        // 12 o'clock - only light LED 0
        pixels.setPixelColor(0, pixels.Color(HOUR_COLOR_R, HOUR_COLOR_G, HOUR_COLOR_B));
    } else {
        // 1-11 o'clock - light LEDs 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22 for hours 1-11
        // The even LEDs (2, 4, 6...) represent hours 1, 2, 3... on the 24-LED ring
        for (int i = 1; i <= hour12; i++) {
            int ledIndex = i * 2; // LEDs 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22
            pixels.setPixelColor(ledIndex, pixels.Color(HOUR_COLOR_R, HOUR_COLOR_G, HOUR_COLOR_B));
        }
    }
}

void ClockDisplay::updateBackgroundPattern() {
    // Outer ring (hours) - background hue for non-hour positions
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, 4), 0, HOUR_LEDS);
    
    // Inner ring (minutes) - complementary color
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, 127), HOUR_LEDS, MINUTE_LEDS);
    
    // Advance hue
    currentHue += HUE_STEP;
    currentHue %= MAX_HUE;
}

void ClockDisplay::showCalibrationPattern() {
    pixels.fill(pixels.Color(4, 4, 4));
    pixels.show();
    delay(1000);
    pixels.setPixelColor(0, pixels.Color(128, 128, 128));
    pixels.show();
}

void ClockDisplay::showStartupAnimation() {
    // Spiral startup animation
    clearAll();
    for (int i = 0; i < TOTAL_LEDS; i++) {
        // Use a safer hue calculation to avoid overflow warnings
        uint16_t hue = (uint16_t)((i * 1820L) % 65536L);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, 100));
        pixels.show();
        delay(50);
    }
    delay(500);
    clearAll();
}

void ClockDisplay::showHourChange(int newHour) {
    // Flash the new hour position
    int ledIndex = (newHour % 12 == 1) ? 0 : ((newHour % 12 - 1) * 2);
    
    for (int flash = 0; flash < 3; flash++) {
        pixels.setPixelColor(ledIndex, pixels.Color(255, 255, 255));
        pixels.show();
        delay(200);
        pixels.setPixelColor(ledIndex, pixels.Color(HOUR_COLOR_R, HOUR_COLOR_G, HOUR_COLOR_B));
        pixels.show();
        delay(200);
    }
}

void ClockDisplay::showRainbowMode() {
    pixels.rainbow(currentHue, 1, 255, 255, true);
    currentHue += HUE_STEP * 2;
    currentHue %= MAX_HUE;
    pixels.show();
}

void ClockDisplay::showTemperatureMode(float temperature) {
    uint32_t tempColor = getTemperatureColor(temperature);
    pixels.fill(tempColor);
    pixels.show();
}

void ClockDisplay::showDateMode(int day, int month) {
    clearAll();
    
    // Show month on outer ring (1-12)
    for (int i = 0; i < month && i < 12; i++) {
        int ledIndex = (i == 0) ? 0 : i * 2;
        pixels.setPixelColor(ledIndex, pixels.Color(0, 100, 255));
    }
    
    // Show day on inner ring (approximate)
    int dayLeds = map(day, 1, 31, 1, 12);
    for (int i = 0; i < dayLeds; i++) {
        pixels.setPixelColor(HOUR_LEDS + i, pixels.Color(255, 100, 0));
    }
    
    pixels.show();
}

void ClockDisplay::showAlarmMode() {
    // Pulsing red pattern
    static int brightness = 0;
    static int direction = 5;
    
    brightness += direction;
    if (brightness > 255 || brightness < 0) {
        direction = -direction;
        brightness = constrain(brightness, 0, 255);
    }
    
    pixels.fill(pixels.Color(brightness, 0, 0));
    pixels.show();
}

void ClockDisplay::setAnimationEnabled(bool enabled) {
    animationEnabled = enabled;
}

void ClockDisplay::clearAll() {
    pixels.clear();
}

uint32_t ClockDisplay::getHourColor(int hour) {
    // Different colors for different times of day
    if (hour >= 6 && hour < 12) {
        return pixels.Color(255, 200, 100); // Morning yellow
    } else if (hour >= 12 && hour < 18) {
        return pixels.Color(100, 200, 255); // Afternoon blue
    } else if (hour >= 18 && hour < 22) {
        return pixels.Color(255, 100, 50); // Evening orange
    } else {
        return pixels.Color(100, 100, 200); // Night purple
    }
}

uint32_t ClockDisplay::getTemperatureColor(float temp) {
    // Blue for cold, red for hot
    if (temp < 0) return pixels.Color(0, 100, 255);
    if (temp < 10) return pixels.Color(0, 200, 255);
    if (temp < 20) return pixels.Color(100, 255, 100);
    if (temp < 30) return pixels.Color(255, 200, 0);
    return pixels.Color(255, 50, 0);
}

// Direct pixel access methods for compatibility
void ClockDisplay::setPixelColor(uint16_t n, uint32_t c) {
    pixels.setPixelColor(n, c);
}

uint32_t ClockDisplay::Color(uint8_t r, uint8_t g, uint8_t b) {
    return pixels.Color(r, g, b);
}

void ClockDisplay::show() {
    pixels.show();
}