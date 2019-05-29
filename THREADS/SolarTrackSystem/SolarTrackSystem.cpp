/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Stepper.h>

#include "Nema17Motor.h"
#include "Stepper28BYJ.h"

#define		DEBUG_ON                1		// Compiler switch for Serial commands
#define		MOTOR_ACTIVE_28BYJ	    1		// Compiler switch to deactivate all motor 28BYJ functionality
#define		MOTOR_ACTIVE_NEMA17		1		// Compiler switch to deactivate all motor Nema17 functionality

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
String      inputString     = "";           // a String to hold incoming data
boolean     stringComplete  = false;        // whether the string is complete
String      serialCommand;
#endif

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#if (DEBUG_ON == 1)
#define		SERIAL(x)				Serial.print(x)
#else
#define		SERIAL(x)
#endif

// Nema17 data
#define     MOTOR_ENABLE_PIN        3
#define     MOTOR_STEP_PIN          4
#define     MOTOR_DIR_PIN           5
#define     END_DETECTOR_RIGHT      7
#define     END_DETECTOR_LEFT       6

#if (MOTOR_ACTIVE_28BYJ == 1)
#define     END_DETECTOR_FRONT		9
#define     END_DETECTOR_REAR		8
#endif

#define     END_REACHED             LOW
#define		NEMA17_INACTIVE			0
#define		NEMA17_ACTIVE			1
#define		NEMA17_END_LEFT			2
#define		NEMA17_END_RIGHT		3

#if (MOTOR_ACTIVE_28BYJ == 1)
// 28BYJ data
#define 	LINE1 					13		// motor 28BYJ driver input line 1
#define 	LINE2 					12		// motor 28BYJ driver input line 2
#define 	LINE3 					11		// motor 28BYJ driver input line 3
#define 	LINE4 					10		// motor 28BYJ driver input line 4

// 28BYJ commands
Stepper28BYJ motor28BYJ(LINE1, LINE3, LINE2, LINE4);
#endif

#if (MOTOR_ACTIVE_NEMA17 == 1)
// Nema17 data
Nema17Motor Nema17(MOTOR_DIR_PIN, MOTOR_STEP_PIN, MOTOR_ENABLE_PIN);
EndDetector Ends;
uint8_t		Nema17_State;
uint16_t 	stepSpeed;
uint8_t		endsSteps;
#endif

/***********************************************************************
 *                        CUSTOM TIMERS
 ***********************************************************************/
uint32_t	tm_StepHigh      		= 0;
uint32_t	tm_StepLow       		= 0;
uint32_t	tm_EndLeft       		= 0;
uint32_t	tm_EndRight      		= 0;

#define		TM_STOP_VALUE           0xFFFFFFFF
#define		TM_START(x)             if(x == TM_STOP_VALUE) { x = micros(); }
#define		TM_STOP(x)              x = TM_STOP_VALUE
#define		TM_DONE(x,t)            ( (x != TM_STOP_VALUE) && (micros() >= (x+(t))) )

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
#if (DEBUG_ON == 1)
void 	Serial_Process(void);
#endif

void 	ReadInputs(void);
void 	GlobalData_Init(void);

void 	Motor_Nema17_Process(void);
void	Motor_Nema17_Inactive();
void	Motor_Nema17_Active();
void	Motor_Nema17_Left();
void	Motor_Nema17_Right();

#if (MOTOR_ACTIVE_28BYJ == 1)
void 	Motor_28BYJ_Process(void);
#endif

/***********************************************************************
 *                        setup() function
 ***********************************************************************/
void setup() {
	GlobalData_Init();

#if (DEBUG_ON == 1)
	Serial.begin(9600);
#endif

#if (MOTOR_ACTIVE_NEMA17 == 1)
	// setup ends detectors pins
	pinMode(END_DETECTOR_LEFT , INPUT_PULLUP);
	pinMode(END_DETECTOR_RIGHT, INPUT_PULLUP);
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
	// Motor 28BYJ setups
	pinMode(END_DETECTOR_FRONT, INPUT_PULLUP);
	pinMode(END_DETECTOR_REAR , INPUT_PULLUP);
	motor28BYJ.setSpeed(10000);
#endif

	delay(3000);
}

/***********************************************************************
 *                        loop() function
 ***********************************************************************/
void loop() {
	ReadInputs();

#if (MOTOR_ACTIVE_NEMA17 == 1)
	Motor_Nema17_Process();
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
	Motor_28BYJ_Process();
#endif

#if (DEBUG_ON == 1)
	Serial_Process();
#endif
}

/***********************************************************************
 *                        ReadInputs() function
 ***********************************************************************/
