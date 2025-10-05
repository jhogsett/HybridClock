# Manager Classes - Animation & Pattern Management

## Overview
The Clock library now includes two lightweight manager classes to make it easy to configure and customize animations and display patterns.

## AnimationManager

Manages hour change animations with easy selection and random rotation.

### Available Animations:
1. **WINDMILL** - Original rotating rainbow (default)
2. **PULSE** - Pulsing rings that expand from center
3. **SWEEP** - Color sweep like a clock hand
4. **SPARKLE** - Random twinkling celebration

### Basic Usage:

```cpp
// Get reference to animation manager
AnimationManager& animMgr = hybridClock.getAnimationManager();

// Set a specific animation
animMgr.setAnimation(AnimationManager::PULSE);

// Enable random selection from enabled animations
animMgr.enableRandomSelection(true);

// Disable specific animations
animMgr.enableAnimation(AnimationManager::SPARKLE, false);
```

### Configuration Examples:

**Always use Windmill (default):**
```cpp
// No configuration needed - Windmill is default
```

**Random animation each hour:**
```cpp
hybridClock.getAnimationManager().enableRandomSelection(true);
```

**Only use Pulse and Sweep:**
```cpp
AnimationManager& animMgr = hybridClock.getAnimationManager();
animMgr.enableAnimation(AnimationManager::WINDMILL, false);
animMgr.enableAnimation(AnimationManager::SPARKLE, false);
animMgr.enableRandomSelection(true);
```

## PatternManager

Manages background LED display patterns with automatic rotation.

### Available Patterns:
1. **DEFAULT_COMPLEMENT** - Complementary hue rings (original)
2. **BREATHING_RINGS** - Gentle pulsing effect
3. **RIPPLE_EFFECT** - Ripples from 12 o'clock
4. **SLOW_SPIRAL** - Spiraling colors at different speeds
5. **GENTLE_WAVES** - Wave-like movement
6. **COLOR_DRIFT** - Smooth color transitions

### Basic Usage:

```cpp
// Get reference to pattern manager
PatternManager& patMgr = hybridClock.getPatternManager();

// Set a specific pattern
patMgr.setPattern(ClockDisplay::BREATHING_RINGS);

// Enable automatic rotation
patMgr.enableAutoRotation(true);
patMgr.setRotationInterval(1);  // Rotate every N hours

// Enable/disable specific patterns
patMgr.enablePattern(ClockDisplay::COLOR_DRIFT, true);
```

### Configuration Examples:

**Fixed pattern (no rotation):**
```cpp
hybridClock.setDisplayPattern(ClockDisplay::BREATHING_RINGS);
```

**Rotate through first 4 patterns every hour (default):**
```cpp
PatternManager& patMgr = hybridClock.getPatternManager();
patMgr.enableAutoRotation(true);
patMgr.setRotationInterval(1);
// First 4 patterns enabled by default
```

**Rotate through ALL patterns every 2 hours:**
```cpp
PatternManager& patMgr = hybridClock.getPatternManager();
patMgr.enableAllPatterns();
patMgr.enableAutoRotation(true);
patMgr.setRotationInterval(2);
```

**Use only specific patterns:**
```cpp
PatternManager& patMgr = hybridClock.getPatternManager();
patMgr.disableAllPatterns();
patMgr.enablePattern(ClockDisplay::BREATHING_RINGS, true);
patMgr.enablePattern(ClockDisplay::GENTLE_WAVES, true);
patMgr.enablePattern(ClockDisplay::COLOR_DRIFT, true);
patMgr.enableAutoRotation(true);
patMgr.setRotationInterval(1);
```

## Complete Example (main.cpp)

