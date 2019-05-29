/*
 * Nema17.cpp
 *
 *  Created on: May 29, 2019
 *      Author: DorinSaramet
 */

#include "Nema17.h"

Nema17::Nema17(uint8_t pinDir, uint8_t pinStep, uint8_t pinEnable)
{
	this->pin_dir = pinDir;
	this->pin_step = pinStep;
	this->pin_enable = pinEnable;
	this->step_speed = DEFAULT_SPEED;
	this->is_Enabled = false;

	// configure motor pins as output
	pinMode(pinDir   , OUTPUT);
	pinMode(pinStep  , OUTPUT);
	pinMode(pinEnable, OUTPUT);
}

Nema17::~Nema17() {
	// TODO Auto-generated destructor stub
}

