#include "ClockDisplay.h"

ClockDisplay::ClockDisplay(int pin, int hourLeds, int minuteLeds, uint8_t brightness)
    : pixels(hourLeds + minuteLeds, pin, NEO_GRB + NEO_KHZ800)
    , hourLeds(hourLeds)
    , minuteLeds(minuteLeds)
    , totalLeds(hourLeds + minuteLeds)
    , currentPattern(DEFAULT_COMPLEMENT)
    , currentHue(0)
    , quietMode(false)
    , patternStartTime(0) {
    pixels.setBrightness(brightness);
}

void ClockDisplay::begin() {
    pixels.begin();
    pixels.clear();
    pixels.show();
    patternStartTime = millis();
}

void ClockDisplay::adjustBrightnessForQuietMode(uint8_t& outerMin, uint8_t& outerMax,
                                                uint8_t& innerMin, uint8_t& innerMax) {
    if (quietMode) {
        // Double minimum brightness for visibility in quiet mode
        outerMin = min(255, outerMin * 2);
        outerMax = min(255, outerMax * 2);
        innerMin = min(255, innerMin + 10);
    }
}

void ClockDisplay::displayPattern(Pattern pattern) {
    switch (pattern) {
        case BREATHING_RINGS:
            displayBreathingRings();
            break;
        case RIPPLE_EFFECT:
            displayRippleEffect();
            break;
        case SLOW_SPIRAL:
            displaySlowSpiral();
            break;
        case GENTLE_WAVES:
            displayGentleWaves();
            break;
        case COLOR_DRIFT:
            displayColorDrift();
            break;
        default:
            displayDefaultComplement();
            break;
    }
}

void ClockDisplay::displayDefaultComplement() {
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, 8), 0, hourLeds);
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, 127), hourLeds, minuteLeds);
    
    currentHue += 1024; // HUE_STEP
    currentHue %= (5 * 65536); // MAX_HUE
}

void ClockDisplay::displayBreathingRings() {
    uint32_t time = millis();
    float breathCycle = sin((time / 1000.0) * 0.5) * 0.5 + 0.5;
    float breathCycle2 = sin((time / 1000.0) * 0.3 + 1.5) * 0.5 + 0.5;
    
    uint8_t outerMinBrightness = 4;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 60;
    uint8_t innerMaxBrightness = 127;
    
    adjustBrightnessForQuietMode(outerMinBrightness, outerMaxBrightness,
                                innerMinBrightness, innerMaxBrightness);
    
    uint8_t brightness1 = outerMinBrightness + (breathCycle * (outerMaxBrightness - outerMinBrightness));
    uint8_t brightness2 = innerMinBrightness + (breathCycle2 * (innerMaxBrightness - innerMinBrightness));
    
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue, 255, brightness1), 0, hourLeds);
    pixels.fill(Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, brightness2), hourLeds, minuteLeds);
    
    currentHue += 512; // HUE_STEP / 2
    currentHue %= (5 * 65536);
}

void ClockDisplay::displayRippleEffect() {
    uint32_t time = millis();
    float ripplePhase = (time / 200.0);
    
    uint8_t outerMinBrightness = 4;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 50;
    uint8_t innerMaxBrightness = 130;
    
    adjustBrightnessForQuietMode(outerMinBrightness, outerMaxBrightness,
                                innerMinBrightness, innerMaxBrightness);
    
    for (int i = 0; i < hourLeds; i++) {
        float distance = min(i, hourLeds - i);
        float ripple = sin(ripplePhase - distance * 0.8) * 0.5 + 0.5;
        uint8_t brightness = outerMinBrightness + (ripple * (outerMaxBrightness - outerMinBrightness));
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(currentHue, 255, brightness));
    }
    
    for (int i = 0; i < minuteLeds; i++) {
        float distance = min(i, minuteLeds - i);
        float ripple = sin(ripplePhase - distance * 1.2 + 1.0) * 0.5 + 0.5;
        uint8_t brightness = innerMinBrightness + (ripple * (innerMaxBrightness - innerMinBrightness));
        pixels.setPixelColor(hourLeds + i, Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, brightness));
    }
    
    currentHue += 1024;
    currentHue %= (5 * 65536);
}

