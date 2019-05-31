/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Stepper.h>

#include "Nema17Motor.h"
#include "Stepper28BYJ.h"
#include "LdrSensor.h"

#define		DEBUG_ON                1		// Compiler switch for Serial commands
#define		MOTOR_ACTIVE_28BYJ	    0		// Compiler switch to deactivate all motor 28BYJ functionality
#define		MOTOR_ACTIVE_NEMA17		1		// Compiler switch to deactivate all motor Nema17 functionality

/***********************************************************************
 *                        GLOBAL TYPES
 ***********************************************************************/
typedef struct {
	uint8_t Left 			: 1;
	uint8_t Right			: 1;
	uint8_t Front			: 1;
	uint8_t Rear			: 1;
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

#define		LDR_PIN_H				A5
#define		LDR_PIN_C				A4
#define		LDR_PIN_V				A3

#define		MANUAL_NEMA_CTRL		A2

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
//    		LDR_DIR_TO_LEFT         -1
//    		LDR_DIR_NO_DIR          0
#define		NEMA17_INACTIVE			((int8_t)(0))
//    		LDR_DIR_TO_RIGHT        1
#define		NEMA17_END_LEFT			((int8_t)(2))
#define		NEMA17_END_RIGHT		((int8_t)(3))
#define		NEMA17_ACTIVE			((int8_t)(4))

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
int8_t		Nema17_State;
uint16_t 	stepSpeed;
uint8_t		endsSteps;
int16_t		manualCmd;

uint32_t	revolutionTime;
uint16_t	revolutionSteps;
uint8_t		isTesting;
#define		TEST_STOP		0
#define		TEST_RUN		1
#define		TEST_END		2
#endif

LdrSensor	ldr(LDR_PIN_H, LDR_PIN_V, LDR_PIN_C);

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
void 	Motor_Nema17_ToLeft();
void 	Motor_Nema17_ToRight();

#if (MOTOR_ACTIVE_28BYJ == 1)
void 	Motor_28BYJ_Process(void);
#endif

/***********************************************************************
 *                        setup() function
 ***********************************************************************/
void setup() {
	GlobalData_Init();
	Nema17.setSpeed(250);

#if (DEBUG_ON == 1)
	Serial.begin(57600);
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

	delay(1000);
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
	int8_t dirToGo = LDR_DIR_NO_DIR;
	ldr.readAll();
	dirToGo = ldr.getDirectionV();

	if( digitalRead(END_DETECTOR_LEFT ) == END_REACHED) {
		Ends.Left = 1;
		Nema17_State = NEMA17_END_LEFT;
		TM_START(tm_EndLeft);
	}
	else {
		Ends.Left = 0;
	}
	if(Ends.Left && dirToGo == LDR_DIR_TO_LEFT) {
		// don't go to left if left end was detected and the LDR want to move in the same direction
		// to avoid commands collision, choose command NEMA17_END_LEFT to reverse the motor
		// and cancel the LDR command
		dirToGo = LDR_DIR_NO_DIR;
	}
	
	if( digitalRead(END_DETECTOR_RIGHT) == END_REACHED) {
		Ends.Right = 1;
		Nema17_State = NEMA17_END_RIGHT;
		TM_START(tm_EndRight);
	}
	else {
		Ends.Right = 0;
	}
	if(Ends.Right && dirToGo == LDR_DIR_TO_RIGHT) {
		// don't go to right if right end was detected and the LDR want to move in the same direction
		// to avoid commands collision, choose command NEMA17_END_RIGHT to reverse the motor
		// and cancel the LDR command
		dirToGo = LDR_DIR_NO_DIR;
	}

	if( Ends.Left == 0 && Ends.Right == 0) {
		// no end was detected
		// Nema17 motor is controlled in this case by LDR command
		Nema17_State = dirToGo;
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
        	SERIAL("Speed = ");
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

        if(serialCommand == "min") {
        	int data = inputString.substring(3).toInt();
			ldr.setMinDifference(data);
		}
        if(serialCommand == "cah") {
			int data = inputString.substring(3).toInt();
			ldr.setCalibrationH(data);
		}
        if(serialCommand == "cav") {
			int data = inputString.substring(3).toInt();
			ldr.setCalibrationV(data);
		}
        if(serialCommand == "cac") {
			int data = inputString.substring(3).toInt();
			ldr.setCalibrationC(data);
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
	static int8_t Nema17_State_old = 0;

	if(
		(Nema17_State == LDR_DIR_TO_LEFT && Nema17_State_old == NEMA17_END_LEFT) ||
		(Nema17_State == LDR_DIR_TO_RIGHT && Nema17_State_old == NEMA17_END_RIGHT)
	  )
	{
		return;
	}

	if(Nema17_State != Nema17_State_old) {
		//SERIAL(String("\n") + String(Nema17_State) + String(": "));
		Nema17_State_old = Nema17_State;
	}

	switch(Nema17_State) {
		case NEMA17_INACTIVE:
			Motor_Nema17_Inactive();
			break;
		case NEMA17_END_LEFT:
			Motor_Nema17_Left();
			//SERIAL(".");
			break;
		case NEMA17_END_RIGHT:
			Motor_Nema17_Right();
			//SERIAL(".");
			break;
		case LDR_DIR_TO_LEFT:
			Motor_Nema17_ToLeft();
			//SERIAL(".");
			break;
		case LDR_DIR_TO_RIGHT:
			Motor_Nema17_ToRight();
			//SERIAL(".");
			break;
		case NEMA17_ACTIVE:
			Motor_Nema17_Active();
			//SERIAL("^");
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
	Nema17.turnOff();
}

/***********************************************************************
 *                        Motor_Nema17_Active() function
 ***********************************************************************/
void	Motor_Nema17_Active()
{
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
	if(TM_DONE(tm_EndLeft,1000000)) {
		Nema17.goForward();
		Nema17.turnOn();

		if( isTesting != TEST_RUN ){
			isTesting = TEST_RUN;
			revolutionTime = millis();
			revolutionSteps = 0;
			Nema17.stepMeasureStart();
		}

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
			//Nema17_State = NEMA17_ACTIVE;
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

	if( isTesting == TEST_RUN ){
		isTesting = TEST_STOP;
		revolutionTime = millis() - revolutionTime;
		revolutionSteps = Nema17.getStepMeasurement();
		Nema17.stepMeasureStop();
		SERIAL("Measurement -------------------\n");
		SERIAL(String("Steps: ") + String(revolutionSteps) );
		SERIAL(String("\nMilis: ") + String(revolutionTime) );
		uint32_t spd = (1000UL * (uint32_t)revolutionSteps) / (uint32_t)revolutionTime;
		SERIAL(String("\nSpeed: ") + String(spd) );
	}

	TM_START(tm_EndRight);
	if(TM_DONE(tm_EndRight,1000000)) {

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
			//Nema17_State = NEMA17_ACTIVE;
			TM_STOP(tm_EndRight);
		}
	}
}

/***********************************************************************
 *                        Motor_Nema17_ToLeft() function
 ***********************************************************************/
void 	Motor_Nema17_ToLeft()
{
	Nema17.turnOff();
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
	}
	// Nema17  -  M A K E   N E X T   S T E P     -  STOP
}

/***********************************************************************
 *                        Motor_Nema17_ToRight() function
 ***********************************************************************/
void 	Motor_Nema17_ToRight()
{
	Nema17.turnOff();
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
	}
	// Nema17  -  M A K E   N E X T   S T E P     -  STOP
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
		motor28BYJ.GoBackward();
		while( digitalRead(END_DETECTOR_FRONT) == END_REACHED ) {
			// turn back until no end detected
			motor28BYJ.stepExactly(20);
		}
		return;
	}
	else if( Ends.Rear )
	{
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
	Nema17_State = NEMA17_INACTIVE;
	endsSteps = 0;

	revolutionTime = 0;
	revolutionSteps = 0;
	isTesting = TEST_STOP;
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
