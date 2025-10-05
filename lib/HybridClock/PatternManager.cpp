#include "PatternManager.h"

PatternManager::PatternManager()
    : currentPattern(ClockDisplay::DEFAULT_COMPLEMENT)
    , autoRotationEnabled(false)
    , rotationIntervalHours(1)
    , lastRotationHour(-1) {
    // Enable first 4 patterns by default (original behavior)
    for (int i = 0; i < ClockDisplay::PATTERN_COUNT; i++) {
        enabledPatterns[i] = (i < 4);  // Enable first 4 patterns
    }
}

void PatternManager::enablePattern(ClockDisplay::Pattern pattern, bool enabled) {
    if (pattern >= 0 && pattern < ClockDisplay::PATTERN_COUNT) {
        enabledPatterns[pattern] = enabled;
    }
}

bool PatternManager::isPatternEnabled(ClockDisplay::Pattern pattern) const {
    if (pattern >= 0 && pattern < ClockDisplay::PATTERN_COUNT) {
        return enabledPatterns[pattern];
    }
    return false;
}

void PatternManager::enableAllPatterns() {
    for (int i = 0; i < ClockDisplay::PATTERN_COUNT; i++) {
        enabledPatterns[i] = true;
    }
}

void PatternManager::disableAllPatterns() {
    for (int i = 0; i < ClockDisplay::PATTERN_COUNT; i++) {
        enabledPatterns[i] = false;
    }
}

void PatternManager::selectRandomPattern() {
    int enabledCount = countEnabledPatterns();
    if (enabledCount == 0) {
        currentPattern = ClockDisplay::DEFAULT_COMPLEMENT; // Fallback
        return;
    }
    
    // Pick a random enabled pattern
    int selection = random(enabledCount);
    int count = 0;
    
    for (int i = 0; i < ClockDisplay::PATTERN_COUNT; i++) {
        if (enabledPatterns[i]) {
            if (count == selection) {
                currentPattern = (ClockDisplay::Pattern)i;
                return;
            }
            count++;
        }
    }
}

bool PatternManager::shouldRotate(int currentHour) {
    if (!autoRotationEnabled) {
        return false;
    }
    
    // Check if we've crossed an hour boundary
    if (lastRotationHour == -1) {
        lastRotationHour = currentHour;
        return false;
    }
    
    // Calculate if enough hours have passed
    int hoursPassed = (currentHour - lastRotationHour + 24) % 24;
    
    if (hoursPassed >= rotationIntervalHours) {
        lastRotationHour = currentHour;
        return true;
    }
    
    return false;
}

int PatternManager::countEnabledPatterns() const {
    int count = 0;
    for (int i = 0; i < ClockDisplay::PATTERN_COUNT; i++) {
        if (enabledPatterns[i]) count++;
    }
    return count;
}

#ifdef DEBUG_PATTERN_NAMES
const char* PatternManager::getPatternName(ClockDisplay::Pattern pattern) const {
    switch (pattern) {
        case ClockDisplay::DEFAULT_COMPLEMENT:
            return "Default Complement";
        case ClockDisplay::BREATHING_RINGS:
            return "Breathing Rings";
        case ClockDisplay::RIPPLE_EFFECT:
            return "Ripple Effect";
        case ClockDisplay::SLOW_SPIRAL:
            return "Slow Spiral";
        case ClockDisplay::GENTLE_WAVES:
            return "Gentle Waves";
        case ClockDisplay::COLOR_DRIFT:
            return "Color Drift";
        default:
            return "Unknown";
    }
}
#endif
