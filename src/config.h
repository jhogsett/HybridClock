#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define SENSOR_PIN 2
#define LED_PIN 3
#define NEOPIXEL_PIN 6
#define FIRST_MOTOR_PIN 14

// Device-specific calibration
// #define BLACK_DEVICE 
#define WHITE_DEVICE 

#if defined(BLACK_DEVICE)
  #define CENTERING_ADJUSTMENT 9
#elif defined(WHITE_DEVICE)
  #define CENTERING_ADJUSTMENT 6
#else
  #define CENTERING_ADJUSTMENT 0
#endif

// Motor Configuration
#define STEPS_PER_REVOLUTION 2048
#define MOTOR_SPEED 11
#define SLOW_DELAY 0
#define SETTLE_TIME 100

// LED Configuration
#define HOUR_LEDS 24
#define MINUTE_LEDS 12
#define TOTAL_LEDS (HOUR_LEDS + MINUTE_LEDS)
#define DEFAULT_BRIGHTNESS 63

// Timing Configuration
#define RTC_CHECK_DELAY 50
#define RESTART_WAIT 3000L
#define RESET_COUNT 5
#define CALIBRATION_DISPLAY_TIME 3000

// Color Configuration
#define MAX_HUE (5*65536)
#define HUE_STEP 1024
#define HOUR_COLOR_R 128
#define HOUR_COLOR_G 128
#define HOUR_COLOR_B 128

// Random Seed Configuration
#define RANDOM_SEED_PIN A7
#define RANDOM_SEED_SAMPLES 16

// Test Configuration
// #define TEST_HOUR_CHANGE_ON_STARTUP  // Comment out to disable startup test

// Display Pattern Configuration
#define ENABLE_PATTERN_SYSTEM        // Master enable for advanced patterns
#define ENABLE_HOURLY_PATTERN_ROTATION // Change patterns randomly every hour
// #define TEST_BREATHING_RINGS        // Test breathing pattern immediately
// #define TEST_RIPPLE_EFFECT          // Test ripple pattern immediately  
// #define TEST_SLOW_SPIRAL            // Test spiral pattern immediately
// #define ENABLE_QUARTER_HOUR_EFFECTS // Enable 15/30/45 minute celebrations
// #define ENABLE_PATTERN_ROTATION     // Cycle through patterns every few minutes

// Pattern Timing
#define PATTERN_CHANGE_INTERVAL 180  // Seconds between pattern changes (3 minutes)
#define QUARTER_HOUR_EFFECT_DURATION 5  // Seconds for quarter-hour animations

// Sensor States
#define FOUND LOW
#define NOTFOUND HIGH

// Motor Directions
#define FORE 1
#define BACK -1

#endif