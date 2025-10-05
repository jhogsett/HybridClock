#ifndef PATTERN_MANAGER_H
#define PATTERN_MANAGER_H

#include <Arduino.h>
#include <ClockDisplay.h>

/**
 * PatternManager - Manages background LED pattern selection and rotation
 * 
 * Provides a simple way to configure which patterns are active and
 * handles automatic pattern rotation.
 */
class PatternManager {
public:
    PatternManager();
    
    // Pattern selection
    void setPattern(ClockDisplay::Pattern pattern) { currentPattern = pattern; }
    ClockDisplay::Pattern getPattern() const { return currentPattern; }
    
    // Enable/disable specific patterns for rotation
    void enablePattern(ClockDisplay::Pattern pattern, bool enabled);
    bool isPatternEnabled(ClockDisplay::Pattern pattern) const;
    
    // Enable/disable all patterns at once
    void enableAllPatterns();
    void disableAllPatterns();
    
    // Random selection from enabled patterns
    void selectRandomPattern();
    
    // Automatic rotation
    void enableAutoRotation(bool enabled) { autoRotationEnabled = enabled; }
    bool isAutoRotationEnabled() const { return autoRotationEnabled; }
    
    // Check if it's time to rotate (call this every hour)
    bool shouldRotate(int currentHour);
    
    // Configuration
    void setRotationInterval(int hours) { rotationIntervalHours = hours; }
    int getRotationInterval() const { return rotationIntervalHours; }
    
#ifdef DEBUG_PATTERN_NAMES
    // Get pattern name for logging (only available if DEBUG_PATTERN_NAMES is defined)
    const char* getPatternName(ClockDisplay::Pattern pattern) const;
#endif
    
private:
    ClockDisplay::Pattern currentPattern;
    bool autoRotationEnabled;
    int rotationIntervalHours;
    int lastRotationHour;
    bool enabledPatterns[ClockDisplay::PATTERN_COUNT];
    
    // Helper to count enabled patterns
    int countEnabledPatterns() const;
};

#endif // PATTERN_MANAGER_H
