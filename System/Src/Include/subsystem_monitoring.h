#ifndef SUBSYSTEM_MONITORING_H
#define SUBSYSTEM_MONITORING_H

#include "singleton.h"
#include "timer.h"
#include "messages.h"
#include "subsystem_iSubsystem.h"
#include "device_iMonitoring.h"
#include "debug.h"
#include "extensionSystem.h"

class MonitoringSubsystem : public Singleton<MonitoringSubsystem>, public ISubsystem{
private:
    static const unsigned int SUBSYSTEM_TYPE = 3;

private:
    virtual void timerHandler();

public:
    MonitoringSubsystem();
    ~MonitoringSubsystem();

    virtual void onMessage(Message message);

private:
    static const unsigned int MAX_DEVICE_COUNT = 32;
    struct RegDevice{
        unsigned char id;
        IMonitoringDevice* (*pCreateDevice) (unsigned char, unsigned int);
        RegDevice()
            :	id(0), pCreateDevice(nullptr)
        {}
    };
    RegDevice devices[MAX_DEVICE_COUNT];
public:
    bool regDevice(unsigned char id, IMonitoringDevice* (*pCreateDevice) (unsigned char, unsigned int)){
        for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
            if (devices[i].pCreateDevice == nullptr){
                devices[i].id = id;
                devices[i].pCreateDevice = pCreateDevice;
                return true;
            }
        return false;
    }
    IMonitoringDevice* pDevices[MAX_DEVICE_COUNT];
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

public:
    enum MONITOR_MANAGER_MESSAGE{
        MONITOR_MANAGER_MESSAGE_MANAGER_READY = 34,
        MONITOR_MANAGER_MESSAGE_EVENT = 35,
        MONITOR_MANAGER_MESSAGE_RPK = 36
    };

public:
    void createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER messageNumber, unsigned char parameter1 = 0, unsigned char parameter2 = 0, unsigned char parameter3 = 0, unsigned char parameter4 = 0);

private:
    void enableDevicesDebug();
    void disableDevicesDebug();
};
#endif

