#include <Servo.h>

// global debug switcher
#define DEBUG 1

// Pins where LDR sensors are connected
#define sensorPinR        A0
#define sensorPinL        A1

// Pins where LED are connected
#define outLedLeft        2
#define outLedRight       4

// Pin that controls servo motor
#define servoControlPin   3
#define servoWriteDelay   100
#define servoWriteSteps   30

// keep track how many times loop() function process something
unsigned long LoopCycle     = 0;
unsigned long LoopCyclePrev = 0;
#define START_CYCLE()   if(LoopCycle == LoopCyclePrev)  LoopCycle++;
#define STOP_CYCLE()    if(LoopCycle > LoopCyclePrev)   LoopCyclePrev = LoopCycle;
#define LOG_CYCLE()     Serial.print(String("") + LoopCycle + ": ")

// object that controls servo motor for axe 1 - horizontal movement
Servo servoAxe1;

// timer that keeps LEDs on for a certain period after update
unsigned long LedSwitchTimer;
#define LED_SWITCH_TIMEOUT  700
#define IS_TIMER_ELAPSED(timer,period)  (millis() >= (timer+period))

// percentage LDR
int percentLastL = 0;  // variable to store the value coming from the sensor 1
int percentLastR = 0;  // variable to store the value coming from the sensor 2
int percentL = 0;
int percentR = 0;
#define deltaLightIntensity 2

unsigned int LdrChanges = 0;

// servo motor data
unsigned char servoPosition     = 0;
unsigned char servoPositionPrev = 0;
#define SERVO_MIN_LDR_DIFF  3   // eg: one LDR is 52% and the other one is 47%
#define SERVO_MIN_POS_LOG   10

#define CHECK_TASK_PERIOD(taskTimer,taskPeriod)     \
  if (millis() < (taskTimer+taskPeriod))  return;   \
  else  {                                           \
    taskTimer = millis();                           \
    START_CYCLE();                                  \
  }

#define TASK_20_MS        20
#define TASK_25_MS        25
#define TASK_50_MS        50
#define TASK_100_MS       100
#define TASK_200_MS       200

// ======================== Task list =====================
void Task01_ReadLdrSensors();
void Task02_CompareLdrVariation();
void Task03_LedProcess();
void Task04_ServoProcess();

unsigned long TaskTimer_ReadLdrSensors      = 0;
unsigned long TaskTimer_CompareLdrVariation = 0;
unsigned long TaskTimer_LedProcess          = 0;
unsigned long TaskTimer_ServoProcess        = 0;


void setup() {  
  Serial.begin(9600);
  pinMode(outLedLeft ,OUTPUT);
  pinMode(outLedRight,OUTPUT);
  servoAxe1.attach(servoControlPin);
}

void loop() {
  // task 1: read LDR sensors
  Task01_ReadLdrSensors();

  // task 2: compare new LDR sensors values
  Task02_CompareLdrVariation();

  // task 3: process LED commands
  Task03_LedProcess();

  // task 4: process servo commands
  Task04_ServoProcess();

  STOP_CYCLE();
}

void Task01_ReadLdrSensors()
{
    CHECK_TASK_PERIOD(TaskTimer_ReadLdrSensors, TASK_25_MS);

    unsigned long sensorIntensityL = 0;
    unsigned long sensorIntensityR = 0;
  
    sensorIntensityL = analogRead(sensorPinL);
    sensorIntensityR = analogRead(sensorPinR);
  
    percentL = (sensorIntensityL*100)/(sensorIntensityL+sensorIntensityR);
    percentR = (sensorIntensityR*100)/(sensorIntensityL+sensorIntensityR);

    #if 0 //(DEBUG == 1)
    LOG_CYCLE();
    String info = String("Light: ") + sensorIntensityL + " - " +  sensorIntensityR;
    Serial.println(info);
    #endif
}

void Task02_CompareLdrVariation()
{
    CHECK_TASK_PERIOD(TaskTimer_CompareLdrVariation, TASK_25_MS);

    // the light change sensitivity can be low or high
    // low: detect all small changes
    // high: detect bigger changes only
    // this can be done by verify delta difference between current intensity and the latest one
    // Delta can be between 1-6 (if is bigger then 6, then will be very hard to detect any chnage)

    bool isDiffL = abs(percentLastL-percentL) >= deltaLightIntensity;
    bool isDiffR = abs(percentLastR-percentR) >= deltaLightIntensity;

    if( isDiffL || isDiffR )
    {
      LdrChanges++; // count how many times LDR changed
      // difference is bigger then deltaLightIntensity
      #if (DEBUG == 1)
      LOG_CYCLE();
      String info = String("L: ") + percentLastL + ">>" +  percentL
      + " R: " + percentLastR + ">>" +  percentR;
      Serial.println(info);
      #endif

      percentLastL = percentL;
      percentLastR = percentR;
    }
}

void Task03_LedProcess()
{
    CHECK_TASK_PERIOD(TaskTimer_LedProcess, TASK_100_MS);

    if( LdrChanges > 0 )
    {
        LdrChanges = 0;
        if(percentL > percentR) {
            digitalWrite(outLedRight,HIGH);
            digitalWrite(outLedLeft,LOW);
        }
        else {
            digitalWrite(outLedLeft,HIGH);
            digitalWrite(outLedRight,LOW);
        }
        LedSwitchTimer = millis();

        #if (DEBUG == 1)
        LOG_CYCLE();
        String info = String("Led: ") + (percentL > percentR) + " - " +  (percentL < percentR);
        Serial.println(info);
        #endif
    }
    
    if( IS_TIMER_ELAPSED(LedSwitchTimer, LED_SWITCH_TIMEOUT) ) {
        digitalWrite(outLedLeft ,HIGH);
        digitalWrite(outLedRight,HIGH);
    }
}

void Task04_ServoProcess()
{
    CHECK_TASK_PERIOD(TaskTimer_ServoProcess, TASK_100_MS);

    // go forward
    if (percentL > percentR) {
        char diff = abs((percentL-percentR));
        if (diff > SERVO_MIN_LDR_DIFF) {
            if (servoPosition>0) {
                servoPosition-=3;
                servoAxe1.write(servoPosition);
                #if (DEBUG == 1)
                LOG_CYCLE();
                String info = String("Move >: ") + servoPosition;
                Serial.println(info);
                #endif
            }
        }
    }

    // go backward
    if (percentR > percentL) {
        char diff = abs((percentR-percentL));
        if (diff > SERVO_MIN_LDR_DIFF) {
            if (servoPosition<180) {
                servoPosition+=3;
                servoAxe1.write(servoPosition);
                #if (DEBUG == 1)
                LOG_CYCLE();
                String info = String("Move <: ") + servoPosition;
                Serial.println(info);
                #endif
            }
        }
    }

    // servo position change will be monitored only for minimum 10 degres
    char diff = abs(((int)servoPositionPrev - (int)servoPosition));
    if (diff >= SERVO_MIN_POS_LOG) {
        #if (DEBUG == 1)
        LOG_CYCLE();
        String info = String("Servo: ") + servoPosition;
        Serial.println(info);
        #endif

        servoPositionPrev = servoPosition;
    }
    
}


