
//  ===================  GLOBAL DATA  ======================
String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete

String cmd;

int Index;

int stepSpeed = 1000;
int stepCount = 400;
int cycleDelay = 1000;
 
 void setup() {
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
  pinMode(6, OUTPUT); // enable
  pinMode(5, OUTPUT); // step
  pinMode(4, OUTPUT); // direction
  Serial.begin(9600);

  digitalWrite(6, LOW);
}

void stepperCmd();

void loop() {    
    if (stringComplete) {
        cmd = inputString.substring(0,3);

        Serial.println(cmd);

        if(cmd == "onn") {
          digitalWrite(6, LOW);
        }

        if(cmd == "off") {
          digitalWrite(6, HIGH);
        }

        if(cmd == "spd") {
          String tmp = inputString.substring(3);
          stepSpeed = tmp.toInt();
        }

        if(cmd == ">>>") {
          digitalWrite(4, HIGH);
        }

        if(cmd == "<<<") {
          digitalWrite(4, LOW);
        }

        if(cmd == "nmb") {
          String tmp = inputString.substring(3);
          stepCount = tmp.toInt();
        }

        if(cmd == "del") {
          String tmp = inputString.substring(3);
          cycleDelay = tmp.toInt();
        }
       
        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }
    
    stepperCmd();
}

void stepperCmd()
{
  for(Index=0; Index<stepCount; Index++)
  {
      digitalWrite(5, HIGH);
      delayMicroseconds(stepSpeed);
      digitalWrite(5, LOW);
      delayMicroseconds(stepSpeed);
  }
  if(cycleDelay > 0) {
      delay(cycleDelay);
  }
}

void serialEvent() {
  while (Serial.available()) {
      // get the new byte:
      char inChar = (char)Serial.read();
      
      // if the incoming character is a newline, set a flag so the main loop can
      // do something about it:
      if (inChar == '\n') {
        stringComplete = true;
      }
      else {
        // add it to the inputString:
        inputString += inChar;
      }
   }
}
