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
#define LDR_READ_SAMPLES	(3)
#define LDR_PRINT_DELAY		(2000)		// [ms]

#define	LDR_DIR_TO_DOWN		((int8_t)(-1))
#define	LDR_DIR_TO_LEFT		((int8_t)(-1))
#define	LDR_DIR_NO_DIR		((int8_t)( 0))
#define	LDR_DIR_TO_RIGHT	((int8_t)( 1))
#define	LDR_DIR_TO_UP		((int8_t)( 1))

class LdrSensor {
private:
	uint8_t ldr_pin_H;
	uint8_t ldr_pin_V;
	uint8_t ldr_pin_C;

	uint16_t ldrValueH;
	uint16_t ldrValueV;
	uint16_t ldrValueC;

	uint16_t ldrCalibrationH;
	uint16_t ldrCalibrationV;
	uint16_t ldrCalibrationC;

	uint16_t ldrMinDifference;

	uint32_t lastPrintTimestamp;

public:
	LdrSensor(uint8_t ldrPinH, uint8_t ldrPinV, uint8_t ldrPinC);
	virtual ~LdrSensor();

	void readAll();
	void printValues();

	int8_t getDirectionV();
	int8_t getDirectionH();

	void setMinDifference(uint16_t minDiff) { this->ldrMinDifference = minDiff; }
	void setCalibrationH(uint16_t calibH) { this->ldrCalibrationH = calibH; }
	void setCalibrationV(uint16_t calibV) { this->ldrCalibrationV = calibV; }
	void setCalibrationC(uint16_t calibC) { this->ldrCalibrationC = calibC; }
};

#endif /* LDRSENSOR_H_ */
