#define LED_PIN  9
 
volatile uint8_t led_blink = 1;
volatile uint8_t spiDataOld = 0;
volatile uint8_t spiData = 0;
 
ISR(SPI_STC_vect)
{
    uint8_t data_byte = SPDR;
    spiData = data_byte;
    
    switch (data_byte)
    {
        case '0':
            led_blink = 0;
            SPDR = 0;  
        break;
        case '1':
            led_blink = 1;
            SPDR = 0;  
        break;
        case '?':
            // Place LED blinking status in SPDR register for next transfer
            SPDR = led_blink;  
        break;
    }
}
 
void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
 
    // Set MISO pin as output
    pinMode(MISO, OUTPUT);
    
    // Turn on SPI in slave mode
    SPCR |= (1 << SPE);
    // Turn on interrupt
    SPCR |= (1 << SPIE);
}
 
void loop() 
{
    if(spiDataOld != spiData)
    {
        spiDataOld = spiData;
        if(spiData == 0x01)
        {
          Serial.print("\n");
          Serial.print(millis());
          Serial.print(": ");
          Serial.print(spiData);
        }
        else
        {
          Serial.print(", ");
          Serial.print(spiData);
        }
    }
    
    // If LED blink status is on, then blink LED for 250ms
    if (spiData == 8)
    {
        digitalWrite(LED_PIN, HIGH);
    }
    else if (spiData == 6)
    {
        digitalWrite(LED_PIN, LOW); 
    }
}
