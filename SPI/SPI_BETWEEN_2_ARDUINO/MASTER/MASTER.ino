#include <SPI.h>

/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String          inputString     = "";             // a String to hold incoming data
String          serialCommand;
boolean         stringComplete  = false;          // whether the string is complete


/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#define   DEBUG_ON                1
#define   ONBOARD_LED             4
#define   SLAVE_PIN          10


/***********************************************************************
 *                        Private functions
 ***********************************************************************/
uint8_t Serial_Process(void);
uint8_t Led_Process(void);
uint8_t SPI_Process(void);

 /***********************************************************************
  *                        setup() function
  ***********************************************************************/
 void setup() {
  Serial.begin(115200);
  inputString.reserve(200);             // reserve 200 bytes for the inputString:

  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);

    // set the slaveSelectPin as an output:
  pinMode(SLAVE_PIN, OUTPUT);
  // initialize SPI:
  SPI.begin();
}

 /***********************************************************************
  *                        loop() function
  ***********************************************************************/
void loop() {
    Serial_Process();
    Led_Process();
    SPI_Process();
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

        }

        if(serialCommand == "off") {

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
uint8_t Led_Process(void)
{
    static long lastCall = 0;
    static byte isOff = 1;
    long now = millis();
    if(now >= (lastCall + 250))
    {
      if(isOff) {
          digitalWrite(ONBOARD_LED, HIGH);
          isOff = 0;
      }
      else
      {
          digitalWrite(ONBOARD_LED, LOW);
          isOff = 1;
      }
      lastCall = now;
    }
    return 0;
}

/***********************************************************************
 *                        Motor_Process()
 ***********************************************************************/
uint8_t SPI_Process(void)
{
    static long lastCall = 0;
    static byte counter = 0;
    long now = millis();

    if(now >= (lastCall + 1000))
    {
        // take the SS pin low to select the chip:
        digitalWrite(SLAVE_PIN, LOW);
        delay(100);
    
        counter++;
        SPI.transfer(counter);
    
        delay(100);
        // take the SS pin high to de-select the chip:
        digitalWrite(SLAVE_PIN, HIGH);
    
        lastCall = now;
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
