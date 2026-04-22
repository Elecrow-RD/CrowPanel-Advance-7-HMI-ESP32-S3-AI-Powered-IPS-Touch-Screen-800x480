// get_Time.cpp
#include "get_Time.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ui.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

static unsigned long lastTimeUpdate = 0;
static unsigned long lastNTPSync = 0;
const long timeUpdateInterval = 1000; // 1 second
const long ntpSyncInterval = 600000; // 10 minutes

void initNTP() {
  setenv("TZ", "CST-8", 1); // Set Beijing time
  // setenv("TZ", "EST5EDT", 1);  // Set New York Time Zone (Eastern Time, supports daylight saving time)
  tzset();
  timeClient.begin();
  timeClient.setUpdateInterval(ntpSyncInterval);
  timeClient.forceUpdate();
}

void getTimeData() {
  timeClient.update();
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeInfo = localtime(&rawTime);

  char timeBuf[9];
  char dateBuf[11];
  strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", timeInfo);
  strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", timeInfo);

  lv_label_set_text(ui_TimeLabel, timeBuf);
  lv_label_set_text(ui_DateLabel, dateBuf);

  Serial.printf("Current time: %s %s\n", dateBuf, timeBuf);
}

void handleTimeUpdate(unsigned long currentMillis) {
  if (currentMillis - lastTimeUpdate >= timeUpdateInterval) {
    lastTimeUpdate = currentMillis;
    getTimeData();
  }
}

void handleNTPUpdate(unsigned long currentMillis) {
  if (currentMillis - lastNTPSync >= ntpSyncInterval) {
    timeClient.forceUpdate();
    lastNTPSync = currentMillis;
  }
}
