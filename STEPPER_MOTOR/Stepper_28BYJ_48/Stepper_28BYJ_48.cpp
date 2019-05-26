/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Stepper.h>
#include "Motor28BYJ.h"

#define		DEBUG_ON                1		// Compiler switch for Serial commands
#define		MOTOR_ACTIVE_28BYJ	    1		// Compiler switch to deactivate all motor 28BYJ functionality
#define		MOTOR_ACTIVE_NEMA17		0		// Compiler switch to deactivate all motor Nema17 functionality

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

EndDetector Ends;

// 28BYJ data
#if (MOTOR_ACTIVE_28BYJ == 1)
int			StepsRequired;					// Number of Steps Required
boolean     is28ByjEnabled;
#endif

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#if (DEBUG_ON == 1)
#define		SERIAL(x)				Serial.print(x)
#else
#define		SERIAL(x)
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
#define     END_DETECTOR_FRONT		9
#define     END_DETECTOR_REAR		8
#endif

#define     END_REACHED             LOW

#if (MOTOR_ACTIVE_28BYJ == 1)
// 28BYJ data
#define 	LINE1 					13		// motor 28BYJ driver input line 1
#define 	LINE2 					12		// motor 28BYJ driver input line 2
#define 	LINE3 					11		// motor 28BYJ driver input line 3
#define 	LINE4 					10		// motor 28BYJ driver input line 4
#define		STEPS_PER_REV			32		// Number of steps per internal motor revolution
#define 	DEFAULT_STEPS_NUMBER	4
#define 	DEFAULT_STEPPER_SPEED	200

// 28BYJ commands
Stepper 	steppermotor(STEPS_PER_REV, LINE1, LINE3, LINE2, LINE4); // Pins entered in sequence line1-line3-line2-line4 for proper step sequencing
Motor28BYJ  smallStepper(LINE1, LINE3, LINE2, LINE4);
#define		Motor_28BYJ_Off()				smallStepper.TurnOff()
#define		Motor_28BYJ_On()				smallStepper.TurnOn()
#endif

/***********************************************************************
 *                        CUSTOM TIMERS
 ***********************************************************************/
uint32_t	tm_StepHigh      		= 0;
uint32_t	tm_StepLow       		= 0;

#define		TM_STOP_VALUE           0xFFFFFFFF
#define		TM_START(x)             x = micros()
#define		TM_STOP(x)              x = TM_STOP_VALUE
#define		TM_DONE(x,t)            ( (x != TM_STOP_VALUE) && (micros() >= (x+t)) )

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
#if (DEBUG_ON == 1)
void 	Serial_Process(void);
#endif

void 	ReadInputs(void);
void 	GlobalData_Init(void);

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

#if (MOTOR_ACTIVE_28BYJ == 1)
	// Motor 28BYJ setups
	pinMode(END_DETECTOR_FRONT, INPUT_PULLUP);
	pinMode(END_DETECTOR_REAR , INPUT_PULLUP);
	steppermotor.setSpeed(DEFAULT_STEPPER_SPEED);
	smallStepper.setSpeed(10000);
#endif

	delay(3000);
}

/***********************************************************************
 *                        loop() function
 ***********************************************************************/
void loop() {
	ReadInputs();

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
#if (MOTOR_ACTIVE_28BYJ == 1)
            Motor_28BYJ_On();
#endif
        }

        if(serialCommand == "off") {
#if (MOTOR_ACTIVE_28BYJ == 1)
            Motor_28BYJ_Off();
#endif
        }

        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }
}
#endif

/***********************************************************************
 *                        Motor_28BYJ_Process() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_28BYJ == 1)
void Motor_28BYJ_Process(void)
{
	if( smallStepper.isOn() == false ) {
		return;
	}

	if( Ends.Front )
	{
		Ends.Front = 0;
		SERIAL("Front stop");
		smallStepper.GoBackward();
		while( digitalRead(END_DETECTOR_FRONT) == END_REACHED ) {
			// turn back until no end detected
			smallStepper.stepExactly(40);
		}
		return;
	}
	else if( Ends.Rear )
	{
		Ends.Rear = 0;
		SERIAL("Rear stop");
		smallStepper.GoForward();
		while( digitalRead(END_DETECTOR_REAR) == END_REACHED ) {
			// turn back until no end detected
			smallStepper.stepExactly(40);
		}
		return;
	}

	smallStepper.step();
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

#if (MOTOR_ACTIVE_28BYJ == 1)
	// Motor 28BYJ data
	Motor_28BYJ_On();
	StepsRequired = DEFAULT_STEPS_NUMBER;
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
