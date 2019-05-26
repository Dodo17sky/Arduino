/***********************************************************************
 *                        LIBRARIES
 ***********************************************************************/
#include <Arduino.h>
#include <Stepper.h>

#define		DEBUG_ON                0		// Compiler switch for Serial commands

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
uint32_t	stepSpeed       = 4000;			// delay between steps [us]

// 28BYJ data
int			StepsRequired;					// Number of Steps Required
boolean     is28ByjEnabled;

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
#define     END_DETECTOR_FRONT		9
#define     END_DETECTOR_REAR		8
#define     END_REACHED             LOW

// Nema17 commands
#define     Motor_Nema17_On()       		digitalWrite(MOTOR_ENABLE_PIN, LOW); isNema17Enabled = true
#define     Motor_Nema17_Off()      		digitalWrite(MOTOR_ENABLE_PIN, HIGH); isNema17Enabled = false
#define     Motor_Nema17_IsOn       		(isNema17Enabled == true)
#define     Motor_Nema17_IsOff      		(isNema17Enabled == false)
#define     Motor_Nema17_SetSpeed(x)       	stepSpeed = (x)
#define     Motor_Nema17_SetDirForward()   	digitalWrite(MOTOR_DIR_PIN, HIGH); isMovingForward = true
#define     Motor_Nema17_SetDirBackward()	digitalWrite(MOTOR_DIR_PIN, LOW); isMovingForward = false
#define     Motor_Nema17_StepHigh()        	digitalWrite(MOTOR_STEP_PIN, HIGH)
#define     Motor_Nema17_StepLow()         	digitalWrite(MOTOR_STEP_PIN, LOW)

// 28BYJ data
#define 	LINE1 					13		// motor 28BYJ driver input line 1
#define 	LINE2 					12		// motor 28BYJ driver input line 2
#define 	LINE3 					11		// motor 28BYJ driver input line 3
#define 	LINE4 					10		// motor 28BYJ driver input line 4
#define		STEPS_PER_REV			32		// Number of steps per internal motor revolution
#define 	DEFAULT_STEPS_NUMBER	4
#define 	DEFAULT_STEPPER_SPEED	250

// 28BYJ commands
Stepper 	steppermotor(STEPS_PER_REV, LINE1, LINE3, LINE2, LINE4); // Pins entered in sequence line1-line3-line2-line4 for proper step sequencing
#define		STEPPER_28BYJ_REVERSE()			StepsRequired = (-1) * StepsRequired

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
void 	Motor_Nema17_Process(void);
void    Nema17_MoveExactly(uint16_t stepNumber);
void 	Motor_28BYJ_Process(void);
void 	ReadInputs(void);
void 	GlobalData_Init(void);

/***********************************************************************
 *                        setup() function
 ***********************************************************************/
void setup() {
	GlobalData_Init();

#if (DEBUG_ON == 1)
	Serial.begin(9600);
#endif

	// setup stepper motor pins
	pinMode(MOTOR_ENABLE_PIN, OUTPUT);    // enable pin
	pinMode(MOTOR_DIR_PIN   , OUTPUT);    // step
	pinMode(MOTOR_STEP_PIN  , OUTPUT);    // direction

	// setup ends detectors pins
	pinMode(END_DETECTOR_LEFT , INPUT_PULLUP);
	pinMode(END_DETECTOR_RIGHT, INPUT_PULLUP);

	// Motor 28BYJ setups
	pinMode(END_DETECTOR_FRONT, INPUT_PULLUP);
	pinMode(END_DETECTOR_REAR , INPUT_PULLUP);
	steppermotor.setSpeed(DEFAULT_STEPPER_SPEED);

	delay(3000);

	Motor_Nema17_On();
	Motor_Nema17_SetDirForward();

	TM_START(tm_StepHigh);
}

/***********************************************************************
 *                        loop() function
 ***********************************************************************/
void loop() {
	ReadInputs();
	Motor_Nema17_Process();
	Motor_28BYJ_Process();
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
        }

        if(serialCommand == "off") {
            Motor_Nema17_Off();
        }

        if(serialCommand == "spd") {
            Motor_Nema17_SetSpeed( inputString.substring(3).toInt() );
            #if (DEBUG_ON == 1)
            Serial.print(stepSpeed);
            #endif
        }

        if(serialCommand == ">>>") {
            Serial.println("Move to right");
            Motor_Nema17_SetDirForward();
        }

        if(serialCommand == "<<<") {
            Serial.println("Move to left");
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
    if( Motor_Nema17_IsOff )
        return;

    if( Ends.Left) {
    	SERIAL("Left stop\n");
		Motor_Nema17_Off();
		Motor_Nema17_SetDirForward();
		delay(1000);
		//Nema17_MoveExactly(80);
		Motor_Nema17_On();
	}
    else if( Ends.Right ) {
    	SERIAL("Right stop\n");
		Motor_Nema17_Off();
		Motor_Nema17_SetDirBackward();
		delay(1000);
		//Nema17_MoveExactly(80);
		Motor_Nema17_On();
	}

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

/***********************************************************************
 *                        Motor_28BYJ_Process() function
 ***********************************************************************/
void Motor_28BYJ_Process(void)
{
	if( is28ByjEnabled == false ) {
		return;
	}

	if( Ends.Front )
	{
		SERIAL("Front stop");
		STEPPER_28BYJ_REVERSE();
	}
	else if( Ends.Rear )
	{
		SERIAL("Rear stop");
		STEPPER_28BYJ_REVERSE();
	}

	steppermotor.step(StepsRequired);
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

	// Motor 28BYJ data
	is28ByjEnabled = true;
	StepsRequired = DEFAULT_STEPS_NUMBER;
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
