// Alarm_Clock.h
#pragma once

#include <lvgl.h>

void buzzer_start();
void buzzer_stop();
void initAlarmClock();
void Turn_On_Off_Alarm();
void handleAlarmClock(unsigned long currentMillis);