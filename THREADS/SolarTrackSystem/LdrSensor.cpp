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

	this->lastPrintTimestamp = 0;
}

LdrSensor::~LdrSensor() {
}

void LdrSensor::readAll() {
	uint8_t idx = 0;
	uint16_t ldrSum = 0;
	uint16_t ldrValue = 0;

	for(idx=0; idx<LDR_READ_SAMPLES; idx++) {
		ldrValue = analogRead(this->ldr_pin_H);
		ldrSum += ldrValue;
	}
	this->ldrValueH = (ldrSum/LDR_READ_SAMPLES);
	ldrSum = 0U;

	for(idx=0; idx<LDR_READ_SAMPLES; idx++) {
		ldrValue = analogRead(this->ldr_pin_V);
		ldrSum += ldrValue;
	}
	this->ldrValueV = (ldrSum/LDR_READ_SAMPLES);
	ldrSum = 0U;

	for(idx=0; idx<LDR_READ_SAMPLES; idx++) {
		ldrValue = analogRead(this->ldr_pin_C);
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

	log += String("H: ") + String(this->ldrValueH) + String("    ");
	log += String("C: ") + String(this->ldrValueC) + String("    ");
	log += String("V: ") + String(this->ldrValueV);

	Serial.println(log);
}
