#ifndef ROOT_H
#define ROOT_H

#include "singleton.h"
#include "interrupt.h"
#include "display.h"
#include "serialPort.h"
#include "timer.h"
#include "hddManager.h"
#include "touchpad.h"
#include "application.h"
#include "messages.h"
#include "log.h"
#include "clock.h"
#include "config.h"
#include "ui.h"
#include "extensionSystem.h"
#include "device_bk16.h"
//#include "device_fv300.h"
//#include "device_scanner.h"
//#include "device_tv.h"
#include "device_monitoring.h"
#include "device_rpk.h"
#include "povorotniy.h"
#include "cpu.h"

#include "lowDisplay.h"

//! ����� �������� ��� ��������� �������.
/*!
	��������� ����� Root �������� ��������� �� ��� ��������� �������, ������� � ������� ��.
*/
class Root : public Singleton<Root>{
private:
	MemoryAllocator memoryAllocator;
	Display* pDisplay;
	TimerManager* pTimerManager;
	SerialPortManager* pSerialPortManager;
	HddManager* pHddManager;
	Touchpad* pTouchpad;

	SerialDebug* pSerialDebug;

	Log* pLog;
	Clock* pClock;
	Config* pConfig;

	Povorotniy* pPovorotniy;

	UI* pUI;

	Application* pApplication;

public:
	//! �����������.
	/*!
		�������������� ���������� ����������, ������ �����������, ���������� ���.
		������� ��� ��������� �������.
		������������ ���������� ����������.
		���������� ������ �� ������������ ��� ����� ���������� ����������� ��������� ����� ���������� ����� �������������� � ������������ ���.
	*/
	Root(){
		Interrupts::initInterrupts();
		MessageReceiver::init();

		if (Display::getSingletonPtr() == nullptr)
			pDisplay = new Display();

		if (TimerManager::getSingletonPtr() == nullptr)
			pTimerManager = new TimerManager();

		if (SerialPortManager::getSingletonPtr() == nullptr)
			pSerialPortManager = new SerialPortManager();

		if (HddManager::getSingletonPtr() == nullptr)
			pHddManager = new HddManager();

		pSerialDebug = new SerialDebug(); // здесь !!!!!!!!!!!!!!!!!!!!!! в debug.cpp закоментирована строка в конструкторе

		if (Touchpad::getSingletonPtr() == nullptr)
			pTouchpad = new Touchpad();

		Cpu::sti();

		if (Log::getSingletonPtr() == nullptr)
			pLog = new Log();

		if (Clock::getSingletonPtr() == nullptr)
			pClock = new Clock();

		if (Config::getSingletonPtr() == nullptr)
			pConfig = new Config();

print("root2", 40, getNextPos());
Cpu::cli();
for(;;);
		while (!Config::getSingleton().getConfigData()->isReady()){}  // здесь !!!!!!!!!!!!!!!!!!!!!!
print("root3", 40, getNextPos());
Cpu::cli();
for(;;);

		Bk16Device::registerDevice();

//		bool scannerDev = false;

		//if (Config::getSingleton().getConfigData()->isTvExistsInConfig())
		//	TvDevice::registerDevice();
		//else
		//{
		//	if (Config::getSingleton().getConfigData()->isFv300ExistsInConfig())
		//		Fv300Device::registerDevice();
		//	else
		//	{
//				scannerDev = true;
//				ScannerDevice::registerDevice();
		//	}
		//}

		RpkDevice::registerDevice();
		MonitoringDevice::registerDevice();

		ExtensionSystem::getSingleton().init();

//		if (scannerDev)
//			ExtensionSystem::getSingleton().forceAddDevice(2, ScannerDevice::ID, 30);

		if (Povorotniy::getSingletonPtr() == nullptr)
			pPovorotniy = new Povorotniy();

		if (UI::getSingletonPtr() == nullptr)
			pUI = new UI();

		if (Application::getSingletonPtr() == nullptr)
			pApplication = new Application();
	}

	//! ����������.
	/*!
		������� ��� ��������� �������.
	*/
	~Root(){
		SAFE_DELETE(pApplication)
		SAFE_DELETE(pUI)
		SAFE_DELETE(pPovorotniy)
		if (ExtensionSystem::getSingletonPtr() != nullptr)
			delete ExtensionSystem::getSingletonPtr();
		SAFE_DELETE(pConfig)
		SAFE_DELETE(pClock)

	delete pSerialDebug;

		SAFE_DELETE(pLog)
		SAFE_DELETE(pTouchpad)
		SAFE_DELETE(pHddManager)
		SAFE_DELETE(pSerialPortManager)
		SAFE_DELETE(pTimerManager)
		SAFE_DELETE(pDisplay)
	}
};

#endif
