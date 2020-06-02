/*
 *   Setup:
 *      - Pin 8 "GND" to ground
 *      - Pin 16 "Vcc" to 5V
 *      - Pin 13 "OE" to ground
 *      - Pin 10 "MR" to 5V
 *
 *   Bit shift process:
 *      - Select "serial data input" value.
 *        Pin 14 "SER" to GND (to shift a "0") or to VCC (to shift a "1")
 *      - Toggle pin 11, from GND to VCC
 *
 *   Refresh outputs:
 *      - Each bit shifted in, will stay inside STORAGE register, and will not be visible on the output pin.
 *      - To update the 8 output pins (QA, QB, .., QH)
 *        you have to toggle pin 12 "RCLK", from GND to VCC.
 */

#include <Arduino.h>

#define SER_PIN         8   // Serial data input
#define RCLK_PIN        9   // Storage register clock - refresh outputs
#define SRCLK_PIN       10  // Shift register clock - to shift in next bit

void setup() {
    pinMode(SER_PIN, OUTPUT);
    pinMode(SRCLK_PIN, OUTPUT);
    pinMode(RCLK_PIN, OUTPUT);
}

void loop() {
    for(int number=0; number<256; number++)
    {
        digitalWrite(RCLK_PIN, LOW);

        shiftOut(SER_PIN, SRCLK_PIN, MSBFIRST, number);

        digitalWrite(RCLK_PIN, HIGH);

        delay(500);
    }
}