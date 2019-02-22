/*
 * If the light is more intensive on the left sensor, turn on the left led
 * If the light is more intensive on the right sensor, turn on the right led
 */

byte sensorPin1 = A0;
byte sensorPin2 = A1;

byte outLedLeft  = 7;
byte outLedRight = 8;

int percentLastL = 0;  // variable to store the value coming from the sensor 1
int percentLastR = 0;  // variable to store the value coming from the sensor 2
int percentL = 0;
int percentR = 0;

void setup() {  
  Serial.begin(9600);
  pinMode(outLedLeft ,OUTPUT);
  pinMode(outLedRight,OUTPUT);
}

void loop() {
  unsigned long sensorIntensityL = 0;
  unsigned long sensorIntensityR = 0;

  sensorIntensityL = analogRead(sensorPin1);
  sensorIntensityR = analogRead(sensorPin2);

  percentL = (sensorIntensityL*100)/(sensorIntensityL+sensorIntensityR);
  percentR = (sensorIntensityR*100)/(sensorIntensityL+sensorIntensityR);

  if( (percentLastL != percentL) || (percentLastR != percentR) )
  {
      if(percentL > percentR) {
          digitalWrite(outLedRight,LOW);
          digitalWrite(outLedLeft,HIGH);
      }
      else {
          digitalWrite(outLedLeft,LOW);
          digitalWrite(outLedRight,HIGH);
      }
      
      percentLastL = percentL;
      percentLastR = percentR;
  }
}
