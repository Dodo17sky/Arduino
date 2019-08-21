/*
 * Nema17.h
 *
 *  Created on: May 29, 2019
 *      Author: DorinSaramet
 */

#ifndef NEMA17MOTOR_H_
#define NEMA17MOTOR_H_

#include <Arduino.h>

#define DEFAULT_SPEED			((uint16_t) 250)
#define DIRECTION_FORWARD		1
#define DIRECTION_BACKWARD		-1

#define NEXT_STEP_HIGH			1
#define NEXT_STEP_LOW			2

class Nema17Motor {
private:
	uint8_t		pin_dir		:4;
	uint8_t		pin_step	:4;
	uint8_t		pin_enable	:4;
	int8_t		direction	:2;
	uint8_t		next_step	:2;
	boolean		is_Enabled	:1;
	boolean		is_Measure_Enabled 	:1;
	boolean		is_free_to_move		:1;
	uint16_t 	step_speed;  // [us]
	uint32_t	last_step_ts;
	uint16_t	remainingSteps;

public:
	Nema17Motor(uint8_t pinDir, uint8_t pinStep, uint8_t pinEnable);
	virtual ~Nema17Motor();

	bool step();
	void move(uint16_t howMany);
	void setSpeed(uint16_t speed);
	inline uint16_t getSpeed() const { return this->step_speed; }
	inline bool isOn() const { return (this->is_Enabled == true); }
	inline boolean isFreeToMove() { return this->is_free_to_move; }
	inline void turnOn() {
		if( this->is_Enabled == false ) {
			digitalWrite(this->pin_enable, LOW);
			this->is_Enabled = true;
		}
	}
	inline void turnOff() {
		if( this->is_Enabled == true ) {
			digitalWrite(this->pin_enable, HIGH);
			this->is_Enabled = false;
		}
	}
	inline void goForward() {
		if( this->direction != DIRECTION_FORWARD ) {
			digitalWrite(this->pin_dir, HIGH);
			this->direction = DIRECTION_FORWARD;
		}
	}
	inline void goBackward() {
		if( this->direction != DIRECTION_BACKWARD ) {
			digitalWrite(this->pin_dir, LOW);
			this->direction = DIRECTION_BACKWARD;
		}
	}
	inline void stepHigh() {
		digitalWrite(this->pin_step, HIGH);
	}
	inline void stepLow() {
		digitalWrite(this->pin_step, LOW);
	}
};

#endif /* NEMA17MOTOR_H_ */
