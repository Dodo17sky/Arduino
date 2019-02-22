byte sensorPin1 = A0;    // select the input pin for the potentiometer
byte sensorPin2 = A1;

int sensorValLeft  = 0;  // variable to store the value coming from the sensor 1
int sensorValRight = 0;  // variable to store the value coming from the sensor 2
unsigned long timestamp = 0;

void setup() {  
  Serial.begin(9600);
}

void loop() {
  int tmpSenL = 0;
  int tmpSenR = 0;

  tmpSenL = analogRead(sensorPin1);
  tmpSenR = analogRead(sensorPin2);

  if( ((abs(sensorValLeft-tmpSenL) > 25) || (abs(sensorValRight-tmpSenR) > 25)) &&
      (millis() >= (timestamp + 100))
    ) 
  {
      // print new LDR values if there is a difference of at least 20, but not faster then 100 ms
    
      Serial.print(sensorValLeft);
      Serial.print(" - ");
      Serial.println(sensorValRight);

      sensorValLeft  = tmpSenL;
      sensorValRight = tmpSenR;

      timestamp = millis();
  }
  
}
