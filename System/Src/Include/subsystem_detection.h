#ifndef SUBSYSTEM_DETECTION_H
#define SUBSYSTEM_DETECTION_H

#include "thread.h"
#include "singleton.h"
#include "timer.h"
#include "messages.h"
#include "subsystem_iSubsystem.h"
#include "device_iDetection.h"
#include "debug.h"
#include "extensionSystem.h"
#include "math.h"
#include "fire.h"
#include "action_moveToPoint.h"

class DetectionSubsystem : public Singleton<DetectionSubsystem>, public Fr::Thread<DetectionSubsystem>, public ISubsystem{
private:
    static const unsigned int SUBSYSTEM_TYPE = 2;
    virtual void timerHandler();

public:
    DetectionSubsystem();
    ~DetectionSubsystem();

    virtual void onMessage(Message message);

public:
    enum DETECTION_MANAGER_MESSAGE{
        DETECTION_MANAGER_MESSAGE_MANAGER_READY = 23,
        DETECTION_MANAGER_MESSAGE_DEVICE_FAULT = 24,
        DETECTION_MANAGER_MESSAGE_DEVICE_RECOVERY = 25,
        DETECTION_MANAGER_MESSAGE_CHANNELS_UPDATED = 26
    };

    enum DETECTION{
        DETECTION_PROCCESS,
        DETECTION_FAULT,
        DETECTION_READY
    };

private:
    static const unsigned int COORD_OFFSET_COUNT = 40;
    Point3<float> coordOffset[COORD_OFFSET_COUNT];
    static const unsigned int START_SECTOR = 60000;

public:
    void searchFire(unsigned int* listProgramIndex, unsigned int count);
    void searchFireJustirovka(unsigned char address);
    bool getFireJustirovka();
    void resetDevices();

private:
    static const unsigned int MAX_DEVICE_COUNT = 32;
    struct RegDevice{
        unsigned char id;
        IDetectionDevice* (*pCreateDevice) (unsigned char, unsigned int);
        RegDevice()
            :	id(0), pCreateDevice(nullptr)
        {}
    };
    RegDevice devices[MAX_DEVICE_COUNT];
public:
    bool regDevice(unsigned char id, IDetectionDevice* (*pCreateDevice) (unsigned char, unsigned int)){
        for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
            if (devices[i].pCreateDevice == nullptr){
                devices[i].id = id;
                devices[i].pCreateDevice = pCreateDevice;
                return true;
            }
        return false;
    }
    IDetectionDevice* pDevices[MAX_DEVICE_COUNT];
    virtual bool addDevice(unsigned char id, unsigned char address);

    virtual void action();
    virtual void init();
    virtual bool putFrame(unsigned char* _pArea, bool isNotTransfer = false);
    virtual bool isReady();

private:
    static const char* MISSING_DEVICE_LOG_TEXT;
public:
    virtual void testMissingDevices();

private:
    void enableDevicesDebug();
    void disableDevicesDebug();

    Fr::CPointer<DetectionSubsystem> qqq();
};
#endif
