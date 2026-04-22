#include "Light_Control.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

static QueueHandle_t cmdQueue;    // Message queue for inter-task communication
// IP or domain name of the controlled device (can be changed to your slave IP)
const char* slave_host = "192.168.50.253"; // The IP address of the slave, wait for the slave to burn successfully, and check the slave IP by printing through the serial port.

// General HTTP request function (called only in the network task)
static void sendControlCommand(bool turnOn) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, unable to control LED");
        return;
    }
    HTTPClient http;
    String url = String("http://") + slave_host + (turnOn ? "/on" : "/off");
    http.begin(url);
    int code = http.GET();
    if (code > 0) {
        Serial.printf("Command %s returned %d\n", turnOn ? "ON" : "OFF", code);
    } else {
        Serial.printf("HTTP request failed: %s\n", http.errorToString(code).c_str());
    }
    http.end();
}

// FreeRTOS network task: retrieve command from queue and send it
static void networkTask(void* param) {
    bool cmd;
    while (true) {
        if (xQueueReceive(cmdQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            sendControlCommand(cmd);
        }
    }
}

void lightControlInit() {
    // Create a queue with capacity 4, element type is bool
    cmdQueue = xQueueCreate(4, sizeof(bool));
    // Start the network task, stack 4 KB, priority 5
    xTaskCreate(networkTask, "NetworkTask", 4096, nullptr, 5, nullptr);

    Serial.println("Light control initialized. You can now operate it on Page 6!");
}

void On_LED() {
    bool on = true;
    // Push "turn on light" command to queue without waiting
    xQueueSend(cmdQueue, &on, 0);
    Serial.println("Turn-on request sent!");
}

void Off_LED() {
    bool off = false;
    // Push "turn off light" command to queue without waiting
    xQueueSend(cmdQueue, &off, 0);
    Serial.println("Turn-off request sent!");
}
