#include <Arduino.h>

/*
 *   This is a demo Arduino project from Eclipse
 */

void setup() {
	pinMode(13, OUTPUT);
}

void loop() {
	digitalWrite(13, HIGH);
	delay(1000);
	digitalWrite(13, LOW);
	delay(1000);
}
