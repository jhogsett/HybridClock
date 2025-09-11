#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#include <Adafruit_NeoPixel.h>
#include "config.h"

class ClockDisplay {
private:
    Adafruit_NeoPixel& pixels;
    uint32_t currentHue;
    int lastHour;
    bool animationEnabled;
    
public:
    ClockDisplay(Adafruit_NeoPixel& pixelStrip);
    
    void begin();
    void setBrightness(uint8_t brightness);
    void updateTime(int hour, int minute, int second);
    void showCalibrationPattern();
    void showStartupAnimation();
    void showHourChange(int newHour);
    void setAnimationEnabled(bool enabled);
    
    // Advanced display modes
    void showRainbowMode();
    void showTemperatureMode(float temperature);
    void showDateMode(int day, int month);
    void showAlarmMode();
    
    // Direct pixel access for compatibility
    void setPixelColor(uint16_t n, uint32_t c);
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
    void show();
    
private:
    void updateHourDisplay(int hour);
    void updateBackgroundPattern();
    void clearAll();
    uint32_t getHourColor(int hour);
    uint32_t getTemperatureColor(float temp);
};

#endif