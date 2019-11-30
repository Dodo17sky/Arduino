/*
 * LdrSensor.cpp
 *
 *  Created on: May 29, 2019
 *      Author: dr
 */

#include "LdrSensor.h"

LdrSensor::LdrSensor(uint8_t ldrPinH, uint8_t ldrPinV, uint8_t ldrPinC) {
	this->ldr_pin_H = ldrPinH;
	this->ldr_pin_V = ldrPinV;
	this->ldr_pin_C = ldrPinC;

	this->ldrValueH = LDR_INVALID_VALUE;
	this->ldrValueV = LDR_INVALID_VALUE;
	this->ldrValueC = LDR_INVALID_VALUE;

	this->ldrCalibrationH = 0;
	this->ldrCalibrationV = 0;
	this->ldrCalibrationC = 0;

	this->ldrMinDifference = 15;
	this->lastPrintTimestamp = 0;
}

LdrSensor::~LdrSensor() {
}

void LdrSensor::readAll() {
	uint8_t idx = 0;
	uint16_t ldrSum = 0;
	uint16_t ldrValue = 0;

	for(idx=0; idx<LDR_READ_SAMPLES; idx++) {
		ldrValue = analogRead(this->ldr_pin_H) + this->ldrCalibrationH;
		ldrSum += ldrValue;
	}
	this->ldrValueH = (ldrSum/LDR_READ_SAMPLES);
	ldrSum = 0U;

	for(idx=0; idx<LDR_READ_SAMPLES; idx++) {
		ldrValue = analogRead(this->ldr_pin_V) + this->ldrCalibrationV;
		ldrSum += ldrValue;
	}
	this->ldrValueV = (ldrSum/LDR_READ_SAMPLES);
	ldrSum = 0U;

	for(idx=0; idx<LDR_READ_SAMPLES; idx++) {
		ldrValue = analogRead(this->ldr_pin_C)  + this->ldrCalibrationC;
		ldrSum += ldrValue;
	}
	this->ldrValueC = (ldrSum/LDR_READ_SAMPLES);
	ldrSum = 0U;
}

void LdrSensor::printValues() {
	String log = "";
	uint32_t timestamp = millis();

	if(timestamp < (this->lastPrintTimestamp + LDR_PRINT_DELAY)) {
		return;
	}
	this->lastPrintTimestamp = timestamp;

	String msg = "";
	if(ldrValueC > ldrValueV) {
		msg += String("to right ");
		msg += String(ldrValueC - ldrValueV);
	}
	else {
		msg += String("to left ");
		msg += String(ldrValueV - ldrValueC);
	}

	msg += String("         ");
	if(ldrValueC > ldrValueH) {
		msg += String("to down ");
		msg += String(ldrValueC - ldrValueH);
	}
	else {
		msg += String("to up ");
		msg += String(ldrValueH - ldrValueC);
	}

	/*
	log += String("H: ") + String(this->ldrValueH) + String("    ");
	log += String("C: ") + String(this->ldrValueC) + String("    ");
	log += String("V: ") + String(this->ldrValueV);
	*/

	Serial.println(msg);
}

int8_t LdrSensor::getDirectionV() {
	int8_t  dirToGo = LDR_DIR_NO_DIR;
	uint16_t diffBetweenLdr = abs((int)this->ldrValueV - (int)this->ldrValueC);

	if( diffBetweenLdr >= this->ldrMinDifference ) {
		// We have a left or right direction only if the difference between sensors
		// is greater then LDR_MIN_DIFFERENCE

		if( this->ldrValueV > this->ldrValueC ) {
			// move to the left
			dirToGo = LDR_DIR_TO_LEFT;
		}
		else {
			// move to the right
			dirToGo = LDR_DIR_TO_RIGHT;
		}
	}
	else {
		// ignore sensors difference smaller then LDR_MIN_DIFFERENCE
		// the motor movements may became unstable due to sensors reading errors
	}

	return dirToGo;
}

int8_t LdrSensor::getDirectionH() {
	int8_t  dirToGo = LDR_DIR_NO_DIR;
	uint16_t diffBetweenLdr = abs((int)this->ldrValueH - (int)this->ldrValueC);

	if( diffBetweenLdr >= this->ldrMinDifference ) {
		// We have a up or down direction only if the difference between sensors
		// is greater then LDR_MIN_DIFFERENCE

		if( this->ldrValueH > this->ldrValueC ) {
			// move to the left
			dirToGo = LDR_DIR_TO_UP;
		}
		else {
			// move to the right
			dirToGo = LDR_DIR_TO_DOWN;
		}
	}
	else {
		// ignore sensors difference smaller then LDR_MIN_DIFFERENCE
		// the motor movements may became unstable due to sensors reading errors
	}

	return dirToGo;
}

