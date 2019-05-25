/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Thread.h>

#define		DEBUG_ON                0

/***********************************************************************
 *                        GLOBAL TYPES
 ***********************************************************************/
typedef struct {
	uint8_t Left 	: 1;
	uint8_t Right	: 1;
	uint8_t Front	: 1;
	uint8_t Rear	: 1;
} EndDetector;

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
#if (DEBUG_ON == 1)
String      inputString     = "";             // a String to hold incoming data
boolean     stringComplete  = false;          // whether the string is complete
String      serialCommand;
#endif
boolean     isEnabled;
boolean     isMovingForward = true;           // the motor moving direction

EndDetector Ends;

unsigned long stepSpeed       = 4000;           // delay between steps [us]
int           stepTarget      = 400;
int           stepCounter     = 400;

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#if (DEBUG_ON == 1)
#define		SERIAL(x)				Serial.print(x)
#else
#define		SERIAL(x)
#endif

#define     MOTOR_ENABLE_PIN        3
#define     MOTOR_STEP_PIN          4
#define     MOTOR_DIR_PIN           5
#define     END_DETECTOR_RIGHT      7
#define     END_DETECTOR_LEFT       6
#define     END_DETECTOR_FRONT		9
#define     END_DETECTOR_REAR		8
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
#if (DEBUG_ON == 1)
void 	Serial_Process(void);
#endif
void 	Motor_Process(void);
void 	ReadInputs(void);
void    MoveExactly(uint16_t stepNumber);
void 	GlobalData_Init(void);

/***********************************************************************
 *                        setup() function
 ***********************************************************************/
void setup() {
	GlobalData_Init();

	// setup stepper motor pins
	pinMode(MOTOR_ENABLE_PIN, OUTPUT);    // enable pin
	pinMode(MOTOR_DIR_PIN   , OUTPUT);    // step
	pinMode(MOTOR_STEP_PIN  , OUTPUT);    // direction

	// setup ends detectors pins
	pinMode(END_DETECTOR_LEFT , INPUT_PULLUP);
	pinMode(END_DETECTOR_RIGHT, INPUT_PULLUP);

#if (DEBUG_ON == 1)
	Serial.begin(9600);
#endif

	delay(3000);

	Motor_On();
	Motor_SetDirForward();

	TM_START(tm_StepHigh);
}

/***********************************************************************
 *                        loop() function
 ***********************************************************************/
void loop() {
	ReadInputs();
	Motor_Process();
#if (DEBUG_ON == 1)
	Serial_Process();
#endif
}

/***********************************************************************
 *                        ReadInputs() function
 ***********************************************************************/
void ReadInputs(void)
{
	if( digitalRead(END_DETECTOR_LEFT ) == END_REACHED) {
		Ends.Left = 1;
	}
	else {
		Ends.Left = 0;
	}
	
	if( digitalRead(END_DETECTOR_RIGHT) == END_REACHED) {
		Ends.Right = 1;
	}
	else {
		Ends.Right = 0;
	}
	
	if( digitalRead(END_DETECTOR_FRONT ) == END_REACHED) {
		Ends.Front = 1;
	}
	else {
		Ends.Front = 0;
	}

	if( digitalRead(END_DETECTOR_REAR) == END_REACHED) {
		Ends.Rear = 1;
	}
	else {
		Ends.Rear = 0;
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
#if (DEBUG_ON == 1)
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

        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }
}
#endif

/***********************************************************************
 *                        Motor_Process() function
 ***********************************************************************/
void Motor_Process(void)
{
    if( Motor_IsOff )
        return;

    if( Ends.Left) {
    	SERIAL("Left stop\n");
		Motor_Off();
		Motor_SetDirForward();
		delay(1000);
		MoveExactly(80);
		Motor_On();
	}
    else if( Ends.Right ) {
    	SERIAL("Right stop\n");
		Motor_Off();
		Motor_SetDirBackward();
		delay(1000);
		MoveExactly(80);
		Motor_On();
	}

    // Nema17  -  M A K E   N E X T   S T E P     -  START
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
    // Nema17  -  M A K E   N E X T   S T E P     -  STOP
}

/***********************************************************************
 *                        GlobalData_Init() function
 ***********************************************************************/
void GlobalData_Init(void)
{
#if (DEBUG_ON == 1)
	inputString.reserve(200);			// reserve 200 bytes for the inputString:
#endif
	Ends.Left  = 0;
	Ends.Right = 0;
	Ends.Front = 0;
	Ends.Rear  = 0;
}

/***********************************************************************
 *                        serialEvent()
 ***********************************************************************/
#if (DEBUG_ON == 1)
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
#endif
