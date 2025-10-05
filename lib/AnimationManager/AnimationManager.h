#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <Arduino.h>

// Forward declaration
class ClockDisplay;

/**
 * AnimationManager - Manages hour change animations
 * 
 * Provides a simple way to select and play different hour change animations.
 * Animations are identified by enum values and can be easily enabled/disabled.
 */
class AnimationManager {
public:
    enum AnimationType {
        WINDMILL = 0,           // Original rotating rainbow windmill
        // Future animations can be added here
        ANIMATION_COUNT = 1
    };
    
    AnimationManager();
    
    // Play the current animation
    void playHourChangeAnimation(ClockDisplay& display, int newHour);
    
    // Animation selection
    void setAnimation(AnimationType type) { currentAnimation = type; }
    AnimationType getAnimation() const { return currentAnimation; }
    
    // Enable/disable specific animations for random selection
    void enableAnimation(AnimationType type, bool enabled);
    bool isAnimationEnabled(AnimationType type) const;
    
    // Random selection from enabled animations
    void selectRandomAnimation();
    
    // Configuration
    void enableRandomSelection(bool enabled) { randomSelectionEnabled = enabled; }
    bool isRandomSelectionEnabled() const { return randomSelectionEnabled; }
    
private:
    AnimationType currentAnimation;
    bool randomSelectionEnabled;
    bool enabledAnimations[ANIMATION_COUNT];
    
    // Animation implementations
    void playWindmill(ClockDisplay& display, int newHour);
    // Future: Additional animation methods can be added here
    
    // Helper to count enabled animations
    int countEnabledAnimations() const;
};

#endif // ANIMATION_MANAGER_H
