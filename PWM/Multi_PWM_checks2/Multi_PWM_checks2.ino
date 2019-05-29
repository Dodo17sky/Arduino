
/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String        inputString     = "";             // a String to hold incoming data
boolean       stringComplete  = false;          // whether the string is complete
String        serialCommand;



/***********************************************************************
 *                        Private functions
 ***********************************************************************/
 uint8_t Serial_Process(void);

 void setup() {
  inputString.reserve(200);             // reserve 200 bytes for the inputString:

  pinMode(10, OUTPUT);
  Serial.begin(9600);
}

void loop() {
    Serial_Process();
}

uint8_t Serial_Process(void)
{
    if (stringComplete) {
        serialCommand = inputString.substring(0,3);   // the first 3 characters define command type
        #if (DEBUG_ON == 1)
        Serial.print(String("\nCmd: ") + serialCommand + " ");
        #endif

        if(serialCommand == "spd") {
            int inByte = inputString.substring(3).toInt();
            if(inByte >= 0 && inByte <=255) {
              analogWrite(10, inByte);
              Serial.print("Set pwm to ");
              Serial.println(inByte);
            }
        }

       
        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
    }
    return 0;
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
