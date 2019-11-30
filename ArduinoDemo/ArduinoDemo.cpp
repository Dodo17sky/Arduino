#include <Arduino.h>

/*
 *   This is a demo Arduino project from Eclipse
 */


/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String        inputString     = "";             // a String to hold incoming data
boolean       stringComplete  = false;          // whether the string is complete
String        serialCommand;

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
	 pinMode(10, OUTPUT);
	 Serial.begin(9600);
}

void loop() {
	int inByte = 333;

	if (stringComplete) {
		serialCommand = inputString.substring(0,3);   // the first 3 characters define command type
		inByte = inputString.substring(0,3).toInt();

		stringComplete = false;
		while(Serial.read() >= 0) ; // flush the receive buffer
		inputString = "";
	}

	if(inByte >= 0 && inByte <=255) {
		analogWrite(10, inByte);
		Serial.print("Set pwm to ");
		Serial.println(inByte);
	}

	for(int i=0; i<256; i+=30) {
		analogWrite(10, i);
		delay(2000);
		Serial.println(i);
	}
	while(Serial.read() >= 0) ; // flush the receive buffer
}

/***********************************************************************
 *                        serialEvent()
 ***********************************************************************/
void serialEvent() {
	while (Serial.available()) {
		char inChar = (char)Serial.read();    // get the new byte:

		if (inChar == '\n') {                 // if the incoming character is a newline, set a flag
			stringComplete = true;              // so the main loop can do something about it:
		}
		else {
			inputString += inChar;              // add it to the inputString:
		}
	}
}
