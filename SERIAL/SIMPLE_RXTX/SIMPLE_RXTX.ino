/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String          inputString     = "";             // a String to hold incoming data
boolean         stringComplete  = false;          // whether the string is complete


/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
uint8_t Serial_Process(void);

 /***********************************************************************
  *                        setup() function
  ***********************************************************************/
 void setup() {
  inputString.reserve(200);
  Serial.begin(115200);
}

 /***********************************************************************
  *                        loop() function
  ***********************************************************************/
void loop() {
    Serial_Process();
}

/***********************************************************************
 *                        Serial_Process()
 ***********************************************************************/
uint8_t Serial_Process(void)
{
    if (stringComplete) {
        Serial.println(inputString);

        
        stringComplete = false;
        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
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
