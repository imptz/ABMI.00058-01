#ifndef ACTION_GATE_CLOSE_H
#define ACTION_GATECLOSE_H

#include "action.h"

class ActionGateClose : public Action{
private:
	virtual void timerHandler();
	unsigned int testTimeOut;
	static const unsigned int TEST_TIME_OUT = 30;
	ActionGateClose();

public:
	ActionGateClose(unsigned char _deviceAddress);
	virtual ~ActionGateClose();
	virtual void step();

private:
	enum PHASE{
		PHASE_COMMAND,
		PHASE_COMMAND_WAIT,
		PHASE_TEST,
		PHASE_TEST_WAIT,
		PHASE_STOP
	};
	PHASE phase;

	void error();
	void finish();
};
#endif

