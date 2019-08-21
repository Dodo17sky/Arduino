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
	this->is_Measure_Enabled = false;
	this->direction = DIRECTION_BACKWARD;
	this->next_step = NEXT_STEP_HIGH;
	this->last_step_ts = 0;
	this->is_free_to_move = true;
	this->remainingSteps = 0;

	// configure motor pins as output
	pinMode(pinDir   , OUTPUT);
	pinMode(pinStep  , OUTPUT);
	pinMode(pinEnable, OUTPUT);

	this->setSpeed(DEFAULT_SPEED);
}

Nema17Motor::~Nema17Motor() {
	// TODO Auto-generated destructor stub
}

bool Nema17Motor::step() {
	// Return codes:
	// true  - motor has moved one step
	// false - motor did not move

	if( this->is_Enabled == false ) {
		// motor is inactive; don't do any step
		return false;
	}

	uint32_t now = micros();
	if( now < (this->last_step_ts + this->step_speed) ) {
		// delay between steps not elapsed; don't do any step
		return false;
	}

	if( this->next_step == NEXT_STEP_HIGH ) {
		this->stepHigh();
		this->next_step = NEXT_STEP_LOW;
	}
	else {
		this->stepLow();
		this->next_step = NEXT_STEP_HIGH;
	}
	this->last_step_ts = now;
	return true;
}

void Nema17Motor::move(uint16_t howMany)
{
	if( is_free_to_move && (remainingSteps == 0)) {
		// start of the movements
		is_free_to_move = false;
		// steps to make are x2 because is count every high and low step
		// e.g. 7 steps = 7 high + 7 low
		remainingSteps = howMany * 2;
	}

	if( !is_free_to_move && (remainingSteps > 0)) {
		// is moving
		if( this->step() == true ) {
			remainingSteps--;
		}
	}

	if( !is_free_to_move && (remainingSteps == 0)) {
		// end of moving
		is_free_to_move = true;
	}
}

void Nema17Motor::setSpeed(uint16_t speed)
{
	// speed must be in range [80,700] [steps/second]
	// convert this speed to delay between steps
	uint16_t tmpSpeed = 0;

	tmpSpeed = (1000000UL/(2*speed));

	// step delay must be a value in range [700, 6300] [us/step]
	// make this check
	if( 700 <= tmpSpeed && tmpSpeed <= 6300 ) {
		// speed is valid; apply this speed
		this->step_speed = tmpSpeed;
	}
	else {
		// speed is not valid; ignore it
	}
}
