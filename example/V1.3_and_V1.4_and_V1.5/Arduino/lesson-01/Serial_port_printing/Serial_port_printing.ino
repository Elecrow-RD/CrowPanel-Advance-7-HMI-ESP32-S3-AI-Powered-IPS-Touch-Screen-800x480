/*---------------------------------------------------------------
 * Counter state
 * Keeps the value printed by the periodic serial reporting loop.
 *--------------------------------------------------------------*/
int counter = 0;

/**
 * @brief Configure the serial interface used by the lesson.
 *
 * Called once after reset so the monitor can receive the counter output.
 */
void setup() {
  Serial.begin(9600);
}

/**
 * @brief Print one counter sample at a fixed interval.
 *
 * The loop runs continuously; the delay establishes a one-second sample
 * period that is easy to observe in the serial monitor.
 */
void loop() {
  Serial.print("Hello, World! ");
  Serial.print("--- ");
  Serial.println(counter);

  counter++;
  delay(1000);
}
