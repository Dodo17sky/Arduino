#include <Servo.h>

//  ===================  GLOBAL DATA  ======================
String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete
byte angle = 0;
byte tmpAngle = 0;
short add = 3;
short val = 0;
unsigned int ts = 0;

Servo myservo;  // create servo object to control a servo

void setup() {
  inputString.reserve(200);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(57600);
}

void loop() {
    if (stringComplete) {
        tmpAngle = inputString.toInt();
        
        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }

    if( 0 <= tmpAngle && tmpAngle <= 180 && tmpAngle != angle) {
        String info = String("Set angle ") + tmpAngle;
        Serial.println(info);
        
        angle = tmpAngle;
        myservo.write(angle);
    }

    if( tmpAngle == 201) {
        if(millis() >= (ts+30)) { 
            Serial.println(val);
            myservo.write(val);

            val += add;
            
            if( val < 0   )   add = (-1*add);
            if( val > 180 )   add = (-1*add);
    
            ts = millis();
        }
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
