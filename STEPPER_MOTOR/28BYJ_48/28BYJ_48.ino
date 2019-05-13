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

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, LINE1, LINE2, LINE3, LINE4);

int stepCount = 0;  // number of steps the motor has taken
int speedMotor = 25;
int stepsCount = 100;
bool isOff = false;

void setup() {
  Serial.begin(9600);
  analogWrite(10,115);
}

void loop() {

  if (stringComplete) {
        serialCommand = inputString.substring(0,3);   // the first 3 characters define command type
        Serial.print(String("\nCmd: ") + serialCommand + " ");

        if(serialCommand == "rev") {
            //stepsCount = (-1) * stepsCount;
            speedMotor = (-1) * speedMotor;
            Serial.print("turn back ");
            Serial.println(speedMotor);
        }

        if(serialCommand == "stp") {
            isOff = !isOff;
            Serial.print("State ");
            Serial.print(isOff);
        }

        if(serialCommand == "spd") {
            speedMotor = inputString.substring(3).toInt();
            Serial.print(speedMotor);
        }

        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }
    
  if (speedMotor > 0 && isOff) {
    myStepper.setSpeed(speedMotor);
    myStepper.step(-100);
    delay(2000);
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


