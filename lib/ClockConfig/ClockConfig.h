#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

/**
 * ClockConfig - Portable configuration for clock system
 * 
 * Contains default configuration values that can be overridden
 * by the main application config.h
 */

// Hardware defaults (can be overridden)
#ifndef SENSOR_PIN
#define SENSOR_PIN 2
#endif

#ifndef NEOPIXEL_PIN
#define NEOPIXEL_PIN 6
#endif

#ifndef FIRST_MOTOR_PIN
#define FIRST_MOTOR_PIN 14
#endif

// Motor defaults
#ifndef STEPS_PER_REVOLUTION
#define STEPS_PER_REVOLUTION 2048
#endif

#ifndef MOTOR_SPEED
#define MOTOR_SPEED 11
#endif

#ifndef SLOW_DELAY
#define SLOW_DELAY 0
#endif

#ifndef CENTERING_ADJUSTMENT
#define CENTERING_ADJUSTMENT 0
#endif

// LED defaults
#ifndef HOUR_LEDS
#define HOUR_LEDS 24
#endif

#ifndef MINUTE_LEDS
#define MINUTE_LEDS 12
#endif

#ifndef DEFAULT_BRIGHTNESS
#define DEFAULT_BRIGHTNESS 63
#endif

// Timing defaults
#ifndef RTC_CHECK_DELAY
#define RTC_CHECK_DELAY 50
#endif

// Quiet hours defaults
#ifndef QUIET_HOURS_START
#define QUIET_HOURS_START 22
#endif

#ifndef QUIET_HOURS_END
#define QUIET_HOURS_END 6
#endif

#ifndef QUIET_BRIGHTNESS_PERCENT
#define QUIET_BRIGHTNESS_PERCENT 50
#endif

// Helper functions for quiet hours
inline bool isQuietHours(int hour, int start = QUIET_HOURS_START, int end = QUIET_HOURS_END) {
    if (start > end) {
        // Quiet hours cross midnight
        return (hour >= start || hour < end);
    } else {
        // Quiet hours within same day
        return (hour >= start && hour < end);
    }
}

inline uint8_t getQuietBrightness(uint8_t defaultBrightness, int percent = QUIET_BRIGHTNESS_PERCENT) {
    return (defaultBrightness * percent) / 100;
}

#endif // CLOCK_CONFIG_H
