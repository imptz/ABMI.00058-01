#ifndef APPLICATION_H
#define APPLICATION_H

#include "timer.h"
#include "ui.h"
#include "logic.h"

class Application : public ITimer, public Singleton<Application>, public MessageReceiver, public MessageSender{
private:
    static const unsigned int TIMER_PERIOD = 10000;
    virtual void timerHandler();
	Logic* logic;

	void createLogic();

public:
    Application();
	~Application();
    void start();
	virtual void onMessage(Message message);
};

#endif

