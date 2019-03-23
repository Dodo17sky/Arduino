#include "Debouncer.h"
#include <Arduino.h>

Debouncer::Debouncer() {
	pulse = 0;
	state = STATE_IDLE;
	lastChange = millis();
}

Debouncer::~Debouncer() {
}

void Debouncer::tick(unsigned long ticks) {
	unsigned long now;

	if(ticks == 0) return;

	now = millis();
	pulse += ticks;

	if( (now-lastChange) >= DEBOUNCE_TIME ) {
		state = STATE_IDLE;
	}
	else {
		state = STATE_ACTION;
	}

	lastChange = now;
}
