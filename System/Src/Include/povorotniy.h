#ifndef POVOROTNIY_H
#define POVOROTNIY_H

#include "thread.h"
#include "timer.h"
#include "config.h"
#include "action_povorotniy.h"

class Povorotniy : public Fr::Thread<Povorotniy>, public ITimer, public Singleton<Povorotniy>{
private:
    virtual void timerHandler();
    static const unsigned int TIMER_PERIOD = 1000;

public:
    Povorotniy();
    ~Povorotniy();

private:
    Fr::CPointer<Povorotniy> init();
    Fr::CPointer<Povorotniy> wait();
    Fr::CPointer<Povorotniy> go();

    ConfigDataStructPRPosition** sp;
    unsigned int actionCount;
    Action** actionList;

    bool fStart;
public:
    void start();
};
#endif
