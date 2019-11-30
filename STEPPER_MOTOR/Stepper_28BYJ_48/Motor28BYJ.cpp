/*
 * Motor28BYJ.cpp
 *
 *  Created on: May 26, 2019
 *      Author: dr
 */

#include "Motor28BYJ.h"

Motor28BYJ::Motor28BYJ(uint8_t motor_pin_1, uint8_t motor_pin_2, uint8_t motor_pin_3, uint8_t motor_pin_4)
{
	// Arduino pins for the motor control connection:
	this->motor_pin_1 = motor_pin_1;
	this->motor_pin_2 = motor_pin_2;
	this->motor_pin_3 = motor_pin_3;
	this->motor_pin_4 = motor_pin_4;

	this->last_step_time = 0;
	this->step_phase  = 0;
	this->step_delay  = DEFAULT_STEP_DELAY;
	this->direction   = DIRECTION_FORWARD;
	this->isEnabled   = false;

	// setup the pins on the microcontroller:
	pinMode(this->motor_pin_1, OUTPUT);
	pinMode(this->motor_pin_2, OUTPUT);
	pinMode(this->motor_pin_3, OUTPUT);
	pinMode(this->motor_pin_4, OUTPUT);
}

Motor28BYJ::~Motor28BYJ() {

}

void Motor28BYJ::step() {
	if( this->isEnabled == false ) {
		// return if motor is off
		return;
	}

	uint32_t now = micros();
	if( (now-this->last_step_time) < this->step_delay ) {
		// return if step delay not passed
		return;
	}
	else {
		// next step can be performed
		this->last_step_time = now;
	}

	// rotate the motor
	this->stepMotor(this->step_phase % 4);

	if ( this->direction == DIRECTION_FORWARD ) {
		this->step_phase = (this->step_phase + 5) % 4;
	}
	else {
		this->step_phase = (this->step_phase + 3) % 4;
	}
}

void Motor28BYJ::setSpeed(uint32_t whatSpeed) {
	this->step_delay = whatSpeed;
}

void Motor28BYJ::stepExactly(uint16_t numberOfSteps) {
	if( this->isEnabled == false ) {
		// return if motor is off
		return;
	}

	// wait one step delay
	while(micros() < (this->last_step_time + this->step_delay) );

	for(uint16_t i = 0; i < numberOfSteps; i++) {
		// rotate the motor
		this->stepMotor(this->step_phase % 4);

		if ( this->direction == DIRECTION_FORWARD ) {
			this->step_phase = (this->step_phase + 5) % 4;
		}
		else {
			this->step_phase = (this->step_phase + 3) % 4;
		}
		delayMicroseconds(this->step_delay);
	}
}

void Motor28BYJ::stepMotor(uint8_t thisStep) {
	switch (thisStep) {
		case 0:  // 1010
			digitalWrite(motor_pin_1, HIGH);
			digitalWrite(motor_pin_2, LOW);
			digitalWrite(motor_pin_3, HIGH);
			digitalWrite(motor_pin_4, LOW);
			break;
		case 1:  // 0110
			digitalWrite(motor_pin_1, LOW);
			digitalWrite(motor_pin_2, HIGH);
			digitalWrite(motor_pin_3, HIGH);
			digitalWrite(motor_pin_4, LOW);
			break;
		case 2:  //0101
			digitalWrite(motor_pin_1, LOW);
			digitalWrite(motor_pin_2, HIGH);
			digitalWrite(motor_pin_3, LOW);
			digitalWrite(motor_pin_4, HIGH);
			break;
		case 3:  //1001
			digitalWrite(motor_pin_1, HIGH);
			digitalWrite(motor_pin_2, LOW);
			digitalWrite(motor_pin_3, LOW);
			digitalWrite(motor_pin_4, HIGH);
			break;
	}
}
