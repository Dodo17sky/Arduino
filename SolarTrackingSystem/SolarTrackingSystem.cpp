#include <Arduino.h>
#include "MicroTimer.h"
#include "Debouncer.h"

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String        	inputString     = "";             // a String to hold incoming data
boolean       	isEnabled       = true;
boolean       	isMovingForward = true;           // the motor moving direction
String        	serialCommand;
boolean       	stringComplete  = false;          // whether the string is complete

unsigned long 	stepSpeed       = 1000;           // delay between steps [us]
unsigned long 	cycleDelay      = 1000;           // delay between directions [ms]
int           	stepTarget      = 400;			// number of steps for one cycle
int           	stepCounter     = 0;            	// counter of steps done

// debounce stopper pins
Debouncer		verticalStopper;
Debouncer		horizontalStopper;
volatile unsigned long EndCntV	= 0;
volatile unsigned long EndCntH	= 0;

MicroTimer	  	utMotorStepHigh;
MicroTimer	  	utMotorStepLow;

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#define		DEBUG_ON                1
#define		ONBOARD_LED				13
#define     MOTOR_ENABLE_PIN        7
#define     MOTOR_STEP_PIN          6
#define     MOTOR_DIR_PIN           5
#define		ROT_STOPPER_PIN			2

#define     Motor_On()              digitalWrite(MOTOR_ENABLE_PIN, LOW); isEnabled = true
#define     Motor_Off()             digitalWrite(MOTOR_ENABLE_PIN, HIGH); isEnabled = false
#define     Motor_IsOn              (isEnabled == true)
#define     Motor_IsOff             (isEnabled == false)

#define     Motor_SetSpeed(x)       stepSpeed = x
#define     Motor_SetSteps(x)       stepTarget = x

#define     Motor_SetDirForward()   digitalWrite(MOTOR_DIR_PIN, HIGH); isMovingForward = true
#define     Motor_SetDirBackward()  digitalWrite(MOTOR_DIR_PIN, LOW); isMovingForward = false

#define     Motor_StepHigh()        digitalWrite(MOTOR_STEP_PIN, HIGH)
#define     Motor_StepLow()         digitalWrite(MOTOR_STEP_PIN, LOW)

 /***********************************************************************
 *                        CUSTOM TIMERS
 ***********************************************************************/
 unsigned long tm_StepHigh      = 0;
 unsigned long tm_StepLow       = 0;

 #define      TM_STOP_VALUE           0xFFFFFFFF
 #define      TM_START(x)             x = micros()
 #define      TM_STOP(x)              x = TM_STOP_VALUE
 #define      TM_DONE(x,t)            ( (x != TM_STOP_VALUE) && (micros() >= (x+t)) )

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
uint8_t Serial_Process(void);
uint8_t Motor_Process(void);
uint8_t EndsCheck_Process(void);

 /***********************************************************************
  *                        ISR
  ***********************************************************************/
void Stopper_Callback(void);

 /***********************************************************************
  *                        setup() function
  ***********************************************************************/
 void setup() {
	pinMode(ROT_STOPPER_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(ROT_STOPPER_PIN), Stopper_Callback, FALLING);
	inputString.reserve(200);             // reserve 200 bytes for the inputString:

	pinMode(MOTOR_ENABLE_PIN, OUTPUT);    // enable pin
	pinMode(MOTOR_DIR_PIN   , OUTPUT);    // step
	pinMode(MOTOR_STEP_PIN  , OUTPUT);    // direction

	pinMode(ONBOARD_LED, OUTPUT);
	digitalWrite(ONBOARD_LED, LOW);

	Serial.begin(57600);
	Motor_On();
	Motor_SetDirForward();

	// set motor speed => controlled by these two timers
	utMotorStepHigh.setPeriod(stepSpeed);
	utMotorStepLow.setPeriod(stepSpeed);

	// start high phase timer
	utMotorStepHigh.restart();  // start() > you set a specific period; restart() > used already set period
}

 /***********************************************************************
  *                        loop() function
  ***********************************************************************/
void loop() {
    Serial_Process();
    Motor_Process();
    EndsCheck_Process();
}

/***********************************************************************
 *                        Serial_Process()
 ***********************************************************************/
uint8_t Serial_Process(void)
{
    if (stringComplete) {
        serialCommand = inputString.substring(0,3);   // the first 3 characters define command type
        #if (DEBUG_ON == 1)
        Serial.print(String("\nCmd: ") + serialCommand + " ");
        #endif

        if(serialCommand == "onn") {
            Motor_On();
        }

        if(serialCommand == "off") {
            Motor_Off();
        }

        if(serialCommand == "spd") {
            Motor_SetSpeed( inputString.substring(3).toInt() );
            // set motor speed => controlled by these two timers
			utMotorStepHigh.setPeriod(stepSpeed);
			utMotorStepLow.setPeriod(stepSpeed);
            #if (DEBUG_ON == 1)
            Serial.print(stepSpeed);
            #endif
        }

        if(serialCommand == ">>>") {
            Motor_SetDirForward();
        }

        if(serialCommand == "<<<") {
            Motor_SetDirBackward();
        }

        if(serialCommand == "nmb") {
            Motor_SetSteps( inputString.substring(3).toInt() );
            #if (DEBUG_ON == 1)
            Serial.print(stepTarget);
            #endif
        }

        if(serialCommand == "del") {
            String tmp = inputString.substring(3);
            cycleDelay = tmp.toInt();
            Serial.print(cycleDelay);
        }

        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }
    return 0;
}

/***********************************************************************
 *                        Motor_Process()
 ***********************************************************************/
uint8_t Motor_Process(void)
{
    if( Motor_IsOn ) {
    	// motor is ON

		if( utMotorStepHigh.done() ) {
			Motor_StepHigh();
			utMotorStepHigh.stop();
			utMotorStepLow.restart();
		}

		if( utMotorStepLow.done() ) {
			Motor_StepLow();
			utMotorStepHigh.restart();
			utMotorStepLow.stop();
			stepCounter ++;
		}

		if ( (cycleDelay > 0) && (stepCounter >= stepTarget) ) {
			stepCounter = 0;
			delay(cycleDelay);
		}

		return 0;
    }
    else {
    	// motor is OFF
    	return 1;
    }
}

/***********************************************************************
 *                        EndsCheck_Process()
 ***********************************************************************/
uint8_t EndsCheck_Process(void)
{
	static unsigned long lastEndCntV = 0;
	static unsigned long lastEndCntH = 0;
	static bool eventReady = false;
	static bool ledState = true;
	static int eventCounter = 0;

	verticalStopper.tick(EndCntV-lastEndCntV);
	lastEndCntV = EndCntV;
	horizontalStopper.tick(EndCntH-lastEndCntH);
	lastEndCntH = EndCntH;

	if(verticalStopper.isSteady()) {
		if(eventReady == false) {
			// debounce just finished
			ledState = !ledState;
			digitalWrite(ONBOARD_LED, ledState);

			eventCounter ++;
			Serial.print(String("\n event ") + eventCounter);

			eventReady = true;
		}
	}
	else {
		eventReady = false;
	}


	return 0;
}

/***********************************************************************
 *                        ISR Stopper_Callback
 ***********************************************************************/
void Stopper_Callback(void)
{
	EndCntV++;
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
