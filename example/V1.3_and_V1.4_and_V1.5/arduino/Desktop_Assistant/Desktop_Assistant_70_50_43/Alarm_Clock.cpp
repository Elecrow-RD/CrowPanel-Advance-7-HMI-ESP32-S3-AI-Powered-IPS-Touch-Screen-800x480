// Alarm_Clock.cpp
#include "Alarm_Clock.h"
#include "ui.h"
#include <Arduino.h>
#include <Wire.h>

// User-selected alarm time
static int alarmHour = -1;
static int alarmMin = -1;
static bool alarmEnabled = false;

// Timing for buzzer
const int buzzerPin = 8;
static unsigned long alarmStartMillis = 0;
const unsigned long alarmDuration = 10000; // 10 seconds

// void buzzer_start()
// {
//   analogWrite(buzzerPin, 249);
// }

// void buzzer_stop()
// {
//   analogWrite(buzzerPin, 0);
// }

void buzzer_start() {
  // analogWrite(8, 249);
  // Start sending command 246 to address 0x30
  Wire.beginTransmission(0x30);            // 7-digit address 0x30
  Wire.write(246);                        // Send command 246
  uint8_t error = Wire.endTransmission();  // End transmission and return status

  if (error == 0) {
    Serial.println("Command 246 successfully sent");
  } else {
    Serial.print("Command sending error, error code：");
    Serial.println(error);
  }
}

void buzzer_stop() {
  // analogWrite(8, 0);
  //  Start sending command 247 to address 0x30
  Wire.beginTransmission(0x30);            // 7-digit address 0x30
  Wire.write(247);                        // Send command 247
  uint8_t error = Wire.endTransmission();  // End transmission and return status

  if (error == 0) {
    Serial.println("Command 247 successfully sent");
  } else {
    Serial.print("Command sending error, error code：");
    Serial.println(error);
  }
}

void initAlarmClock() {
    pinMode(buzzerPin, OUTPUT);
    // Stop buzzer at initialization
    buzzer_stop();
}

void Turn_On_Off_Alarm() {
    // Toggle alarm status
    alarmEnabled = (lv_obj_get_state(ui_ClockSwitch) & LV_STATE_CHECKED);
    if (alarmEnabled) {
        // Read selected hour and minute from dropdowns
        char buf[3];
        lv_dropdown_get_selected_str(ui_HourDropdown, buf, sizeof(buf));
        alarmHour = atoi(buf);
        lv_dropdown_get_selected_str(ui_MinDropdown, buf, sizeof(buf));
        alarmMin = atoi(buf);
        alarmStartMillis = 0; // Reset timer
        Serial.printf("Alarm set for %02d:%02d\n", alarmHour, alarmMin);
    } else {
        // Disable alarm and stop buzzer
        buzzer_stop();
        alarmStartMillis = 0;
        Serial.println("Alarm disabled");
    }
}

void handleAlarmClock(unsigned long currentMillis) {
    if (!alarmEnabled) return;

    // Get current time from the time label
    const char *timeText = lv_label_get_text(ui_TimeLabel);
    int hour, min, sec;
    if (sscanf(timeText, "%d:%d:%d", &hour, &min, &sec) != 3) return;

    // Trigger alarm when reaching the set time and second equals 0
    if (hour == alarmHour && min == alarmMin && sec == 0 && alarmStartMillis == 0) {
        buzzer_start();
        alarmStartMillis = currentMillis;
        Serial.println("Alarm triggered!");
    }

    // Stop after 10 seconds
    if (alarmStartMillis > 0 && currentMillis - alarmStartMillis >= alarmDuration) {
        buzzer_stop();
        alarmStartMillis = 0;
        alarmEnabled = false; // Automatically disable alarm state
        // Update UI switch to off
        lv_obj_clear_state(ui_ClockSwitch, LV_STATE_CHECKED);
        Serial.println("Alarm stopped");
    }
}