void ReadInputs(void)
{
	stepSpeed = Nema17.getSpeed();

#if (MOTOR_ACTIVE_NEMA17 == 1)
	if( digitalRead(END_DETECTOR_LEFT ) == END_REACHED) {
		Ends.Left = 1;
		Nema17_State = NEMA17_END_LEFT;
		TM_START(tm_EndLeft);
	}
	else {
		Ends.Left = 0;
	}
	
	if( digitalRead(END_DETECTOR_RIGHT) == END_REACHED) {
		Ends.Right = 1;
		Nema17_State = NEMA17_END_RIGHT;
		TM_START(tm_EndRight);
	}
	else {
		Ends.Right = 0;
	}
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
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
#endif
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
        	Nema17.turnOn();
            Nema17_State = NEMA17_ACTIVE;
#if (MOTOR_ACTIVE_28BYJ == 1)
            motor28BYJ.TurnOn();
#endif
        }

        if(serialCommand == "off") {
            Nema17.turnOff();
            Nema17_State = NEMA17_INACTIVE;
#if (MOTOR_ACTIVE_28BYJ == 1)
            motor28BYJ.TurnOff();
#endif
        }

        if(serialCommand == "spd") {
        	int tmpSpeed = inputString.substring(3).toInt();
        	Nema17.setSpeed(tmpSpeed);
            #if (DEBUG_ON == 1)
            SERIAL(tmpSpeed);
            #endif
        }

        if(serialCommand == ">>>") {
            SERIAL("Move to right\n");
            Nema17.goForward();
        }

        if(serialCommand == "<<<") {
        	SERIAL("Move to left\n");
            Nema17.goBackward();
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
void Motor_Nema17_Process(void)
{
	switch(Nema17_State) {
		case NEMA17_INACTIVE:
			Motor_Nema17_Inactive();
			break;
		case NEMA17_ACTIVE:
			Motor_Nema17_Active();
			break;
		case NEMA17_END_LEFT:
			Motor_Nema17_Left();
			break;
		case NEMA17_END_RIGHT:
			Motor_Nema17_Right();
			break;
		default:
			break;
	}
}

/***********************************************************************
 *                        Motor_Nema17_Inactive() function
 ***********************************************************************/
void	Motor_Nema17_Inactive()
{
	if( Nema17_State != NEMA17_INACTIVE ) {
		return;
	}
}

/***********************************************************************
 *                        Motor_Nema17_Active() function
 ***********************************************************************/
void	Motor_Nema17_Active()
{
	// Nema17  -  M A K E   N E X T   S T E P     -  START
	if( TM_DONE(tm_StepHigh, stepSpeed) ) {
		Nema17.stepHigh();
		TM_STOP(tm_StepHigh);
		TM_START(tm_StepLow);
	}

	if( TM_DONE(tm_StepLow, stepSpeed) ) {
		Nema17.stepLow();
		TM_STOP(tm_StepLow);
		TM_START(tm_StepHigh);
	}
	// Nema17  -  M A K E   N E X T   S T E P     -  STOP
}

/***********************************************************************
 *                        Motor_Nema17_Left() function
 ***********************************************************************/
void	Motor_Nema17_Left()
{
	Nema17.turnOff();
	TM_START(tm_EndLeft);
	if(TM_DONE(tm_EndLeft,3000000)) {
		Nema17.goForward();
		Nema17.turnOn();
		// Nema17  -  M A K E   N E X T   S T E P     -  START
		if( TM_DONE(tm_StepHigh, stepSpeed) ) {
			Nema17.stepHigh();
			TM_STOP(tm_StepHigh);
			TM_START(tm_StepLow);
		}

		if( TM_DONE(tm_StepLow, stepSpeed) ) {
			Nema17.stepLow();
			TM_STOP(tm_StepLow);
			TM_START(tm_StepHigh);
			endsSteps++;
		}
		// Nema17  -  M A K E   N E X T   S T E P     -  STOP
		if(endsSteps == 80) {
			endsSteps = 0;
			Nema17_State = NEMA17_ACTIVE;
			TM_STOP(tm_EndLeft);
		}
	}
}

/***********************************************************************
 *                        Motor_Nema17_Right() function
 ***********************************************************************/
void	Motor_Nema17_Right()
{
	Nema17.turnOff();
	TM_START(tm_EndRight);
	if(TM_DONE(tm_EndRight,3000000)) {
		Nema17.goBackward();
		Nema17.turnOn();
		// Nema17  -  M A K E   N E X T   S T E P     -  START
		if( TM_DONE(tm_StepHigh, stepSpeed) ) {
			Nema17.stepHigh();
			TM_STOP(tm_StepHigh);
			TM_START(tm_StepLow);
		}

		if( TM_DONE(tm_StepLow, stepSpeed) ) {
			Nema17.stepLow();
			TM_STOP(tm_StepLow);
			TM_START(tm_StepHigh);
			endsSteps++;
		}
		// Nema17  -  M A K E   N E X T   S T E P     -  STOP
		if(endsSteps == 80) {
			endsSteps = 0;
			Nema17_State = NEMA17_ACTIVE;
			TM_STOP(tm_EndRight);
		}
	}
}

/***********************************************************************
 *                        Motor_28BYJ_Process() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_28BYJ == 1)
void Motor_28BYJ_Process(void)
{
	if( motor28BYJ.isOn() == false ) {
		return;
	}

	if( Ends.Front )
	{
		SERIAL("Front stop");
		motor28BYJ.GoBackward();
		while( digitalRead(END_DETECTOR_FRONT) == END_REACHED ) {
			// turn back until no end detected
			motor28BYJ.stepExactly(20);
		}
		return;
	}
	else if( Ends.Rear )
	{
		SERIAL("Rear stop");
		motor28BYJ.GoForward();
		while( digitalRead(END_DETECTOR_REAR) == END_REACHED ) {
			motor28BYJ.stepExactly(20);
		}
		return;
	}

	motor28BYJ.step();
}
#endif

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

#if (MOTOR_ACTIVE_NEMA17 == 1)
	// Motor Nema17 data
	Nema17.turnOn();
	Nema17.goBackward();
	TM_START(tm_StepHigh);
	TM_STOP(tm_StepLow);
	TM_STOP(tm_EndLeft);
	TM_STOP(tm_EndRight);
	Nema17_State = NEMA17_ACTIVE;
	endsSteps = 0;
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
	// Motor 28BYJ data
	motor28BYJ.TurnOn();
#endif
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
