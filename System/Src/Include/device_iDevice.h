#ifndef DEVICE_IDEVICE_H
#define DEVICE_IDEVICE_H

#include "messages.h"
#include "fifo.h"
#include "debug.h"

class IDevice{
public:
    enum COMMAND{
        COMMAND_GET_ID = 1
    };

protected:
    static const unsigned int FIFO_FRAME_SIZE = 1000;
    Fifo<unsigned char*>* fifoFrame;

    unsigned char address;

public:
    IDevice(unsigned char _address, unsigned int _type)
        :	fifoFrame(new Fifo<unsigned char*>(FIFO_FRAME_SIZE)), address(_address), type(_type), actionTimeOut(0), isActionTimeOut(false), disabled(false), sdebug(false)
    {}
	virtual ~IDevice(){};

    virtual void action() = 0;
    virtual bool putFrame(unsigned char* _pArea, bool isNotTransfer = false) = 0;
    virtual void init() = 0;
    virtual bool isReady() = 0;
    virtual void timerHandler() = 0;
    virtual unsigned int getId() = 0;

protected:
    unsigned int type;
    static const unsigned int INITIALIZE_RESULT_OK = 1;
    static const unsigned int ACTION_TIME_OUT = 20;
    unsigned int actionTimeOut;
    bool isActionTimeOut;

    bool disabled;
    virtual void disablingDevice() = 0;

public:
    bool sdebug;
};
#endif

