
/***********************************************************************
 *                        MACRO DEFINES
 ***********************************************************************/
#define UT_UNINIT_VALUE				0xFFFFFFFF				// for an uninitialized timer, the best value is a max 32 bit value
#define UT_STOP_TIMER				UT_UNINIT_VALUE			// a stopped timer means it has the max value

class MicroTimer {
	unsigned long	mTime;
	unsigned long	mPeriod;
public:
	MicroTimer();
	~MicroTimer();

public:  // public functions
	void	setPeriod(unsigned long period);
	void	start(unsigned long period);
	void	stop();
	void	restart();
	bool	done(void);

	/*
	 *   ToDo: Find a way so that timer wont be affected by 32 bits overflow
	 */

	/*
	 *   ToDo: Find a way to disable/enable all MicroTimer created
	 */

private: // private functions
};
