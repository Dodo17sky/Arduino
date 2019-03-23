#ifndef DEBOUNCER_H_
#define DEBOUNCER_H_

#define	STATE_INIT		0
#define STATE_IDLE		1
#define STATE_ACTION	2

#define DEBOUNCE_TIME	50

class Debouncer {
	/*
	 * ToDo: Check if this data member "pulse" is needed, else remove it
	 */
	unsigned long pulse;
	unsigned char state;
	unsigned long lastChange;
public:
	Debouncer();
	virtual ~Debouncer();
	void tick(unsigned long ticks);
	bool isSteady() { return (state == STATE_IDLE); }
};

#endif /* DEBOUNCER_H_ */
