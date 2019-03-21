#include <Stepper.h>

int mDelay = 3;
const long GEAR_PER_REV = 32;
const long GEAR_RED = 64;
const long STEPS_PER_OUT_REV = GEAR_PER_REV * GEAR_RED;
int StepsRequired = 0; 

#define LINE1 8
#define LINE2 9
#define LINE3 10
#define LINE4 11

void UseCustomMove();

Stepper myStepper(GEAR_PER_REV, LINE1, LINE3, LINE2, LINE4);

void setup() {
  Serial.begin(9600);
}

void loop() {
    UseCustomMove();
}

void UseCustomMove()
{
  unsigned long ts = 0;
  mDelay = 5;
  int mSteps = 200;

  ts = millis();
  for(int i=0; i<mSteps; i++) {
    delay(mDelay);
    digitalWrite(LINE1, HIGH);
    digitalWrite(LINE2, HIGH);
    digitalWrite(LINE3, LOW);
    digitalWrite(LINE4, LOW);
  
    delay(mDelay);
    digitalWrite(LINE1, LOW);
    digitalWrite(LINE2, HIGH);
    digitalWrite(LINE3, HIGH);
    digitalWrite(LINE4, LOW);
  
    delay(mDelay);
    digitalWrite(LINE1, LOW);
    digitalWrite(LINE2, LOW);
    digitalWrite(LINE3, HIGH);
    digitalWrite(LINE4, HIGH);
  
    delay(mDelay);
    digitalWrite(LINE1, HIGH);
    digitalWrite(LINE2, LOW);
    digitalWrite(LINE3, LOW);
    digitalWrite(LINE4, HIGH);
  }
  ts = millis() - ts;
  Serial.println(String("Full step forward ") + ts);
  delay(2000);

  ts = millis();
  for(int i=0; i<mSteps; i++) {
    delay(mDelay);
    digitalWrite(LINE1, HIGH);
    digitalWrite(LINE2, LOW);
    digitalWrite(LINE3, LOW);
    digitalWrite(LINE4, HIGH);

    delay(mDelay);
    digitalWrite(LINE1, LOW);
    digitalWrite(LINE2, LOW);
    digitalWrite(LINE3, HIGH);
    digitalWrite(LINE4, HIGH);
      
    delay(mDelay);
    digitalWrite(LINE1, LOW);
    digitalWrite(LINE2, HIGH);
    digitalWrite(LINE3, HIGH);
    digitalWrite(LINE4, LOW);
    
    delay(mDelay);
    digitalWrite(LINE1, HIGH);
    digitalWrite(LINE2, HIGH);
    digitalWrite(LINE3, LOW);
    digitalWrite(LINE4, LOW);
  }
  ts = millis() - ts;
  Serial.println(String("Full step backward ") + ts);
  delay(2000);
}

void UseLibrary()
{
    myStepper.setSpeed(1);
    StepsRequired = 4;
    myStepper.step(StepsRequired);
    delay(2000);
  
    StepsRequired = STEPS_PER_OUT_REV;
    myStepper.setSpeed(100);
    myStepper.step(StepsRequired);
    delay(1000);
  
    StepsRequired = -STEPS_PER_OUT_REV;
    myStepper.setSpeed(700);
    myStepper.step(StepsRequired);
    delay(2000);
}

