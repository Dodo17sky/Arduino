/*
 * Nema17.h
 *
 *  Created on: May 29, 2019
 *      Author: DorinSaramet
 */

#ifndef NEMA17MOTOR_H_
#define NEMA17MOTOR_H_

#include <Arduino.h>

#define DEFAULT_SPEED			((uint16_t) 2500)
#define DIRECTION_FORWARD		1
#define DIRECTION_BACKWARD		-1

class Nema17Motor {
private:
	uint8_t		pin_dir;
	uint8_t		pin_step;
	uint8_t		pin_enable;
	int8_t		direction;
	int16_t		position;
	uint16_t 	step_speed; // [us]
	boolean		is_Enabled;
	boolean		is_Measure_Enabled;
	int16_t		stepsDone;

public:
	Nema17Motor(uint8_t pinDir, uint8_t pinStep, uint8_t pinEnable);
	virtual ~Nema17Motor();

	void goToPosition(int16_t targetPos);
	inline int16_t getPosition() const { return this->position; }
	void setSpeed(uint16_t speed);
	inline void turnOn() { digitalWrite(this->pin_enable, LOW); this->is_Enabled = true; }
	inline void turnOff() { digitalWrite(this->pin_enable, HIGH); this->is_Enabled = false; }
	inline bool isOn() const { return (this->is_Enabled == true); }
	inline void goForward() { digitalWrite(this->pin_dir, HIGH); this->direction = DIRECTION_FORWARD; }
	inline void goBackward() { digitalWrite(this->pin_dir, LOW); this->direction = DIRECTION_BACKWARD; }
	inline void stepHigh() {
		digitalWrite(this->pin_step, HIGH);
	}
	inline void stepLow() {
		digitalWrite(this->pin_step, LOW);
		this->stepsDone += (int16_t)(this->is_Measure_Enabled);
		this->position += this->direction;
	}
	inline void step() {
		this->stepHigh(); delayMicroseconds(this->step_speed);
		this->stepLow();  delayMicroseconds(this->step_speed);
	}
	inline void move(uint16_t howMany) {
		while((howMany--) > 0) {
			this->step();
		}
	}
	inline uint16_t getSpeed() const { return this->step_speed; }
	inline void stepMeasureStart() { this->is_Measure_Enabled = true; this->stepsDone = 0; }
	inline void stepMeasureStop() { this->is_Measure_Enabled = false; }
	inline int16_t getStepMeasurement() const { return this->stepsDone; }
	inline void setCenterPosition() { this->position = 0; }
};

#endif /* NEMA17MOTOR_H_ */
