/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Thread.h>

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String        inputString     = "";             // a String to hold incoming data
boolean       stringComplete  = false;          // whether the string is complete
boolean       isEnabled;
boolean       isMovingForward = true;           // the motor moving direction
String        serialCommand;
unsigned long Index;

unsigned long stepSpeed       = 1000;           // delay between steps [us]
unsigned long cycleDelay      = 0;              // used for separated movements
int           stepTarget      = 400;
int           stepCounter     = 400;

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#define 	PROCESS_THREAD(t)		if(t.shouldRun())	t.run()
#define 	THREAD_SETUP(t,c,i)		t.onRun(c); t.setInterval(i)

#define		DEBUG_ON                1
#define     MOTOR_ENABLE_PIN        7
#define     MOTOR_STEP_PIN          6
#define     MOTOR_DIR_PIN           5
#define     END_DETECTOR_RIGHT      8
#define     END_DETECTOR_LEFT       9
#define     END_REACHED             LOW

// Stepper motor Nema17 commands
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
void 	Serial_Process(void);
void 	Motor_Process(void);
void 	ReadInputs(void);
void    MoveExactly(uint16_t stepNumber);

/***********************************************************************
 *                        setup() function
 ***********************************************************************/
void setup() {
	inputString.reserve(200);             // reserve 200 bytes for the inputString:

	// setup stepper motor pins
	pinMode(MOTOR_ENABLE_PIN, OUTPUT);    // enable pin
	pinMode(MOTOR_DIR_PIN   , OUTPUT);    // step
	pinMode(MOTOR_STEP_PIN  , OUTPUT);    // direction

	// setup ends detectors pins
	pinMode(END_DETECTOR_LEFT , INPUT_PULLUP);
	pinMode(END_DETECTOR_RIGHT, INPUT_PULLUP);

	Serial.begin(9600);
	Motor_Off();
	Motor_SetDirForward();

	TM_START(tm_StepHigh);
}

/***********************************************************************
 *                        loop() function
 ***********************************************************************/
void loop() {
	ReadInputs();
	Serial_Process();
	Motor_Process();
}

/***********************************************************************
 *                        ReadInputs() function
 ***********************************************************************/
void ReadInputs(void)
{
	if( digitalRead(END_DETECTOR_LEFT ) == END_REACHED) {
		Serial.println("Left stop");
		Motor_Off();
		Motor_SetDirForward();
		delay(1000);
		MoveExactly(80);
		Motor_On();
	}
	if( digitalRead(END_DETECTOR_RIGHT) == END_REACHED) {
		Serial.println("Right stop");
		Motor_Off();
		Motor_SetDirBackward();
		delay(1000);
		MoveExactly(80);
		Motor_On();
	}
}

/***********************************************************************
 *                        MoveExactly() function
 ***********************************************************************/
void MoveExactly(uint16_t stepNumber)
{
    Motor_On();
    for(uint16_t s=0; s<stepNumber; s++)
    {
        Motor_StepHigh();
        delayMicroseconds(stepSpeed);
        Motor_StepLow();
        delayMicroseconds(stepSpeed);
    }
    Motor_Off();
}

/***********************************************************************
 *                        Serial_Process() function
 ***********************************************************************/
void Serial_Process(void)
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
            #if (DEBUG_ON == 1)
            Serial.print(stepSpeed);
            #endif
        }

        if(serialCommand == ">>>") {
            Serial.println("Move to right");
            Motor_SetDirForward();
        }

        if(serialCommand == "<<<") {
            Serial.println("Move to left");
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
}

/***********************************************************************
 *                        Motor_Process() function
 ***********************************************************************/
void Motor_Process(void)
{
    if( Motor_IsOff )
        return;

    if( TM_DONE(tm_StepHigh, stepSpeed) ) {
        Motor_StepHigh();
        TM_STOP(tm_StepHigh);
        TM_START(tm_StepLow);
    }

    if( TM_DONE(tm_StepLow, stepSpeed) ) {
        Motor_StepLow();
        TM_STOP(tm_StepLow);
        TM_START(tm_StepHigh);
        stepCounter ++;
    }

    if ( (cycleDelay > 0) && (stepCounter >= stepTarget) ) {
        stepCounter = 0;
        delay(cycleDelay);
    }
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
