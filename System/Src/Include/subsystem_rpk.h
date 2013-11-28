#ifndef SUBSYSTEM_RPK_H
#define SUBSYSTEM_RPK_H

#include "singleton.h"
#include "timer.h"
#include "messages.h"
#include "subsystem_iSubsystem.h"
#include "device_iRpk.h"
#include "debug.h"
#include "extensionSystem.h"

class RpkSubsystem : public Singleton<RpkSubsystem>, public ISubsystem{
private:
    static const unsigned int SUBSYSTEM_TYPE = 4;

private:
    virtual void timerHandler();

public:
    RpkSubsystem();
    ~RpkSubsystem();

    virtual void onMessage(Message message);

    enum IO_SUBSYSTEM_MESSAGE{
        IO_SUBSYSTEM_MESSAGE_SUBSYSTEM_READY = 41
    };

private:
    static const unsigned int MAX_DEVICE_COUNT = 1;
    struct RegDevice{
        unsigned char id;
        IRpkDevice* (*pCreateDevice) (unsigned char, unsigned int);
        RegDevice()
            :	id(0), pCreateDevice(nullptr)
        {}
    };
    RegDevice devices[MAX_DEVICE_COUNT];
public:
    bool regDevice(unsigned char id, IRpkDevice* (*pCreateDevice) (unsigned char, unsigned int)){
        for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
            if (devices[i].pCreateDevice == nullptr){
                devices[i].id = id;
                devices[i].pCreateDevice = pCreateDevice;
                return true;
            }
        return false;
    }
    IRpkDevice* pDevices[MAX_DEVICE_COUNT];
    virtual bool addDevice(unsigned char id, unsigned char address);

    void printRegDev(){
        for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
            if (devices[i].pCreateDevice != nullptr){
            }
    }

    virtual void action();
    virtual void init();
    virtual bool putFrame(unsigned char* _pArea, bool isNotTransfer = false);
    virtual bool isReady();

private:
    static const char* MISSING_DEVICE_LOG_TEXT;
public:
    virtual void testMissingDevices();

    unsigned int write(unsigned char* pFrame);
    IRpkDevice::FRAME_RESULT read(unsigned int id, unsigned char** pFrame);

private:
    void enableDevicesDebug();
    void disableDevicesDebug();
};
#endif

