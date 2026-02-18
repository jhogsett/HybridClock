#include "ClockTime.h"

ClockTime::ClockTime() 
    : currentHour(-1), currentMinute(-1), currentSecond(-1)
    , lastHour(-1), lastMinute(-1), lastSecond(-1)
    , secondChanged(false), minuteChanged(false), hourChanged(false) {
}

void ClockTime::begin() {
    Wire.begin();
    // DS3231 library auto-detects on I2C bus
}

bool ClockTime::update() {
    // Read current time
    bool h12Flag = false;
    bool pm = false;
    
    int newSecond = rtc.getSecond();
    int newMinute = rtc.getMinute();
    int newHour = rtc.getHour(h12Flag, pm);
    
    // Check for changes
    secondChanged = (newSecond != lastSecond);
    minuteChanged = (newMinute != lastMinute);
    hourChanged = (newHour != lastHour);
    
    // Update tracking
    if (secondChanged) {
        lastSecond = currentSecond;
        currentSecond = newSecond;
    }
    
    if (minuteChanged) {
        lastMinute = currentMinute;
        currentMinute = newMinute;
    }
    
    if (hourChanged) {
        lastHour = currentHour;
        currentHour = newHour;
    }
    
    return secondChanged;
}
