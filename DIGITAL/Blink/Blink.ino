
#define OUTPUT_LED_PIN    9

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(OUTPUT_LED_PIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(OUTPUT_LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(OUTPUT_LED_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
