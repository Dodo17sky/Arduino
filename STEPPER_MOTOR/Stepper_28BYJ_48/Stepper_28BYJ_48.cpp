#include <Arduino.h>
#include <Stepper.h>

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String        inputString     = "";             // a String to hold incoming data
boolean       stringComplete  = false;          // whether the string is complete
String        serialCommand;

#define LINE1 9
#define LINE2 4
#define LINE3 3
#define LINE4 2

#define DEFAULT_STEPS_NUMBER	50
#define DEFAULT_STEPS_DELAY 	50
#define DEFAULT_STEPPER_SPEED	250

// stepper 28BYJ enable
bool isStepper28BYJ_Enbaled;

// Number of steps per internal motor revolution
const float STEPS_PER_REV = 32;

//  Amount of Gear Reduction
const float GEAR_RED = 64;

// Number of steps per geared output rotation
//const float STEPS_PER_OUT_REV = STEPS_PER_REV * GEAR_RED;
const float STEPS_PER_OUT_REV = 512;

// Define Variables

// Number of Steps Required
int StepsRequired;

// Delay between steps
int StepsDelay;

// Motor speed
int MotorSpeed;

// Create Instance of Stepper Class
// Specify Pins used for motor coils
// The pins used are 8,9,10,11
// Connected to ULN2003 Motor Driver In1, In2, In3, In4
// Pins entered in sequence 1-3-2-4 for proper step sequencing

Stepper steppermotor(STEPS_PER_REV, LINE1, LINE3, LINE2, LINE4);

// motor process
void Motor_Process();

void setup()
{
	Serial.begin(9600);

	isStepper28BYJ_Enbaled = false;
	StepsRequired = DEFAULT_STEPS_NUMBER;
	StepsDelay = DEFAULT_STEPS_DELAY;
	MotorSpeed = DEFAULT_STEPPER_SPEED;
}

void loop() {

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
			StepsRequired = (-1) * StepsRequired;
		}

		if(serialCommand == "del") {
			StepsDelay = inputString.substring(3).toInt();
		}

		if(serialCommand == "spd") {
			MotorSpeed = inputString.substring(3).toInt();
		}

		stringComplete = false;
		while(Serial.read() >= 0) ; // flush the receive buffer
		inputString = "";
	}

	Motor_Process();

}

void Motor_Process()
{
	if( isStepper28BYJ_Enbaled ) {
		steppermotor.setSpeed(MotorSpeed);
		steppermotor.step(StepsRequired);
		delay(StepsDelay);
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
