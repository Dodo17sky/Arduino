/*
 *  An SPI communication can be tested using shift registers.
 *  This is possible because any SPI module has shift registers on the output/input channels
 *  In this example I am using 74HC595 shift register.
 *
 *  The hardware connection is this:
 *
 *     Arduino pins    |     Shift register 74HC595  |    Meaning
 *  -------------------+-----------------------------+--------------------------------------------------
 *     SPI_CLK         |            SRCLK            | Arduino SPI clock to shift register clock
 *  -------------------+-----------------------------+--------------------------------------------------
 *     SPI_MOSI        |            SER              | Arduino out data to shift register SER data input
 *  -------------------+-----------------------------+--------------------------------------------------
 *     SPI_SS          |            RCLK             | Arduino chip select to shift register storage clock
 *  -------------------+-----------------------------+--------------------------------------------------
 *
 */

#include <Arduino.h>
#include "SPI.h"

#define CHIP_SELECT 10

void setup() {
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);

}

void loop() {
    for(int number=0; number<256; number++)
    {
        digitalWrite(CHIP_SELECT, LOW);
        SPI.transfer(number);
        digitalWrite(CHIP_SELECT, HIGH);
        delay(100);
    }
}
