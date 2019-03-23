#include "MicroTimer.h"
#include <Arduino.h>

MicroTimer::MicroTimer()
{
	mTime = UT_UNINIT_VALUE;
	mPeriod = 0;
}

MicroTimer::~MicroTimer() {
}

void MicroTimer::start(unsigned long period) {
	mTime = micros() + period;
	mPeriod = period;
}

void MicroTimer::stop() {
	mTime = UT_STOP_TIMER;
}

void MicroTimer::restart() {
	mTime = micros() + mPeriod;
}

void MicroTimer::setPeriod(unsigned long period) {
	mPeriod = period;
}

bool MicroTimer::done(void) {
	return (
		(mTime != UT_STOP_TIMER)
			&&
		(micros() >= mTime)
	);
}
