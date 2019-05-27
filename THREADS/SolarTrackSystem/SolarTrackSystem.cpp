/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Stepper.h>
#include "Stepper28BYJ.h"

#define		DEBUG_ON                1		// Compiler switch for Serial commands
#define		MOTOR_ACTIVE_28BYJ	    1		// Compiler switch to deactivate all motor 28BYJ functionality
#define		MOTOR_ACTIVE_NEMA17		1		// Compiler switch to deactivate all motor Nema17 functionality

typedef struct {
	int check_Char 		: (sizeof(char			)==1) ? 1 : -1;
	int check_Short 	: (sizeof(short			)==2) ? 1 : -1;
	int check_Int 		: (sizeof(int			)==2) ? 1 : -1;
	int check_Long 		: (sizeof(long			)==4) ? 1 : -1;
	int check_Float 	: (sizeof(float 		)==4) ? 1 : -1;
	int check_Double	: (sizeof(double		)==4) ? 1 : -1;
	int check_LongLong	: (sizeof(long long		)==8) ? 1 : -1;
} DataTypesSizeChecker;

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

// Nema17 data
boolean     isNema17Enabled;
boolean     isMovingForward = true;			// the motor moving direction
EndDetector Ends;
uint32_t	stepSpeed       = 2500;			// delay between steps [us]
uint8_t		Nema17_State;

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

// Nema17 commands
#define     Motor_Nema17_On()       		digitalWrite(MOTOR_ENABLE_PIN, LOW); isNema17Enabled = true
#define     Motor_Nema17_Off()      		digitalWrite(MOTOR_ENABLE_PIN, HIGH); isNema17Enabled = false
#define     Motor_Nema17_IsOn       		(isNema17Enabled == true)
#define     Motor_Nema17_IsOff      		(isNema17Enabled == false)
#define     Motor_Nema17_SetSpeed(x)       	stepSpeed = (x)
#define     Motor_Nema17_SetDirForward()   	digitalWrite(MOTOR_DIR_PIN, HIGH); isMovingForward = true
#define     Motor_Nema17_SetDirBackward()	digitalWrite(MOTOR_DIR_PIN, LOW); isMovingForward = false
#define     Motor_Nema17_StepHigh()        	digitalWrite(MOTOR_STEP_PIN, HIGH)
#define     Motor_Nema17_StepLow()			digitalWrite(MOTOR_STEP_PIN, LOW);

#if (MOTOR_ACTIVE_28BYJ == 1)
// 28BYJ data
#define 	LINE1 					13		// motor 28BYJ driver input line 1
#define 	LINE2 					12		// motor 28BYJ driver input line 2
#define 	LINE3 					11		// motor 28BYJ driver input line 3
#define 	LINE4 					10		// motor 28BYJ driver input line 4

// 28BYJ commands
Stepper28BYJ motor28BYJ(LINE1, LINE3, LINE2, LINE4);
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
void    Nema17_MoveExactly(uint16_t stepNumber);

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
	// Motor Nema17 setups
	pinMode(MOTOR_ENABLE_PIN, OUTPUT);
	pinMode(MOTOR_DIR_PIN   , OUTPUT);
	pinMode(MOTOR_STEP_PIN  , OUTPUT);
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
 *                        MoveExactly() function
 ***********************************************************************/
void Nema17_MoveExactly(uint16_t stepNumber)
{
    Motor_Nema17_On();
    for(uint16_t s=0; s<stepNumber; s++)
    {
        Motor_Nema17_StepHigh();
        delayMicroseconds(stepSpeed);
        Motor_Nema17_StepLow();
        delayMicroseconds(stepSpeed);
    }
    Motor_Nema17_Off();
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
            Motor_Nema17_On();
            Nema17_State = NEMA17_ACTIVE;

#if (MOTOR_ACTIVE_28BYJ == 1)
            motor28BYJ.TurnOn();
#endif
        }

        if(serialCommand == "off") {
            Motor_Nema17_Off();
            Nema17_State = NEMA17_INACTIVE;

#if (MOTOR_ACTIVE_28BYJ == 1)
            motor28BYJ.TurnOff();
#endif
        }

        if(serialCommand == "spd") {
            Motor_Nema17_SetSpeed( inputString.substring(3).toInt() );
            #if (DEBUG_ON == 1)
            SERIAL(stepSpeed);
            #endif
        }

        if(serialCommand == ">>>") {
            SERIAL("Move to right\n");
            Motor_Nema17_SetDirForward();
        }

        if(serialCommand == "<<<") {
        	SERIAL("Move to left\n");
            Motor_Nema17_SetDirBackward();
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
	static uint8_t endsSteps = 0;

    if( Nema17_State==NEMA17_INACTIVE ) {
        return;
    }

    else if( Nema17_State==NEMA17_END_LEFT ) {
    	Motor_Nema17_Off();
    	TM_START(tm_EndLeft);
    	if(TM_DONE(tm_EndLeft,3000*1000)) {
    		Motor_Nema17_SetDirForward();
    		Motor_Nema17_On();
    		// Nema17  -  M A K E   N E X T   S T E P     -  START
			if( TM_DONE(tm_StepHigh, stepSpeed) ) {
				Motor_Nema17_StepHigh();
				TM_STOP(tm_StepHigh);
				TM_START(tm_StepLow);
			}

			if( TM_DONE(tm_StepLow, stepSpeed) ) {
				Motor_Nema17_StepLow();
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

    else if( Nema17_State==NEMA17_END_RIGHT ) {
    	Motor_Nema17_Off();
		TM_START(tm_EndRight);
		if(TM_DONE(tm_EndRight,3000*1000)) {
			Motor_Nema17_SetDirBackward();
			Motor_Nema17_On();
			// Nema17  -  M A K E   N E X T   S T E P     -  START
			if( TM_DONE(tm_StepHigh, stepSpeed) ) {
				Motor_Nema17_StepHigh();
				TM_STOP(tm_StepHigh);
				TM_START(tm_StepLow);
			}

			if( TM_DONE(tm_StepLow, stepSpeed) ) {
				Motor_Nema17_StepLow();
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

    else {
    	// Nema17  -  M A K E   N E X T   S T E P     -  START
		if( TM_DONE(tm_StepHigh, stepSpeed) ) {
			Motor_Nema17_StepHigh();
			TM_STOP(tm_StepHigh);
			TM_START(tm_StepLow);
		}

		if( TM_DONE(tm_StepLow, stepSpeed) ) {
			Motor_Nema17_StepLow();
			TM_STOP(tm_StepLow);
			TM_START(tm_StepHigh);
		}
		// Nema17  -  M A K E   N E X T   S T E P     -  STOP
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
	Motor_Nema17_On();
	Motor_Nema17_SetDirBackward();
	TM_START(tm_StepHigh);
	TM_STOP(tm_StepLow);
	TM_STOP(tm_EndLeft);
	TM_STOP(tm_EndRight);
	Nema17_State = NEMA17_ACTIVE;
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
