#include <Arduino.h>
#include <neotimer.h>

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#define	ONBOARD_LED				13

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
Neotimer		timer1		= Neotimer(2222);
unsigned long   LoopCalls 	= 0;

void setup() {
	Serial.begin(57600);
	pinMode(ONBOARD_LED, OUTPUT);

}

void loop() {

	if(  timer1.repeat(20) ) {
		Serial.println( String("Timer trigger: ") + millis() );
		Serial.println( String("Loop called for ") + LoopCalls + " times");
		LoopCalls = 0;
	}
	else {
		LoopCalls ++;
	}
}
