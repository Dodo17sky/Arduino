/*
 * Nema17.cpp
 *
 *  Created on: May 29, 2019
 *      Author: DorinSaramet
 */

#include "Nema17Motor.h"

Nema17Motor::Nema17Motor(uint8_t pinDir, uint8_t pinStep, uint8_t pinEnable)
{
	this->pin_dir = pinDir;
	this->pin_step = pinStep;
	this->pin_enable = pinEnable;
	this->step_speed = DEFAULT_SPEED;
	this->is_Enabled = false;
	this->stepsDone = 0;
	this->is_Measure_Enabled = false;
	this->position = 0;
	this->direction = DIRECTION_BACKWARD;

	// configure motor pins as output
	pinMode(pinDir   , OUTPUT);
	pinMode(pinStep  , OUTPUT);
	pinMode(pinEnable, OUTPUT);
}

Nema17Motor::~Nema17Motor() {
	// TODO Auto-generated destructor stub
}

void Nema17Motor::goToPosition(int16_t targetPos) {
	if(targetPos < this->position) {
		this->goBackward();
	}
	else {
		this->goForward();
	}

	while(targetPos != this->position) {
		this->step();
	}
}

void Nema17Motor::setSpeed(uint16_t speed)
{
	// speed must be in range [80,700] [steps/second]
	// convert this speed to delay between steps
	uint16_t tmpSpeed = 0;

	tmpSpeed = (1000000UL/(2*speed));

	// step delay must be a value in range [700, 6300]
	// make this check
	if( 700 <= tmpSpeed && tmpSpeed <= 6300 ) {
		// speed is valid; apply this speed
		this->step_speed = tmpSpeed;
	}
	else {
		// speed is not valid; ignore it
	}
}
