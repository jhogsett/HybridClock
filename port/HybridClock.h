/**
 * HybridClock Library - Main Header
 * 
 * A hybrid analog-digital clock library for Arduino that combines:
 * - Stepper motor-driven analog clock mechanism
 * - NeoPixel LED ring displays
 * - DS3231 RTC for accurate timekeeping
 * - Configurable LED patterns and animations
 * 
 * This header includes all components for convenience.
 * You can also include individual headers as needed.
 * 
 * Usage:
 *   #include <HybridClock.h>
 *   
 *   Clock clock(2048, 14, 2, 6, 24, 12, 63, 11);
 *   
 *   void setup() {
 *     clock.begin();
 *   }
 *   
 *   void loop() {
 *     clock.update();
 *   }
 */

#ifndef HYBRIDCLOCK_H
#define HYBRIDCLOCK_H

// Core components
#include "ClockTime.h"
#include "ClockMotor.h"
#include "ClockDisplay.h"

// Managers
#include "AnimationManager.h"
#include "PatternManager.h"

// Main orchestrator
#include "Clock.h"

#endif // HYBRIDCLOCK_H
