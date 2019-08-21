/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Stepper.h>

#include "Nema17Motor.h"
#include "Stepper28BYJ.h"
#include "LdrSensor.h"

#define		DEBUG_ON                0		// Compiler switch for Serial commands
#define		MOTOR_ACTIVE_28BYJ	    1		// Compiler switch to deactivate all motor 28BYJ functionality
#define		MOTOR_ACTIVE_NEMA17		1		// Compiler switch to deactivate all motor Nema17 functionality

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

#define		MOTOR28BYJ_END_DOWN		((int8_t)(-2))
#define		NEMA17_END_LEFT			((int8_t)(-2))
//    		LDR_DIR_TO_DOWN         -1
//    		LDR_DIR_TO_LEFT         -1
//    		LDR_DIR_NO_DIR          0
#define		MOTOR_INACTIVE			((int8_t)(0))
//    		LDR_DIR_TO_RIGHT        1
//    		LDR_DIR_TO_UP           1
#define		MOTOR28BYJ_END_UP		((int8_t)(2))
#define		NEMA17_END_RIGHT		((int8_t)(2))

#if (MOTOR_ACTIVE_28BYJ == 1)
// 28BYJ data
#define 	LINE1 					13		// motor 28BYJ driver input line 1
#define 	LINE2 					12		// motor 28BYJ driver input line 2
#define 	LINE3 					11		// motor 28BYJ driver input line 3
#define 	LINE4 					10		// motor 28BYJ driver input line 4

// 28BYJ commands
Stepper28BYJ motor28BYJ(LINE1, LINE3, LINE2, LINE4);
int8_t		Motor28BYJ_State;
int8_t		Motor28BYJ_State_Old;
#endif

#if (MOTOR_ACTIVE_NEMA17 == 1)
// Nema17 data
Nema17Motor Nema17(MOTOR_DIR_PIN, MOTOR_STEP_PIN, MOTOR_ENABLE_PIN);
int8_t		Nema17_State;
int8_t		Nema17_State_old;
uint16_t 	stepSpeed;
int16_t		manualCmd;

#endif

LdrSensor	ldr(LDR_PIN_H, LDR_PIN_V, LDR_PIN_C);

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
#if (DEBUG_ON == 1)
void 	Serial_Process(void);
#endif

void 	ReadInputs(void);
void 	GlobalData_Init(void);

#if (MOTOR_ACTIVE_NEMA17 == 1)
void 	ReadInputs_Nema(void);
void 	Motor_Nema17_Process(void);
void	Motor_Nema17_Inactive();
void	Motor_Nema17_Left();
void	Motor_Nema17_Right();
void 	Motor_Nema17_ToLeft();
void 	Motor_Nema17_ToRight();
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
void 	ReadInputs_28BYJ(void);
void 	Motor_28BYJ_Process(void);
void	Motor_28BYJ_Down();
void	Motor_28BYJ_Up();
void	Motor_28BYJ_ToDown();
void	Motor_28BYJ_ToUp();
#endif

/***********************************************************************
 *                        setup() function
 ***********************************************************************/
void setup() {
	GlobalData_Init();

#if (DEBUG_ON == 1)
	Serial.begin(115200);
#endif

#if (MOTOR_ACTIVE_NEMA17 == 1)
	// setup ends detectors pins
	pinMode(END_DETECTOR_LEFT , INPUT_PULLUP);
	pinMode(END_DETECTOR_RIGHT, INPUT_PULLUP);
	Nema17.setSpeed(250);
	stepSpeed = Nema17.getSpeed();
	Nema17_State_old = -9; // just an invalid value
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
	ldr.readAll();

#if (MOTOR_ACTIVE_NEMA17 == 1)
	if (Nema17.isFreeToMove() ) {
		// motor is free to move only when it doesn't reach the ends
		// when it reach the ends, is blocked in order to move a fix number of steps,
		// so is no need to read the inputs
		ReadInputs_Nema();
	}
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
	ReadInputs_28BYJ();
#endif
}

#if (MOTOR_ACTIVE_NEMA17 == 1)
void ReadInputs_Nema(void)
{
	static int8_t last_Nema17_State = 0;

	Nema17_State = ldr.getDirectionV();

	if( digitalRead(END_DETECTOR_LEFT ) == END_REACHED) {
		Nema17_State = NEMA17_END_LEFT;
	}
	
	if( digitalRead(END_DETECTOR_RIGHT) == END_REACHED) {
		Nema17_State = NEMA17_END_RIGHT;
	}
}
#endif

#if (MOTOR_ACTIVE_28BYJ == 1)
void ReadInputs_28BYJ(void)
{
	Motor28BYJ_State = ldr.getDirectionH();

	if( digitalRead(END_DETECTOR_FRONT ) == END_REACHED) {
		Motor28BYJ_State = MOTOR28BYJ_END_DOWN;
	}

	if( digitalRead(END_DETECTOR_REAR) == END_REACHED) {
		Motor28BYJ_State = MOTOR28BYJ_END_UP;
	}
}
#endif

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
#if (MOTOR_ACTIVE_NEMA17 == 1)
        	Nema17.turnOn();
#endif
#if (MOTOR_ACTIVE_28BYJ == 1)
            motor28BYJ.TurnOn();
