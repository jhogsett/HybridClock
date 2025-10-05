#ifndef CLOCK_TIME_H
#define CLOCK_TIME_H

#include <Arduino.h>
#include <DS3231-RTC.h>

/**
 * ClockTime - Manages RTC time reading and tracking
 * 
 * Provides simplified interface for reading time from DS3231 RTC
 * and tracking time changes (second, minute, hour).
 */
class ClockTime {
public:
    ClockTime();
    
    // Initialize RTC
    void begin();
    
    // Update time from RTC - returns true if second changed
    bool update();
    
    // Get current time values
    int getHour() const { return currentHour; }
    int getMinute() const { return currentMinute; }
    int getSecond() const { return currentSecond; }
    int getHour12() const { return (currentHour % 12) + 1; }
    
    // Check for changes since last update
    bool hasSecondChanged() const { return secondChanged; }
    bool hasMinuteChanged() const { return minuteChanged; }
    bool hasHourChanged() const { return hourChanged; }
    
    // Get previous values (useful for detecting transitions)
    int getLastHour() const { return lastHour; }
    int getLastMinute() const { return lastMinute; }
    int getLastSecond() const { return lastSecond; }
    
private:
    DS3231 rtc;
    
    int currentHour;
    int currentMinute;
    int currentSecond;
    
    int lastHour;
    int lastMinute;
    int lastSecond;
    
    bool secondChanged;
    bool minuteChanged;
    bool hourChanged;
};

#endif // CLOCK_TIME_H
