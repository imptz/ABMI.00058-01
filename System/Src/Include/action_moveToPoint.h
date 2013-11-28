#ifndef ACTION_MOVE_TO_POINT_H
#define ACTION_MOVE_TO_POINT_H

#include "action.h"

class ActionMoveToPoint : public Action{
private:
	virtual void timerHandler();
	static const unsigned int TEST_TIME_OUT = 120;
	ActionMoveToPoint();

public:
	ActionMoveToPoint(unsigned char _deviceAddress, Point2<unsigned int> _point, unsigned int _nasadok = 0);
	virtual ~ActionMoveToPoint();
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

	Point2<unsigned int> point;
	unsigned int nasadok;

public:
	Point2<unsigned int> getPoint();
};
#endif