void ClockDisplay::displaySlowSpiral() {
    uint32_t time = millis();
    float spiralPhase1 = (time / 3000.0);
    float spiralPhase2 = (time / 2000.0);
    
    uint8_t outerMinBrightness = 4;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 60;
    uint8_t innerMaxBrightness = 100;
    
    adjustBrightnessForQuietMode(outerMinBrightness, outerMaxBrightness,
                                innerMinBrightness, innerMaxBrightness);
    
    for (int i = 0; i < hourLeds; i++) {
        float angle = (i * 2.0 * PI / hourLeds) - spiralPhase1;
        uint32_t hue = currentHue + (sin(angle) * 16384L);
        uint8_t brightness = outerMinBrightness + (cos(angle) * 0.5 + 0.5) * (outerMaxBrightness - outerMinBrightness);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    for (int i = 0; i < minuteLeds; i++) {
        float angle = (i * 2.0 * PI / minuteLeds) - spiralPhase2;
        uint32_t hue = (currentHue + 32768L) + (sin(angle) * 16384L);
        uint8_t brightness = innerMinBrightness + (cos(angle) * 0.4 + 0.4) * (innerMaxBrightness - innerMinBrightness);
        pixels.setPixelColor(hourLeds + i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    currentHue += 341; // HUE_STEP / 3
    currentHue %= (5 * 65536);
}

void ClockDisplay::displayGentleWaves() {
    uint32_t time = millis();
    float wavePhase = (time / 2500.0);
    
    uint8_t outerMinBrightness = 6;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 70;
    uint8_t innerMaxBrightness = 127;
    
    adjustBrightnessForQuietMode(outerMinBrightness, outerMaxBrightness,
                                innerMinBrightness, innerMaxBrightness);
    
    for (int i = 0; i < hourLeds; i++) {
        float position = (float)i / hourLeds * 2.0 * PI;
        float wave = sin(position + wavePhase) * 0.5 + 0.5;
        uint8_t brightness = outerMinBrightness + (wave * (outerMaxBrightness - outerMinBrightness));
        brightness = constrain(brightness, outerMinBrightness, outerMaxBrightness);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(currentHue, 255, brightness));
    }
    
    for (int i = 0; i < minuteLeds; i++) {
        float position = (float)i / minuteLeds * 2.0 * PI;
        float wave = sin(position + wavePhase + PI) * 0.4 + 0.6;
        uint8_t brightness = innerMinBrightness + (wave * (innerMaxBrightness - innerMinBrightness));
        brightness = constrain(brightness, innerMinBrightness, innerMaxBrightness);
        pixels.setPixelColor(hourLeds + i, Adafruit_NeoPixel::ColorHSV(currentHue + 32768L, 255, brightness));
    }
    
    currentHue += 256; // HUE_STEP / 4
    currentHue %= (5 * 65536);
}

void ClockDisplay::displayColorDrift() {
    uint32_t time = millis();
    float driftPhase = (time / 8000.0);
    
    uint8_t outerMinBrightness = 6;
    uint8_t outerMaxBrightness = 8;
    uint8_t innerMinBrightness = 100;
    uint8_t innerMaxBrightness = 127;
    
    adjustBrightnessForQuietMode(outerMinBrightness, outerMaxBrightness,
                                innerMinBrightness, innerMaxBrightness);
    
    for (int i = 0; i < hourLeds; i++) {
        float position = (float)i / hourLeds;
        uint32_t hue = currentHue + (sin(driftPhase + position * PI) * 8192L);
        uint8_t brightness = outerMinBrightness + (sin(driftPhase * 2 + position * 4) * 0.5 + 0.5) * (outerMaxBrightness - outerMinBrightness);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    for (int i = 0; i < minuteLeds; i++) {
        float position = (float)i / minuteLeds;
        uint32_t hue = (currentHue + 32768L) + (cos(driftPhase * 0.7 + position * PI * 1.5) * 12288L);
        uint8_t brightness = innerMinBrightness + (cos(driftPhase * 1.5 + position * 3) * 0.4 + 0.4) * (innerMaxBrightness - innerMinBrightness);
        pixels.setPixelColor(hourLeds + i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
    }
    
    currentHue += 171; // HUE_STEP / 6
    currentHue %= (5 * 65536);
}

void ClockDisplay::showHourIndicators(int hour12) {
    // Light up LEDs for all hours from 1 through current hour (except 12)
    for (int i = 1; i < 12; i++) {
        if (i < hour12) {
            pixels.setPixelColor(i * 2, pixels.Color(128, 128, 128));
        }
    }
    
    // Special case for 12 o'clock
    if (hour12 == 1) {
        pixels.setPixelColor(0, pixels.Color(128, 128, 128));
    }
}

void ClockDisplay::showWindmillHourChange(int newHour) {
    int rotationSteps = 48;
    int stepDelay = 42;
    
    for (int step = 0; step < rotationSteps; step++) {
        pixels.clear();
        
        uint32_t rotationOffset = (step * 65535L / rotationSteps);
        
        // Outer ring: rainbow color field rotating clockwise
        for (int i = 0; i < hourLeds; i++) {
            uint32_t positionHue = (i * 65535L / hourLeds);
            uint32_t hue = (positionHue - rotationOffset + 65536L) % 65536L;
            uint8_t brightness = 35;
            pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
        }
        
        // Inner ring: synchronized rainbow at half speed
        for (int i = 0; i < minuteLeds; i++) {
            uint32_t positionHue = (i * 65535L / minuteLeds);
            uint32_t hue = (positionHue - (rotationOffset / 2) + 65536L) % 65536L;
            uint8_t brightness = 80;
            pixels.setPixelColor(hourLeds + i, Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
        }
        
        pixels.show();
        delay(stepDelay);
    }
}

void ClockDisplay::showQuarterHourEffect(float progress) {
    if (progress >= 1.0) return;
    
    float bloomIntensity = sin(progress * PI) * 200;
    
    for (int i = 0; i < totalLeds; i++) {
        uint8_t brightness = 20 + bloomIntensity;
        brightness = constrain(brightness, 20, 220);
        pixels.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(currentHue, 180, brightness));
    }
}
