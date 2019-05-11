#include <Arduino.h>

/*
 *   This is a demo Arduino project from Eclipse
 */


/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/


/***********************************************************************
 *                        CONSTANT DATA
 ***********************************************************************/
#define  ON_BOARD_LED_PIN		13

 /***********************************************************************
 *                        CUSTOM TIMERS
 ***********************************************************************/

/***********************************************************************
 *                        Private functions
 ***********************************************************************/

 void setup() {
	 pinMode(ON_BOARD_LED_PIN, OUTPUT);
	 digitalWrite(ON_BOARD_LED_PIN, LOW);
}

void loop() {
	digitalWrite(ON_BOARD_LED_PIN, HIGH);
	delay(1000);
	digitalWrite(ON_BOARD_LED_PIN, LOW);
	delay(1000);
}
