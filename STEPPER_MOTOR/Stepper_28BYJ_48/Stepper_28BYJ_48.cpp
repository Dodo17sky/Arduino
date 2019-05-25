#include <Arduino.h>
#include <Stepper.h>

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String        inputString     = "";             // a String to hold incoming data
boolean       stringComplete  = false;          // whether the string is complete
String        serialCommand;

#define 	LINE1 					13
#define 	LINE2 					12
#define 	LINE3 					11
#define 	LINE4 					10

#define 	DEFAULT_STEPS_NUMBER	4
#define 	DEFAULT_STEPS_DELAY 	50
#define 	DEFAULT_STEPPER_SPEED	250

#define 	ENDS_H_FRONT			9
#define 	ENDS_H_REAR				8
#define     END_REACHED             LOW

// stepper 28BYJ enable
bool isStepper28BYJ_Enbaled;

// Number of steps per internal motor revolution
const float STEPS_PER_REV = 32;

//  Amount of Gear Reduction
const float GEAR_RED = 64;

// Number of steps per geared output rotation
//const float STEPS_PER_OUT_REV = STEPS_PER_REV * GEAR_RED;
const float STEPS_PER_OUT_REV = 512;

// Create Instance of Stepper Class
// Specify Pins used for motor coils
// The pins used are 8,9,10,11
// Connected to ULN2003 Motor Driver In1, In2, In3, In4
// Pins entered in sequence 1-3-2-4 for proper step sequencing

Stepper steppermotor(STEPS_PER_REV, LINE1, LINE3, LINE2, LINE4);

// Number of Steps Required
int StepsRequired;

// reverse direction macro
#define STEPPER_28BYJ_REVERSE()		StepsRequired = (-1) * StepsRequired
// move exactly a number of steps
#define	STEPPER_28BYJ_MOVE(nmb)		steppermotor.step((nmb)*StepsRequired)

// Delay between steps
int StepsDelay;

// Motor speed
int MotorSpeed;
int tmpMotorSpeed;

// motor process
void Motor_Process();
void ReadInputs();
void Serial_Process();

void setup()
{
	Serial.begin(9600);

	isStepper28BYJ_Enbaled = true;
	StepsRequired = DEFAULT_STEPS_NUMBER;
	StepsDelay = DEFAULT_STEPS_DELAY;
	MotorSpeed = DEFAULT_STEPPER_SPEED;

	pinMode(ENDS_H_FRONT, INPUT_PULLUP);
	pinMode(ENDS_H_REAR , INPUT_PULLUP);

	steppermotor.setSpeed(MotorSpeed);
	delay(3000);
}

void loop() {

	Serial_Process();
	Motor_Process();
	ReadInputs();
}

void Serial_Process()
{
	if (stringComplete) {
		serialCommand = inputString.substring(0,3);   // the first 3 characters define command type
		Serial.print(String("\nCmd: ") + serialCommand + " ");

		if(serialCommand == "onn") {
			isStepper28BYJ_Enbaled = true;
		}
		if(serialCommand == "off") {
			isStepper28BYJ_Enbaled = false;
		}

		if(serialCommand == "stp") {
			StepsRequired = inputString.substring(3).toInt();
		}

		if(serialCommand == "rev") {
			STEPPER_28BYJ_REVERSE();
		}

		if(serialCommand == "del") {
			StepsDelay = inputString.substring(3).toInt();
		}

		if(serialCommand == "spd") {
			int tmpMotorSpeed;
			tmpMotorSpeed = inputString.substring(3).toInt();
			if(tmpMotorSpeed != MotorSpeed) {
				MotorSpeed = tmpMotorSpeed;
				steppermotor.setSpeed(MotorSpeed);
			}
		}

		stringComplete = false;
		while(Serial.read() >= 0) ; // flush the receive buffer
		inputString = "";
	}
}

void Motor_Process()
{
	if( isStepper28BYJ_Enbaled ) {
		steppermotor.step(StepsRequired);
	}
}

void ReadInputs()
{
	if( digitalRead(ENDS_H_FRONT) == LOW)
	{
		Serial.println("Front stop");
		STEPPER_28BYJ_REVERSE();
		STEPPER_28BYJ_MOVE(70);
	}

	if( digitalRead(ENDS_H_REAR) == LOW)
	{
		Serial.println("Rear stop");
		STEPPER_28BYJ_REVERSE();
		STEPPER_28BYJ_MOVE(70);
	}
}

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
