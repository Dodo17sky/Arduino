/*
 * Stepper28BYJ.h
 *
 *  Created on: May 27, 2019
 *      Author: dr
 */

#ifndef STEPPER28BYJ_H_
#define STEPPER28BYJ_H_

#include "Arduino.h"

#define DIRECTION_FORWARD		1
#define DIRECTION_BACKWARD		0
#define DEFAULT_STEP_DELAY		5000	//  [us]

class Stepper28BYJ {
private:
	// motor pin numbers:
	uint8_t 	motor_pin_1;
	uint8_t 	motor_pin_2;
	uint8_t 	motor_pin_3;
	uint8_t 	motor_pin_4;

	uint32_t	step_delay;		// delay between steps, in ms, based on speed
	uint32_t	last_step_time; // time stamp of last step performed;
	uint8_t		step_phase;		// next step to make {0, 1, 2, 3}
	uint8_t  	direction;      // Direction of rotation
	boolean		isEnabled;

public:
	Stepper28BYJ(uint8_t motor_pin_1, uint8_t motor_pin_2, uint8_t motor_pin_3, uint8_t motor_pin_4);
	virtual ~Stepper28BYJ();

	void step();
	void stepExactly(uint16_t numberOfSteps);
	void setSpeed(uint32_t whatSpeed);
	void stepMotor(uint8_t thisStep);

	inline void TurnOn() 	{ this->isEnabled = true; }
	inline void TurnOff() 	{ this->isEnabled = false; }
	inline void GoForward() { this->direction = DIRECTION_FORWARD; }
	inline void GoBackward() { this->direction = DIRECTION_BACKWARD; }
	inline boolean isOn()	{ return this->isEnabled; }
};

#endif /* STEPPER28BYJ_H_ */