#endif
        }

        if(serialCommand == "off") {
#if (MOTOR_ACTIVE_NEMA17 == 1)
            Nema17.turnOff();
            Nema17_State = MOTOR_INACTIVE;
#endif
#if (MOTOR_ACTIVE_28BYJ == 1)
            motor28BYJ.TurnOff();
#endif
        }

        if(serialCommand == "spd") {
        	int tmpSpeed = inputString.substring(3).toInt();
#if (MOTOR_ACTIVE_NEMA17 == 1)
        	Nema17.setSpeed(tmpSpeed);
#endif
            #if (DEBUG_ON == 1)
        	SERIAL("Speed = ");
            SERIAL(tmpSpeed);
            #endif
        }

#if (MOTOR_ACTIVE_NEMA17 == 1)
        if(serialCommand == ">>>") {
            SERIAL("Move to right\n");
            Nema17.goForward();
        }
        if(serialCommand == "<<<") {
        	SERIAL("Move to left\n");
            Nema17.goBackward();
        }
#endif
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
#if (MOTOR_ACTIVE_NEMA17 == 1)
void Motor_Nema17_Process(void)
{
	if(
		// ignore "to left" command if the previous command was "end left"
		// because you can't go left once you are already left most
		(Nema17_State == LDR_DIR_TO_LEFT && Nema17_State_old == NEMA17_END_LEFT) ||
		// ignore "to right" command if the previous command was "end right"
		// because you can't go right once you are already right most
		(Nema17_State == LDR_DIR_TO_RIGHT && Nema17_State_old == NEMA17_END_RIGHT)
	  )
	{
		return;
	}

	if(Nema17_State != Nema17_State_old) {
		Nema17_State_old = Nema17_State;
	}

	switch(Nema17_State) {
		case MOTOR_INACTIVE:
			Motor_Nema17_Inactive();
			break;
		case NEMA17_END_LEFT:
			Motor_Nema17_Left();
			break;
		case NEMA17_END_RIGHT:
			Motor_Nema17_Right();
			break;
		case LDR_DIR_TO_LEFT:
			Motor_Nema17_ToLeft();
			break;
		case LDR_DIR_TO_RIGHT:
			Motor_Nema17_ToRight();
			break;
	}
}
#endif

/***********************************************************************
 *                        Motor_Nema17_Inactive() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_NEMA17 == 1)
void	Motor_Nema17_Inactive()
{
	Nema17.turnOff();
}
#endif

/***********************************************************************
 *                        Motor_Nema17_Left() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_NEMA17 == 1)
void	Motor_Nema17_Left()
{
	Nema17.turnOff();
	Nema17.goForward();
	Nema17.turnOn();
	Nema17.move(40);
}
#endif

/***********************************************************************
 *                        Motor_Nema17_Right() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_NEMA17 == 1)
void	Motor_Nema17_Right()
{
	Nema17.turnOff();
	Nema17.goBackward();
	Nema17.turnOn();
	Nema17.move(40);
}
#endif

/***********************************************************************
 *                        Motor_Nema17_ToLeft() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_NEMA17 == 1)
void 	Motor_Nema17_ToLeft()
{
	Nema17.goBackward();
	Nema17.turnOn();
	Nema17.step();
}
#endif

/***********************************************************************
 *                        Motor_Nema17_ToRight() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_NEMA17 == 1)
void 	Motor_Nema17_ToRight()
{
	Nema17.goForward();
	Nema17.turnOn();
	Nema17.step();
}
#endif

/***********************************************************************
 *                        Motor_28BYJ_Process() function
 ***********************************************************************/
#if (MOTOR_ACTIVE_28BYJ == 1)
void Motor_28BYJ_Process(void)
{
	if(
		// ignore "to down" command if the previous command was "end down"
		// because you can't go down once you are already down
		((Motor28BYJ_State_Old == MOTOR28BYJ_END_DOWN) && (Motor28BYJ_State == LDR_DIR_TO_DOWN)) ||
		// ignore "to up" command if the previous command was "end up"
		// because you can't go up once you are already up
		((Motor28BYJ_State_Old == MOTOR28BYJ_END_UP) && (Motor28BYJ_State == LDR_DIR_TO_UP))
	  )
	{
		return;
	}

	if( motor28BYJ.isOff() || (Motor28BYJ_State == MOTOR_INACTIVE) ) {
		return;
	}

	if( Motor28BYJ_State != Motor28BYJ_State_Old ){
		Motor28BYJ_State_Old = Motor28BYJ_State;
	}

	switch(Motor28BYJ_State){
		case MOTOR28BYJ_END_DOWN:
			Motor_28BYJ_Down();
			break;
		case MOTOR28BYJ_END_UP:
			Motor_28BYJ_Up();
			break;
		case LDR_DIR_TO_DOWN:
			Motor_28BYJ_ToDown();
			break;
		case LDR_DIR_TO_UP:
			Motor_28BYJ_ToUp();
			break;
	}
}

void Motor_28BYJ_Down()
{
	motor28BYJ.GoBackward();
	while( digitalRead(END_DETECTOR_FRONT) == END_REACHED ) {
		// turn back until no end detected
		motor28BYJ.step();
	}
}

void Motor_28BYJ_Up()
{
	motor28BYJ.GoForward();
	while( digitalRead(END_DETECTOR_REAR) == END_REACHED ) {
		motor28BYJ.step();
	}
}

void Motor_28BYJ_ToDown()
{
	motor28BYJ.GoForward();
	motor28BYJ.step();
}
void Motor_28BYJ_ToUp()
{
	motor28BYJ.GoBackward();
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

#if (MOTOR_ACTIVE_NEMA17 == 1)
	// Motor Nema17 data
	Nema17.turnOn();
	Nema17.goBackward();
	Nema17_State = MOTOR_INACTIVE;
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
