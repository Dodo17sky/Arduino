/*
 * LdrSensor.h
 *
 *  Created on: May 29, 2019
 *      Author: dr
 */

#ifndef LDRSENSOR_H_
#define LDRSENSOR_H_

#include <Arduino.h>

#define LDR_INVALID_VALUE	( (uint16_t) 0xFFFF )
#define LDR_READ_SAMPLES	(20)
#define LDR_PRINT_DELAY		(2000)		// [ms]

class LdrSensor {
private:
	uint8_t ldr_pin_H;
	uint8_t ldr_pin_V;
	uint8_t ldr_pin_C;

	uint16_t ldrValueH;
	uint16_t ldrValueV;
	uint16_t ldrValueC;

	uint32_t lastPrintTimestamp;

public:
	LdrSensor(uint8_t ldrPinH, uint8_t ldrPinV, uint8_t ldrPinC);
	virtual ~LdrSensor();

	void readAll();
	void printValues();
};

#endif /* LDRSENSOR_H_ */