```cpp
void setup() {
    Serial.begin(115200);
    Serial.println(F("=== Hybrid Clock Starting ==="));
    
    Wire.begin();
    
    // Basic clock configuration
    hybridClock.setCenteringAdjustment(CENTERING_ADJUSTMENT);
    hybridClock.enableMicroCalibration(true, 4);
    
    // ========================================
    // ANIMATION MANAGER CONFIGURATION
    // ========================================
    AnimationManager& animMgr = hybridClock.getAnimationManager();
    
    // Option 1: Use a specific animation
    animMgr.setAnimation(AnimationManager::WINDMILL);
    
    // Option 2: Random animation selection
    // animMgr.enableRandomSelection(true);
    
    // Option 3: Limit which animations can be used
    // animMgr.enableAnimation(AnimationManager::SPARKLE, false);
    // animMgr.enableRandomSelection(true);
    
    // ========================================
    // PATTERN MANAGER CONFIGURATION
    // ========================================
    PatternManager& patMgr = hybridClock.getPatternManager();
    
    // Option 1: Fixed pattern
    // patMgr.setPattern(ClockDisplay::BREATHING_RINGS);
    
    // Option 2: Hourly rotation through first 4 patterns (default)
    patMgr.enableAutoRotation(true);
    patMgr.setRotationInterval(1);
    
    // Option 3: Rotate through ALL patterns
    // patMgr.enableAllPatterns();
    // patMgr.enableAutoRotation(true);
    
    // Option 4: Custom pattern pool
    // patMgr.disableAllPatterns();
    // patMgr.enablePattern(ClockDisplay::BREATHING_RINGS, true);
    // patMgr.enablePattern(ClockDisplay::GENTLE_WAVES, true);
    // patMgr.enableAutoRotation(true);
    
    // Initialize clock
    hybridClock.begin(&rtc);
    
    Serial.println(F("=== Setup Complete ==="));
}
```

## Benefits

### Code Organization
- ✅ **Cleaner Clock class** - animation/pattern logic separated
- ✅ **No duplicate code** - centralized management
- ✅ **Easy to extend** - add new animations/patterns in one place

### Flexibility
- ✅ **Runtime configuration** - change settings without recompiling
- ✅ **Granular control** - enable/disable specific animations/patterns
- ✅ **Random selection** - automatic variety with configurable pool

### Memory Efficiency
- ✅ **Minimal overhead** - ~120 bytes RAM, ~2KB Flash
- ✅ **No memory leaks** - static allocation only
- ✅ **Efficient** - simple array-based selection

## Advanced Features

### Pattern Names
Get human-readable pattern names for logging:
```cpp
const char* name = patMgr.getPatternName(patMgr.getPattern());
Serial.println(name);  // "Breathing Rings"
```

### Check Status
```cpp
// Check if random selection is enabled
if (animMgr.isRandomSelectionEnabled()) {
    // ...
}

// Check if a specific pattern is enabled
if (patMgr.isPatternEnabled(ClockDisplay::COLOR_DRIFT)) {
    // ...
}
```

### Dynamic Changes
```cpp
void loop() {
    hybridClock.update();
    
    // You can change animations/patterns at any time
    if (someCondition) {
        hybridClock.getAnimationManager().setAnimation(AnimationManager::PULSE);
    }
}
```

## Migration from Old API

**Old way:**
```cpp
hybridClock.setDisplayPattern(ClockDisplay::BREATHING_RINGS);
hybridClock.enableHourlyPatternRotation(true);
```

**New way (backward compatible):**
```cpp
// Still works - internally uses PatternManager
hybridClock.setDisplayPattern(ClockDisplay::BREATHING_RINGS);
hybridClock.enableHourlyPatternRotation(true);

// Or use managers directly for more control
hybridClock.getPatternManager().setPattern(ClockDisplay::BREATHING_RINGS);
hybridClock.getPatternManager().enableAutoRotation(true);
```

## Memory Impact

- **AnimationManager**: ~60 bytes RAM, ~1.5KB Flash
- **PatternManager**: ~60 bytes RAM, ~500 bytes Flash
- **Total**: ~120 bytes RAM, ~2KB Flash

The Flash increase is mostly from the 3 new animations (Pulse, Sweep, Sparkle). The original Windmill animation code was moved from ClockDisplay to AnimationManager.

---

*Document created: October 2025*
*Compatible with Clock library v2.0+*
