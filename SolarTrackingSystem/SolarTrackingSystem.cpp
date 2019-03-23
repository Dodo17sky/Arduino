#include <Arduino.h>
#include "MicroTimer.h"

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String        inputString     = "";             // a String to hold incoming data
boolean       stringComplete  = false;          // whether the string is complete
boolean       isEnabled       = true;
boolean       isMovingForward = true;           // the motor moving direction
String        serialCommand;
unsigned long Index;

unsigned long stepSpeed       = 1000;           // delay between steps [us]
unsigned long cycleDelay      = 1000;           // delay between directions [ms]
int           stepTarget      = 400;
int           stepCounter     = 400;

MicroTimer	  utMotorStepHigh;
MicroTimer	  utMotorStepLow;

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
 #define      DEBUG_ON                1
 #define      MOTOR_ENABLE_PIN        7
 #define      MOTOR_STEP_PIN          6
 #define      MOTOR_DIR_PIN           5

 #define      Motor_On()              digitalWrite(MOTOR_ENABLE_PIN, LOW); isEnabled = true
 #define      Motor_Off()             digitalWrite(MOTOR_ENABLE_PIN, HIGH); isEnabled = false
 #define      Motor_IsOn              (isEnabled == true)
 #define      Motor_IsOff             (isEnabled == false)

 #define      Motor_SetSpeed(x)       stepSpeed = x
 #define      Motor_SetSteps(x)       stepTarget = x

 #define      Motor_SetDirForward()   digitalWrite(MOTOR_DIR_PIN, HIGH); isMovingForward = true
 #define      Motor_SetDirBackward()  digitalWrite(MOTOR_DIR_PIN, LOW); isMovingForward = false

 #define      Motor_StepHigh()        digitalWrite(MOTOR_STEP_PIN, HIGH)
 #define      Motor_StepLow()         digitalWrite(MOTOR_STEP_PIN, LOW)

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
 uint8_t Serial_Process(void);
 uint8_t Motor_Process(void);

 /***********************************************************************
  *                        setup() function
  ***********************************************************************/
 void setup() {
  inputString.reserve(200);             // reserve 200 bytes for the inputString:

  pinMode(MOTOR_ENABLE_PIN, OUTPUT);    // enable pin
  pinMode(MOTOR_DIR_PIN   , OUTPUT);    // step
  pinMode(MOTOR_STEP_PIN  , OUTPUT);    // direction

  Serial.begin(9600);
  Motor_On();
  Motor_SetDirForward();

  // set motor speed => controlled by these two timers
  utMotorStepHigh.setPeriod(stepSpeed);
  utMotorStepLow.setPeriod(stepSpeed);

  // start high phase timer
  utMotorStepHigh.restart();  // start() > you set a specific period; restart() > used already set period
}

 /***********************************************************************
  *                        loop() function
  ***********************************************************************/
void loop() {
    Serial_Process();
    Motor_Process();
}

/***********************************************************************
 *                        Serial_Process()
 ***********************************************************************/
uint8_t Serial_Process(void)
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
            // set motor speed => controlled by these two timers
			utMotorStepHigh.setPeriod(stepSpeed);
			utMotorStepLow.setPeriod(stepSpeed);
            #if (DEBUG_ON == 1)
            Serial.print(stepSpeed);
            #endif
        }

        if(serialCommand == ">>>") {
            Motor_SetDirForward();
        }

        if(serialCommand == "<<<") {
            Motor_SetDirBackward();
        }

        if(serialCommand == "nmb") {
            Motor_SetSteps( inputString.substring(3).toInt() );
            #if (DEBUG_ON == 1)
            Serial.print(stepTarget);
            #endif
        }

        if(serialCommand == "del") {
            String tmp = inputString.substring(3);
            cycleDelay = tmp.toInt();
            Serial.print(cycleDelay);
        }

        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }
    return 0;
}

/***********************************************************************
 *                        Motor_Process()
 ***********************************************************************/
uint8_t Motor_Process(void)
{
    if( Motor_IsOff )
        return 0;

    if( utMotorStepHigh.done() ) {
        Motor_StepHigh();
        utMotorStepHigh.stop();
        utMotorStepLow.restart();
    }

    if( utMotorStepLow.done() ) {
        Motor_StepLow();
        utMotorStepHigh.restart();
        utMotorStepLow.stop();
        stepCounter ++;
    }

    if ( (cycleDelay > 0) && (stepCounter >= stepTarget) ) {
        stepCounter = 0;
        delay(cycleDelay);
    }
    return 0;
}

/***********************************************************************
 *                        serialEvent()
 ***********************************************************************/
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
