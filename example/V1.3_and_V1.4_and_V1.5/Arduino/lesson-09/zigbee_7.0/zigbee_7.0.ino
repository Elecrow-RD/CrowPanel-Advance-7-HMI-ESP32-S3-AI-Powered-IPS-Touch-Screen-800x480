/*---------------------------------------------------------------
 * UART pin assignment
 * Serial1 is exposed on the two pins used by the external module.
 *--------------------------------------------------------------*/
#define SERIAL1_RX 19
#define SERIAL1_TX 20

/**
 * @brief Initialize the debug UART and the module UART.
 *
 * Serial remains the debug channel; Serial1 carries newline-terminated
 * payloads from the external Zigbee module.
 */
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX);
  Serial.println("ESP32-S3 Serial Communication Example");
}

/**
 * @brief Forward one complete Serial1 line to the debug monitor.
 *
 * Reading stops at the newline delimiter, so the monitor shows the same
 * message boundary used by the external module.
 */
void loop() {
  if (Serial1.available()) {
    String message = Serial1.readStringUntil('\n');
    Serial.print("The message from serial port was received.: ");
    Serial.println(message);
  }
}
