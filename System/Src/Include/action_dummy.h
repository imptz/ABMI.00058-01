#ifndef ACTION_DUMMY_H
#define ACTION_DUMMY_H

#include "action.h"

class ActionDummy : public Action{
private:
	virtual void timerHandler();
	virtual void step();
	virtual void onMessage(Message message);

public:
	ActionDummy();
	virtual ~ActionDummy();
};

#endif

