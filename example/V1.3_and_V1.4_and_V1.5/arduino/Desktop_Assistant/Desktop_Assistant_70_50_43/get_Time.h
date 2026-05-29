// get_Time.h
#pragma once

void initNTP();
void getTimeData();
void handleTimeUpdate(unsigned long currentMillis);
void handleNTPUpdate(unsigned long currentMillis);
