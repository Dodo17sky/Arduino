/*
 * Nema17.h
 *
 *  Created on: May 29, 2019
 *      Author: DorinSaramet
 */

#ifndef NEMA17_H_
#define NEMA17_H_

#include <Arduino.h>

#define DEFAULT_SPEED	((uint16_t) 2500)

class Nema17 {
private:
	uint8_t		pin_dir;
	uint8_t		pin_step;
	uint8_t		pin_enable;
	uint16_t 	step_speed; // [us]
	boolean		is_Enabled;
public:
	Nema17(uint8_t pinDir, uint8_t pinStep, uint8_t pinEnable);
	virtual ~Nema17();

	inline void setSpeed(uint16_t speed) { this->step_speed = speed; }
	inline void turnOn() { digitalWrite(this->pin_enable, LOW); this->is_Enabled = true; }
	inline void turnOff() { digitalWrite(this->pin_enable, HIGH); this->is_Enabled = false; }
	inline bool isOn() const { return (this->is_Enabled == true); }
	inline void goForward() const { digitalWrite(this->pin_dir, HIGH); }
	inline void goBackward() const { digitalWrite(this->pin_dir, LOW); }
	inline void stepHigh() const { digitalWrite(this->pin_step, HIGH); }
	inline void stepLow() const { digitalWrite(this->pin_step, LOW); }
	inline void step() const {
		this->stepHigh(); delayMicroseconds(this->step_speed);
		this->stepLow();  delayMicroseconds(this->step_speed);
	}
};

#endif /* NEMA17_H_ */
