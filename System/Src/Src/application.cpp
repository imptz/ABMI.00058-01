#include "application.h"
#include "messages.h"
#include "ui.h"
#include "extensionSystem.h"
#include "debug.h"
#include "local.h"
#include "action_dummy.h"
#include "device_monitoring.h"
#include "povorotniy.h"

Application::Application()
	:	ITimer(TIMER_PERIOD), logic(nullptr){
	addReceiver(UI::getSingleton().getMainTabControl());
	UI::getSingleton().getMainTabControl()->addReceiver(this);

	pTimer->start();
}

Application::~Application(){
	delete logic;
}

void Application::createLogic(){
	logic = new Logic(this);
	addReceiver(logic);
}

int taskId = -1;
const unsigned int SIZE = 1024 * 64 * 2;
unsigned char* ab;

void Application::start(){
	Display::getSingleton().print(LOCAL_MESSAGE_TEXT_INITIALIZATION, 33, 7, true);

	while (!ExtensionSystem::getSingleton().isReady()){
		ExtensionSystem::getSingleton().action();
		MessageReceiver::messagesProccess();
	}

	createLogic();

	Display::getSingleton().print("                     ", 33, 7, false);
	UI::getSingleton().getUsoModeControl()->setEnabled(true);

	ActionDummy actionDummy; // ��� ������ ���� ����� ������ ������ Action, ����� �������� ����������� � debug

	for (;;){
		ExtensionSystem::getSingleton().action();
		logic->threadAction();
		Config::getSingleton().action();
		Povorotniy::getSingleton().threadAction();
		MessageReceiver::messagesProccess();
	}
}

void Application::onMessage(Message message){
	if ((message.from == MESSAGE_FROM_OFFSET_LOGIC) && (message.msg == Logic::LOGIC_MESSAGE_GET_CONFIRMATION)){
		UI::getSingleton().getMainTabControl()->setOwner(static_cast<MainConfirmation::CONFIRMATION_OWNER>(message.par2));
		UI::getSingleton().getMainTabControl()->setConfirmationText(reinterpret_cast<char*>(message.par1));
		UI::getSingleton().getMainTabControl()->activateConfirmationTab();
	}

	if ((message.from == MESSAGE_FROM_OFFSET_CONTROLS + UI::getSingleton().getMainTabControl()->getId()) && (message.msg == MainConfirmation::CONFIRMATION_MESSAGE_RESULT)){
		sendMessage(Message(MESSAGE_FROM_OFFSET_APPLICATION, MainConfirmation::CONFIRMATION_MESSAGE_RESULT, message.par1, message.par2));
	}

	if ((message.from == MESSAGE_FROM_OFFSET_LOGIC) && (message.msg == Logic::LOGIC_MESSAGE_GET_FINISH)){
		UI::getSingleton().getMainTabControl()->activateFinishTab();
	}

	if ((message.from == MESSAGE_FROM_OFFSET_CONTROLS + UI::getSingleton().getMainTabControl()->getId()) && (message.msg == MainFinish::FINISH_MESSAGE_RESULT)){
		sendMessage(Message(MESSAGE_FROM_OFFSET_APPLICATION, MainFinish::FINISH_MESSAGE_RESULT, 0, 0));
	}

	if ((message.from == MESSAGE_FROM_OFFSET_LOGIC) && (message.msg == MainTabControl::MAIN_TAB_MESSAGE_SET_MAIN_TAB)){
		UI::getSingleton().getMainTabControl()->activateMainTab();
	}

	if ((message.from == MESSAGE_FROM_OFFSET_LOGIC) && (message.msg == MainFinish::FINISH_MESSAGE_LABEL)){
		sendMessage(Message(MESSAGE_FROM_OFFSET_APPLICATION, MainFinish::FINISH_MESSAGE_LABEL, message.par1, 0));
	}
}

void Application::timerHandler(){
	MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_DUMMY, 0, 0, 0, 0);
}
