/*
 * The porpouse is to display the light captured by the two LDR sensors, as a graphic representation
 * So, we have a range, let's say from 0-100%, and if the two LRD's capture the same amount of light,
 * we will display a indicator on the middle of the bar.
 * If right sensor capture two times more light then left sensor, then the indicator will pe place at
 * 33% of the bar.
 */

byte sensorPin1 = A0;
byte sensorPin2 = A1;

int percentLastL = 0;  // variable to store the value coming from the sensor 1
int percentLastR = 0;  // variable to store the value coming from the sensor 2
int percentL = 0;
int percentR = 0;

#define BAR_GRAPH    "____________________________________________________________________________________________________"

void setup() {  
  Serial.begin(9600);
}

void loop() {
  char barGraph[] = BAR_GRAPH;
  unsigned long sensorIntensityL = 0;
  unsigned long sensorIntensityR = 0;

  sensorIntensityL = analogRead(sensorPin1);
  sensorIntensityR = analogRead(sensorPin2);

  percentL = (sensorIntensityL*100)/(sensorIntensityL+sensorIntensityR);
  percentR = (sensorIntensityR*100)/(sensorIntensityL+sensorIntensityR);

  if( (percentLastL != percentL) || (percentLastR != percentR) )
  {
      barGraph[percentR] = '#';
      Serial.println(barGraph);
      
      percentLastL = percentL;
      percentLastR = percentR;
  }
}
