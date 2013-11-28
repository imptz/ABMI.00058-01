#ifndef ACTION_SEND_PRESSURE_TABLE_H
#define ACTION_SEND_PRESSURE_TABLE_H

#include "action.h"
#include "fire.h"

class ActionSendPressureTable : public Action{
private:
	virtual void timerHandler();
	static const unsigned int TEST_TIME_OUT = 60;

	Fire::FireScanProgram* program;

	ActionSendPressureTable();

public:
	ActionSendPressureTable(Fire::FireScanProgram* _program);
	virtual ~ActionSendPressureTable();
	virtual void step();

private:
	enum PHASE{
		PHASE_COMMAND,
		PHASE_COMMAND_WAIT,
		PHASE_STOP
	};
	PHASE phase;

	void error();
	void finish();
};
#endif
