#include <Arduino.h>
#include <Thread.h>

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#define	ONBOARD_LED				13

#define PROCESS_THREAD(t)		if(t.shouldRun())	t.run()
#define THREAD_SETUP(t,c,i)		t.onRun(c); t.setInterval(i)

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
Thread		ledThread;        			// thread object
Thread		msgTh100 ;        			// thread object
Thread		msgTh1111;        			// thread object

String ts25   = "";
String ts100  = "";
String ts1111 = "";

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
void led_Process_callback(void);
void msg_P100_callback(void);
void msg_P1111_callback(void);

void setup() {
	Serial.begin(9600);
	pinMode(ONBOARD_LED, OUTPUT);
	ts25.reserve(300);
	ts100.reserve(300);
	ts1111.reserve(300);

	//            thread obj , callback function   , interval
	THREAD_SETUP ( ledThread , led_Process_callback, 1000   );
	THREAD_SETUP ( msgTh100  , msg_P100_callback   , 100    );
	THREAD_SETUP ( msgTh1111 , msg_P1111_callback  , 1111   );
}

void loop() {

	PROCESS_THREAD( ledThread );
	PROCESS_THREAD( msgTh100  );
	PROCESS_THREAD( msgTh1111 );

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

/***********************************************************************
 *                        msg_P100_callback
 ***********************************************************************/
void msg_P100_callback(void)
{
	static int counter = 1;

	ts100 += String(millis()) + " ";

	if( (counter % 20) == 0) {  // trigger each 2 seconds
		Serial.println(String("Thread 100: ") + ts100);
		ts100 = "";
		counter = 1;
	}
	counter++;
}

/***********************************************************************
 *                        msg_P1111_callback
 ***********************************************************************/
void msg_P1111_callback(void)
{

	static int counter = 1;

	ts1111 += String(millis()) + " ";

	if( (counter % 5) == 0) {  // trigger each 5555 ms
		Serial.println(String("Thread 1111: ") + ts1111);
		ts1111 = "";
		counter = 1;
	}
	counter++;
}

