#include "timer.h"
#include <Arduino.h>

// LVGL timer label object
extern lv_obj_t * ui_TimerLabel;

// State variables
static bool running = false;
static unsigned long accumulated = 0;   // Accumulated milliseconds (whole seconds × 1000 + previous offsets)
static unsigned long last_tick = 0;     // Last tick "time base"
static unsigned long offset_ms = 0;     // Stored leftover milliseconds

// Update display
static void updateDisplay() {
    unsigned long total = accumulated;
    unsigned int hours   = total / 3600000UL;
    unsigned int minutes = (total % 3600000UL) / 60000UL;
    unsigned int seconds = (total % 60000UL) / 1000UL;
    char buff[9];
    snprintf(buff, sizeof(buff), "%02u:%02u:%02u", hours, minutes, seconds);
    lv_label_set_text(ui_TimerLabel, buff);
}

void initTimer() {
    // Reset state and initialize display
    running = false;
    accumulated = 0;
    offset_ms = 0;
    last_tick = millis();
    lv_label_set_text(ui_TimerLabel, "00:00:00");
}

void handleTimer() {
    if (!running) return;
    unsigned long now = millis();
    unsigned long delta = now - last_tick;
    if (delta >= 1000) {
        unsigned long secs = delta / 1000;
        accumulated += secs * 1000;
        last_tick += secs * 1000;
        updateDisplay();
    }
}

void Activate_Timer() {
    if (!running) {
        // Restore base time by subtracting leftover milliseconds
        unsigned long now = millis();
        last_tick = now - offset_ms;
        running = true;
    }
}

void Deactivate_Timer() {
    if (running) {
        unsigned long now = millis();
        unsigned long delta = now - last_tick;
        // Split: whole seconds and leftover milliseconds
        unsigned long whole_secs = delta / 1000;
        unsigned long rem_ms = delta % 1000;
        // Add whole seconds to accumulated time
        accumulated += whole_secs * 1000;
        // Store leftovers to restore base next Start
        offset_ms = rem_ms;
        running = false;
    }
}

void Reset_Timer() {
    running = false;
    accumulated = 0;
    offset_ms = 0;
    last_tick = millis();
    lv_label_set_text(ui_TimerLabel, "00:00:00");
}
