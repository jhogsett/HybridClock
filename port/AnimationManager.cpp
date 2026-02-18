#include "AnimationManager.h"
#include <ClockDisplay.h>

AnimationManager::AnimationManager()
    : currentAnimation(WINDMILL)
    , randomSelectionEnabled(false) {
    // Enable all animations by default
    for (int i = 0; i < ANIMATION_COUNT; i++) {
        enabledAnimations[i] = true;
    }
}

#ifdef HYBRIDCLOCK_ENABLE_ANIMATIONS

void AnimationManager::playHourChangeAnimation(ClockDisplay& display, int newHour) {
    // For now, just play windmill - the only existing animation
    // In the future, this can be extended to support multiple animations
    playWindmill(display, newHour);
}

void AnimationManager::enableAnimation(AnimationType type, bool enabled) {
    if (type >= 0 && type < ANIMATION_COUNT) {
        enabledAnimations[type] = enabled;
    }
}

bool AnimationManager::isAnimationEnabled(AnimationType type) const {
    if (type >= 0 && type < ANIMATION_COUNT) {
        return enabledAnimations[type];
    }
    return false;
}

void AnimationManager::selectRandomAnimation() {
    int enabledCount = countEnabledAnimations();
    if (enabledCount == 0) {
        currentAnimation = WINDMILL; // Fallback
        return;
    }
    
    // Pick a random enabled animation
    int selection = random(enabledCount);
    int count = 0;
    
    for (int i = 0; i < ANIMATION_COUNT; i++) {
        if (enabledAnimations[i]) {
            if (count == selection) {
                currentAnimation = (AnimationType)i;
                return;
            }
            count++;
        }
    }
}

int AnimationManager::countEnabledAnimations() const {
    int count = 0;
    for (int i = 0; i < ANIMATION_COUNT; i++) {
        if (enabledAnimations[i]) count++;
    }
    return count;
}

// ============================================================================
// ANIMATION IMPLEMENTATIONS
// ============================================================================

void AnimationManager::playWindmill(ClockDisplay& display, int newHour) {
    // Original windmill animation - rotating rainbow
    int rotationSteps = 48;
    int stepDelay = 42;
    
    Adafruit_NeoPixel& pixels = display.getPixels();
    int hourLeds = 24;  // TODO: Get from display
    int minuteLeds = 12;
    
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

// Future: Additional animations can be added here
// For now, only Windmill (moved from ClockDisplay) is implemented

#else // HYBRIDCLOCK_ENABLE_ANIMATIONS not defined - provide stub implementations

void AnimationManager::playHourChangeAnimation(ClockDisplay& display, int newHour) {
    // Stub - do nothing when animations are disabled
}

void AnimationManager::enableAnimation(AnimationType type, bool enabled) {
    // Stub
}

bool AnimationManager::isAnimationEnabled(AnimationType type) const {
    return false;
}

void AnimationManager::selectRandomAnimation() {
    // Stub
}

int AnimationManager::countEnabledAnimations() const {
    return 0;
}

#endif // HYBRIDCLOCK_ENABLE_ANIMATIONS
