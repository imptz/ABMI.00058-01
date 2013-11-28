#ifndef CONTROL_CLOCK_H
#define CONTROL_CLOCK_H

#include "control.h"
#include "timer.h"

class ClockControl : public Control{
private:
    char* text;
    ClockControl();
    void setDateTimeString(unsigned int mDate, unsigned int mTime);

public:
    ClockControl(unsigned int _positionX, unsigned int _positionY, MessageReceiver* _messageReceiver = nullptr);
    virtual ~ClockControl();
    virtual void draw();
    virtual void onMessage(Message message);
};

#endif
