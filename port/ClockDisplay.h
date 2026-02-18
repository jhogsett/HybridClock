#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/**
 * ClockDisplay - Manages LED display patterns
 * 
 * Provides various display patterns for clock LED rings including
 * complementary colors, breathing, ripples, spirals, waves, and more.
 */
class ClockDisplay {
public:
    enum Pattern {
        DEFAULT_COMPLEMENT = 0,
        BREATHING_RINGS = 1,
        RIPPLE_EFFECT = 2,
        SLOW_SPIRAL = 3,
        GENTLE_WAVES = 4,
        COLOR_DRIFT = 5,
        PATTERN_COUNT = 6
    };
    
    ClockDisplay(int pin, int hourLeds, int minuteLeds, uint8_t brightness = 63);
    
    // Initialize display
    void begin();
    
    // Pattern display methods
    void displayPattern(Pattern pattern);
    void displayDefaultComplement();
    void displayBreathingRings();
    void displayRippleEffect();
    void displaySlowSpiral();
    void displayGentleWaves();
    void displayColorDrift();
    
    // Show hour indicators (overlays on pattern)
    void showHourIndicators(int hour12);
    
    // Special effects
    void showWindmillHourChange(int newHour);
    void showQuarterHourEffect(float progress);
    
    // Display control
    void show() { pixels.show(); }
    void clear() { pixels.clear(); }
    void fill(uint32_t color) { pixels.fill(color); }
    
    // Brightness control
    void setBrightness(uint8_t brightness) { pixels.setBrightness(brightness); }
    uint8_t getBrightness() const { return pixels.getBrightness(); }
    
    // Settings
    void setCurrentPattern(Pattern pattern) { currentPattern = pattern; }
    Pattern getCurrentPattern() const { return currentPattern; }
    
    // Quiet hours support
    void setQuietMode(bool quiet) { quietMode = quiet; }
    bool isQuietMode() const { return quietMode; }
    
    // Access pixel strip
    Adafruit_NeoPixel& getPixels() { return pixels; }
    
private:
    Adafruit_NeoPixel pixels;
    int hourLeds;
    int minuteLeds;
    int totalLeds;
    
    Pattern currentPattern;
    uint32_t currentHue;
    bool quietMode;
    
    // Pattern state
    uint32_t patternStartTime;
    
    // Helper methods
    void adjustBrightnessForQuietMode(uint8_t& outerMin, uint8_t& outerMax,
                                     uint8_t& innerMin, uint8_t& innerMax);
};

#endif // CLOCK_DISPLAY_H
