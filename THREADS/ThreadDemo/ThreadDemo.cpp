#include <Arduino.h>
#include <Thread.h>

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#define	ONBOARD_LED				13

#define PROCESS_THREAD(t)		if(t.shouldRun())	t.run()

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
Thread		ledThread;        			// thread object

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
void led_Process_callback(void);

void setup() {
	Serial.begin(57600);
	pinMode(ONBOARD_LED, OUTPUT);

	ledThread.onRun(led_Process_callback);
	ledThread.setInterval(1000);
}

void loop() {

	PROCESS_THREAD( ledThread );

}

/***********************************************************************
 *                        led_Process_callback
 ***********************************************************************/
void led_Process_callback(void)
{
	static bool ledState = false;
	ledState = !ledState;

	digitalWrite(ONBOARD_LED, ledState);
	Serial.println(String("Cool, I'm running ") + millis());
}

