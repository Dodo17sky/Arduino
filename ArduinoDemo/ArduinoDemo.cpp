#include <Arduino.h>

/*
 *   This is a demo Arduino project from Eclipse
 */

#define		ON_BOARD_LED		13

void setup() {
	pinMode(ON_BOARD_LED, OUTPUT);
}

void loop() {
	digitalWrite(ON_BOARD_LED, HIGH);
	delay(1000);
	digitalWrite(ON_BOARD_LED, LOW);
	delay(1000);
}
