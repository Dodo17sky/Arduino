#include <SPI.h>
/***********************************************************************
 *                        GLOBAL DATA
 ***********************************************************************/
String            inputString     = "";             // a String to hold incoming data
boolean           stringComplete  = false;          // whether the string is complete
volatile uint8_t  spiDataOld      = 0;
volatile uint8_t  spiData         = 0;

static int        idx             = 0;

/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/

/***********************************************************************
 *                        Private functions
 ***********************************************************************/
void Serial_Process(void);
void SPI_Process(void);

 /***********************************************************************
  *                        setup() function
  ***********************************************************************/
 void setup() {
    inputString.reserve(200);
    Serial.begin(115200);

    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    
    // Setup SPI
    pinMode(MISO, OUTPUT);  // Set MISO pin as output
    SPCR |= (1 << SPE);     // Turn on SPI in slave mode
    SPCR |= (1 << SPIE);    // Turn on interrupt
}

 /***********************************************************************
  *                        loop() function
  ***********************************************************************/
void loop() {
    Serial_Process();
    SPI_Process();
}

/***********************************************************************
 *                        Serial_Process()
 ***********************************************************************/
void Serial_Process(void)
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
 *                        SPI_Process
 ***********************************************************************/
void SPI_Process(void)
{
    if(spiDataOld != spiData)
    {
        spiDataOld = spiData;
        
        Serial.print(String(" ") + String(spiData));
        if(spiData == 255)
        {
            Serial.println("");
        }
    }
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

/***********************************************************************
 *                        ISR for SPI RX event
 ***********************************************************************/
ISR(SPI_STC_vect)
{
    spiData = SPDR;
}
