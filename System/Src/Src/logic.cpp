#include "logic.h"
#include "memory.h"
#include "subsystem_io.h"
#include "ui.h"
#include "subsystem_monitoring.h"
#include "subsystem_detection.h"
#include "local.h"
#include "debug.h"
#include "display.h"

const char* Logic::CONFIRMATION_START_SEARCH_TEXT = LOCAL_CONFIRMATION_USO_LOGIC_SEARCH_TEXT;
const char* Logic::CONFIRMATION_START_OROSHENIE_TEXT = LOCAL_CONFIRMATION_USO_LOGIC_COOLING_TEXT;
const char* Logic::START_SEARCH_LOG_TEXT = LOCAL_LOGIC_SEARCHING_START_LOG_TEXT;
const char* Logic::START_OROSHENIE_LOG_TEXT = LOCAL_LOGIC_COOLING_START_LOG_TEXT;
const char* Logic::CANCEL_SEARCH_LOG_TEXT = LOCAL_LOGIC_SEARCHING_CANCEL_LOG_TEXT;
const char* Logic::CANCEL_OROSHENIE_LOG_TEXT = LOCAL_LOGIC_COOLING_CANCEL_LOG_TEXT;
const char* Logic::FAULT_OROSHENIE_LOG_TEXT = LOCAL_LOGIC_COOLING_LOG_FAULT_TEXT;
const char* Logic::FINISH_OROSHENIE_LOG_TEXT = LOCAL_LOGIC_COOLING_FINISH_LOG_TEXT;

int Logic::findSignalInPrList(int _signal, unsigned int startIndex){
    for (unsigned int i = startIndex; i < prList.getSize(); ++i)
        if (prList[i].signalIndex == _signal)
            return i;

    return -1;
}

int Logic::findPrNumberInPrList(unsigned int _prNumber){
    for (unsigned int i = 0; i < prList.getSize(); ++i)
        if (prList[i].number == _prNumber)
            return i;

    return -1;
}

Logic::Logic(MessageReceiver* _messageReceiver)
	:	Fr::Thread<Logic>(&Logic::init), ITimer(TIMER_PERIOD),
	listProgramIndex(nullptr), listProgramIndexCount(0),
	timeOutBeforeStart(0),
	mainFunction(LOGIC_FUNCTION_UNDEFINED),
	userStartConfirmationResult(USER_START_CONFIRMATION_RESULT_UNDEFINED),
	fEnableAllPumpStationOutputs(false),
	fUserFinish(false),
	timeOutWaiting(TIME_OUT_WAITING_UNDEFINED){
	pTimer->start();
	addReceiver(_messageReceiver);
	DetectionSubsystem::getSingleton().addReceiver(this);
	debug = false;
	SerialDebug::getSingleton().addReceiver(this);
}

Logic::~Logic(){}

void Logic::timerHandler(){
	if (timeOutBeforeStart != 0)
		timeOutBeforeStart--;

	if (timeOutWaiting != TIME_OUT_WAITING_UNDEFINED){
		if (timeOutWaiting != 0)
			timeOutWaiting--;
	}

	logicTimer.action();
}

int Logic::getActiveSignal(LOGIC_FUNCTION ignorableFunction){
	ConfigDataStructInitSignal** pData = Config::getSingleton().getConfigData()->getConfigDataStructInitSignals();

	for (unsigned int i = 0; i < Config::getSingleton().getConfigData()->getConfigDataStructInitSignalsCount(); ++i){
		if (testInitSignal(pData[i]->number)){
			if ((!pData[i]->ignorable) &&
				(signals.getItemIndex(pData[i]->number) == -1) &&
				(Config::getSingleton().getConfigData()->getConfigDataStructProgramType(pData[i]->number) != ignorableFunction)){
				SAFE_DELETE_ARRAY(listProgramIndex)
				listProgramIndexCount = Config::getSingleton().getConfigData()->getConfigDataStructProgramInitSignal(pData[i]->number, &listProgramIndex);

				if (listProgramIndexCount > 0)
					return pData[i]->number;
				else
					pData[i]->ignorable = true;
			}
			else
				continue;
		}
		else
			pData[i]->ignorable = false;
	}

	return -1;
}

bool Logic::testInitSignal(int number){
	ConfigDataStructInitSignal** pData = Config::getSingleton().getConfigData()->getConfigDataStructInitSignals();

	int index = Config::getSingleton().getConfigData()->getConfigDataStructInitSignalsIndexByNumber(number);

	if (index == -1)
		return false;

	IIODevice::INPUT_STATE signal0 = IOSubsystem::getSingleton().getInput(pData[index]->firstInputNumber);
	IIODevice::INPUT_STATE signal1 = IOSubsystem::getSingleton().getInput(pData[index]->secondInputNumber);
	IIODevice::INPUT_STATE signal2 = IOSubsystem::getSingleton().getInput(pData[index]->thirdInputNumber);

	bool result = true;

	if (pData[index]->firstInputNumber != 0)
		if (signal0 != IIODevice::INPUT_STATE_ON)
			result = false;

	if (pData[index]->secondInputNumber != 0)
		if (signal1 != IIODevice::INPUT_STATE_ON)
			result = false;

	if (pData[index]->thirdInputNumber != 0)
		if (signal2 != IIODevice::INPUT_STATE_ON)
			result = false;

	return result;
}

void Logic::setInitSignalIgnorable(int number, bool value){
	ConfigDataStructInitSignal** pData = Config::getSingleton().getConfigData()->getConfigDataStructInitSignals();

	int index = Config::getSingleton().getConfigData()->getConfigDataStructInitSignalsIndexByNumber(number);
	if (index != -1)
		pData[index]->ignorable = value;
}

void Logic::onMessage(Message message){
	if (message.from == MESSAGE_FROM_OFFSET_SERIAL_DEBUG){
		if (message.msg == SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND)
			if (message.par1 == SerialDebug::COMMAND_DEBUG_LOGIC){
				if (!debug){
					debug = true;
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "Logic DEBUG ON\n")
				}
				else{
					debug = false;
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "Logic DEBUG OFF\n")
				}
			}
	}
	else{
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("message: from = %u, msg = %u, par1 = %u, par2 = %u\n", message.from, message.msg, message.par1, message.par2)

		switch (message.msg){
			case MainConfirmation::CONFIRMATION_MESSAGE_RESULT:
				if (message.par2 == MainConfirmation::CONFIRMATION_OWNER_1){
					timeOutWaiting = TIME_OUT_WAITING_UNDEFINED;
					if (message.par1 == MainConfirmation::CONFIRMATION_RESULT_YES)
						userStartConfirmationResult = USER_START_CONFIRMATION_RESULT_TRUE;
					else
						userStartConfirmationResult = USER_START_CONFIRMATION_RESULT_FALSE;
				}
				break;
			case MainFinish::FINISH_MESSAGE_RESULT:
				fUserFinish = true;
				break;
			default:
				break;
		}
	}
}

Fr::CPointer<Logic> Logic::init(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::init()\n")
		dbg = false;
	}

	UI::getSingleton().getUsoModeControl()->unLock();

	mainFunction = LOGIC_FUNCTION_UNDEFINED;

	signals.clear();

	userStartConfirmationResult = USER_START_CONFIRMATION_RESULT_UNDEFINED;

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			SAFE_DELETE(prList[i].action)

	prList.clear();

	fEnableAllPumpStationOutputs = false;
	fUserFinish = false;

	dbg = true;
	return &Logic::inputsControl;
}

Fr::CPointer<Logic> Logic::inputsControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::inputsControl()\n")
		dbg = false;
	}

	int initSignal = getActiveSignal();

	if (initSignal != -1){
		if (UI::getSingleton().getUsoModeControl()->getMode() != UsoModeControl::USO_MODE_TOOLS){
			signals.add(initSignal);
			timeOutBeforeStart = Config::getSingleton().getConfigData()->getConfigDataStructConst()->timeOutBeforeStart;
			dbg = true;
			return &Logic::inputsPauseAndConfirmation;
		}
	}

	return &Logic::inputsControl;
}

Fr::CPointer<Logic> Logic::inputsPauseAndConfirmation(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::inputsPauseAndConfirmation()\n")
		dbg = false;
	}

	if (timeOutBeforeStart == 0){
		if (testInitSignal(signals[0])){
			mainFunction = Config::getSingleton().getConfigData()->getConfigDataStructProgramType(signals[0]);

			if (UI::getSingleton().getUsoModeControl()->lock()){
				MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_SIGNAL_O_VOZGORANII, 0, signals[0]);

				if (UI::getSingleton().getUsoModeControl()->getMode() == UsoModeControl::USO_MODE_HALF_AUTO){
					userStartConfirmationResult = USER_START_CONFIRMATION_RESULT_UNDEFINED;

					startWaitingConf();

					switch (mainFunction){
						case LOGIC_FUNCTION_SEARCHING:
							sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, LOGIC_MESSAGE_GET_CONFIRMATION, reinterpret_cast<unsigned int>(CONFIRMATION_START_SEARCH_TEXT), MainConfirmation::CONFIRMATION_OWNER_1));
							break;
						case LOGIC_FUNCTION_COOLING_LINE:
						case LOGIC_FUNCTION_COOLING_POINT:
							sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, LOGIC_MESSAGE_GET_CONFIRMATION, reinterpret_cast<unsigned int>(CONFIRMATION_START_OROSHENIE_TEXT), MainConfirmation::CONFIRMATION_OWNER_1));
							break;
						default:
							dbg = true;
							return  &Logic::init;
							break;
					}

					dbg = true;
					return &Logic::userStart;
				}
				else
					if (UI::getSingleton().getUsoModeControl()->getMode() == UsoModeControl::USO_MODE_FULL_AUTO){
						dbg = true;
						return &Logic::machineStart;
					}
			}
		}

		dbg = true;
		return  &Logic::init;
	}

	return &Logic::inputsPauseAndConfirmation;
}

Fr::CPointer<Logic> Logic::machineStart(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::machineStart()\n")
		dbg = false;
	}

	switch (mainFunction){
		case LOGIC_FUNCTION_SEARCHING:
			Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, START_SEARCH_LOG_TEXT, START_ACTOR_FULL_AUTO, signals[0]);
			MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_START_SEARCH, START_ACTOR_FULL_AUTO, signals[0]);
			break;
		case LOGIC_FUNCTION_COOLING_LINE:
		case LOGIC_FUNCTION_COOLING_POINT:
			Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, START_OROSHENIE_LOG_TEXT, START_ACTOR_FULL_AUTO, signals[0]);
			MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_START_OROSHENIA, START_ACTOR_FULL_AUTO, signals[0]);
			break;
		default:
			break;
	}

	dbg = true;
	return &Logic::start;
}

Fr::CPointer<Logic> Logic::userStart(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::userStart()\n")
		dbg = false;
	}

	if (timeOutWaiting == 0){
		DEBUG_PUT_METHOD("PHASE_WAITING_CONFIRMATION ... timeOutWaiting == 0\n");
		timeOutWaiting = TIME_OUT_WAITING_UNDEFINED;
		UI::getSingleton().getUsoModeControl()->setMode(UsoModeControl::USO_MODE_FULL_AUTO, UsoModeControl::USO_MODE_CONTROL_ACTOR_TIME_OUT, true);
		UI::getSingleton().getMainTabControl()->activateMainTab();
		return &Logic::machineStart;
	}

	if (userStartConfirmationResult == USER_START_CONFIRMATION_RESULT_UNDEFINED)
		return &Logic::userStart;

	if (userStartConfirmationResult == USER_START_CONFIRMATION_RESULT_TRUE){
		switch (mainFunction){
			case LOGIC_FUNCTION_SEARCHING:
				Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, START_SEARCH_LOG_TEXT, START_ACTOR_HALF_AUTO, signals[0]);
				MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_START_SEARCH, START_ACTOR_HALF_AUTO, signals[0]);
				break;
			case LOGIC_FUNCTION_COOLING_LINE:
			case LOGIC_FUNCTION_COOLING_POINT:
				Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, START_OROSHENIE_LOG_TEXT, START_ACTOR_HALF_AUTO, signals[0]);
				MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_START_OROSHENIA, START_ACTOR_HALF_AUTO, signals[0]);
				break;
			default:
				break;
		}

		dbg = true;
		return &Logic::start;
	}
	else{
		switch (mainFunction){
			case LOGIC_FUNCTION_SEARCHING:
				Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, CANCEL_SEARCH_LOG_TEXT, START_ACTOR_HALF_AUTO, signals[0]);
				MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_OTMENA_SIGNALA_O_VOZGORANII, START_ACTOR_HALF_AUTO, signals[0]);
				break;
			case LOGIC_FUNCTION_COOLING_LINE:
			case LOGIC_FUNCTION_COOLING_POINT:
				Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, CANCEL_OROSHENIE_LOG_TEXT, START_ACTOR_HALF_AUTO, signals[0]);
				MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_OTMENA_SIGNALA_O_VOZGORANII, START_ACTOR_HALF_AUTO, signals[0]);
				break;
			default:
				break;
		}

		setInitSignalIgnorable(signals[0], true);
		dbg = true;
		return &Logic::init;
	}
}

Fr::CPointer<Logic> Logic::start(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::start()\n")
		dbg = false;
	}

	IOSubsystem::getSingleton().enableAllFireAlarmOutputs();
	IOSubsystem::getSingleton().enableAllHardwareOutputs();

	switch (mainFunction){
		case LOGIC_FUNCTION_SEARCHING:
			return &Logic::search_start;
			break;
		case LOGIC_FUNCTION_COOLING_LINE:
		case LOGIC_FUNCTION_COOLING_POINT:
			return &Logic::cooling_start;
			break;
		default:
			break;
	}

	return &Logic::init;
}

Fr::CPointer<Logic> Logic::search_start(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_METHOD("������� �� Logic::search_start()\n")
		dbg = false;
	}

	DetectionSubsystem::getSingleton().searchFire(listProgramIndex, listProgramIndexCount);

	return &Logic::search_startControl;
}

Fr::CPointer<Logic> Logic::search_startControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_METHOD("������� �� Logic::search_startControl()\n")
		dbg = false;
	}

	return &Logic::search_startControl;
}

Fr::CPointer<Logic> Logic::cooling_start(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::cooling_start()\n")
		dbg = false;
	}

	ConfigDataStructProgram** pPrograms = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();

	for (unsigned int i = 0; i < listProgramIndexCount; ++i){
		prList.add(Pr(signals[0], pPrograms[listProgramIndex[i]]->prNumber, pPrograms[listProgramIndex[i]]->function));
		DEBUG_IF_PUT_METHOD("prList.add(%u, %u, %u)\n", signals[0], pPrograms[listProgramIndex[i]]->prNumber, pPrograms[listProgramIndex[i]]->function)
		switch (pPrograms[listProgramIndex[i]]->function){
			case LOGIC_FUNCTION_COOLING_LINE:
				prList[i].action = new ActionStartProgramScanLine(pPrograms[listProgramIndex[i]]->point1,
					pPrograms[listProgramIndex[i]]->point2,
					pPrograms[listProgramIndex[i]]->nasadok,
					SCAN_PROGRAM_BALLISTICS_OFF, prList[i].address);
				break;
			case LOGIC_FUNCTION_COOLING_POINT:
				prList[i].action = new ActionStartProgramScanPoint(pPrograms[listProgramIndex[i]]->nPointProgram, prList[i].address);
				break;
			default:
				break;
		}
	}

	return &Logic::cooling_startControl;
}

Fr::CPointer<Logic> Logic::cooling_startControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::cooling_startControl()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				default:
					break;
			}
	}

	if (finish){
		unsigned int readyCount = 0;

		unsigned int i = 0;
		bool process = true;
		while (process){
			if (i < prList.getSize()){
				switch (prList[i].action->getState()){
					case Action::STATE_READY:
						readyCount++;
						i++;
						break;
					case Action::STATE_ERROR:
						prList.del(i);  // ��� ���� ����������� ����� ������� �������
						break;
					default:
						break;
				}
			}
			else
				process = false;
		}

		if (readyCount == 0){
			IOSubsystem::getSingleton().disableAllFireAlarmOutputs();
			IOSubsystem::getSingleton().disableAllHardwareOutputs();
			IOSubsystem::getSingleton().disableAllPumpStationOutputs();

			for (unsigned int i = 0; i < prList.getSize(); ++i)
				if (prList[i].action != nullptr)
					SAFE_DELETE(prList[i].action)

			Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, FAULT_OROSHENIE_LOG_TEXT, 0, 0);

			return &Logic::init;
		}
		else
			return &Logic::gateOpen;
	}

	return &Logic::cooling_startControl;
}

Fr::CPointer<Logic> Logic::gateOpen(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::gateOpen()\n")
		dbg = false;
	}

	fEnableAllPumpStationOutputs = false;

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr){
			if (prList[i].action->getState() == Action::STATE_READY){
				SAFE_DELETE(prList[i].action)
				prList[i].action = new ActionGateOpen(prList[i].address);
			}
			else
				SAFE_DELETE(prList[i].action)
		}

	return &Logic::gateOpenControl;
}

Fr::CPointer<Logic> Logic::gateOpenControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::gateOpenControl()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				case Action::STATE_READY:
					if (!fEnableAllPumpStationOutputs){
						IOSubsystem::getSingleton().enableAllPumpStationOutputs();
						fEnableAllPumpStationOutputs = true;
					}
					break;
				default:
					break;
			}
	}

	if (finish){
		for (unsigned int i = 0; i < prList.getSize(); ++i)
			if (prList[i].action != nullptr)
				SAFE_DELETE(prList[i].action)

		return &Logic::startPhase2;
	}

	return &Logic::gateOpenControl;
}

Fr::CPointer<Logic> Logic::startPhase2(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::startPhase2()\n")
		dbg = false;
	}

	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainFinish::FINISH_MESSAGE_LABEL, MainFinish::FINISH_MESSAGE_PARAM_START, 0));
	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, LOGIC_MESSAGE_GET_FINISH, 0, 0));

	return &Logic::phase2ControlOffSignal;
}

Fr::CPointer<Logic> Logic::phase2ControlOffSignal(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2ControlOffSignal()\n")
		dbg = false;
	}

	bool signalOff = false;
	bool proccess = true;

	while (proccess){
		proccess = false;
		for (unsigned int i = 1; i < signals.getSize(); ++i)
			if (!testInitSignal(signals[i])){
				int index = findSignalInPrList(signals[i], 0);
				while (index != -1){
					if (prList[index].action != nullptr)
						SAFE_DELETE(prList[index].action)

					prList[index].action = new ActionStopProgramScan(prList[index].address);
					signalOff = true;

					index = findSignalInPrList(signals[i], index + 1);
				}

				signals.del(i);
				proccess = true;
				break;
			}
	}

	if (signalOff)
		return &Logic::phase2StopPr;
	else
		return &Logic::phase2TimerUserFinish;
}

Fr::CPointer<Logic> Logic::phase2StopPr(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2StopPr()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				default:
					break;
			}
	}

	if (finish){
		//unsigned int i = 0;
		//bool process = true;
		//while (process)
		//{
		//	if (i < prList.getSize())
		//	{
		//		switch (prList[i].action->getState())
		//		{
		//			case Action::STATE_ERROR:
		//				// ������� ������ ��� ������ �� �������. ���� ������� ��� ������ ��, �� ������� �� ����������� ����, ���� ����� ��� �������
		//				//prList.del(i);
		//				break;
		//			default:
		//				++i;
		//				break;
		//		}
		//	}
		//	else
		//		process = false;
		//}

		return &Logic::phase2CloseGate;
	}

	return &Logic::phase2StopPr;
}

Fr::CPointer<Logic> Logic::phase2CloseGate(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2CloseGate()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr){
			SAFE_DELETE(prList[i].action)
			prList[i].action = new ActionGateClose(prList[i].address);
		}
	}

	return &Logic::phase2CloseGateControl;
}

Fr::CPointer<Logic> Logic::phase2CloseGateControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2CloseGateControl()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				default:
					break;
			}
	}

	if (finish){
		for (unsigned int i = 0; i < prList.getSize(); ++i){
			if (prList[i].action != nullptr){
				SAFE_DELETE(prList[i].action)
				prList.del(i);
			}
		}

		return &Logic::phase2ControlOffSignal;
	}

	return &Logic::phase2CloseGateControl;
}

Fr::CPointer<Logic> Logic::phase2TimerUserFinish(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2TimerUserFinish()\n")
		dbg = false;
	}

	if (fUserFinish)
		return &Logic::startDown;

	if (UI::getSingleton().getUsoModeControl()->getMode() == UsoModeControl::USO_MODE_FULL_AUTO){
		if ((!testInitSignal(signals[0])) && (signals.getSize() == 1)){
			switch (logicTimer.getState()){
				case LogicTimer::STATE_STOP:
					logicTimer.start(Config::getSingleton().getConfigData()->getConfigDataStructConst()->timeOutBeforeFinish);
					break;
				case LogicTimer::STATE_EXPIRED:
					return &Logic::startDown;
				default:
					break;
			}

			return &Logic::phase2TimerUserFinish;
		}
	}

	return &Logic::phase2StartNewPr;
}

Fr::CPointer<Logic> Logic::phase2StartNewPr(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_METHOD("������� �� Logic::phase2StartNewPr()\n")
		dbg = false;
	}

	bool newStart = false;

	int signal = getActiveSignal(LOGIC_FUNCTION_SEARCHING);

	if (signal != -1){
		DEBUG_IF_PUT_METHOD("����� ������ - %u\n", signal)
		ConfigDataStructProgram** pPrograms = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();
		for (unsigned int i = 0; i < listProgramIndexCount; ++i){
			int index = findPrNumberInPrList(pPrograms[listProgramIndex[i]]->prNumber);

			for(unsigned int a= 0; a < prList.getSize(); ++a)
				DEBUG_IF_PUT_METHOD("prList[%u].number = %u\n", a, prList[a].number)

			if (index != -1){
				// �� ��� ��������
				DEBUG_IF_PUT_METHOD("�� - %u ��� ��������\n", prList[index].number)
				if (prList[index].function == LOGIC_FUNCTION_SEARCHING){
					if (getPrStartCount(LOGIC_FUNCTION_SEARCHING) > 1){
						//	��������������� �� ����� ���������
					}
					else{
						//	�� ���������������
					}
				}
				else{
					//�� ���������������
					DEBUG_IF_PUT_METHOD("�� �� ���������������, �.�. ��� �������� �� ��������� ����������\n")
				}
			}
			else{
				DEBUG_IF_PUT_METHOD("�� - %u ��� �� ��������\n", pPrograms[listProgramIndex[i]]->prNumber)
				if (!isMaximumPrStart()){
					signals.add(signal);
					prList.add(Pr(signals[signals.getSize() - 1], pPrograms[listProgramIndex[i]]->prNumber, pPrograms[listProgramIndex[i]]->function));

					switch (pPrograms[listProgramIndex[i]]->function){
						case LOGIC_FUNCTION_COOLING_LINE:
							DEBUG_IF_PUT_METHOD("����� ��������� ���������� �� - %u\n", pPrograms[listProgramIndex[i]]->prNumber)
							prList[prList.getSize() - 1].action = new ActionStartProgramScanLine(pPrograms[listProgramIndex[i]]->point1,
								pPrograms[listProgramIndex[i]]->point2,
								pPrograms[listProgramIndex[i]]->nasadok,
								SCAN_PROGRAM_BALLISTICS_OFF, prList[prList.getSize() - 1].address);
							break;
						case LOGIC_FUNCTION_COOLING_POINT:
							DEBUG_IF_PUT_METHOD("����� ���������� �� ���������� �� - %u\n", pPrograms[listProgramIndex[i]]->prNumber)
							prList[prList.getSize() - 1].action = new ActionStartProgramScanPoint(pPrograms[listProgramIndex[i]]->nPointProgram, prList[prList.getSize() - 1].address);
							break;
						default:
							break;
					}

					newStart = true;
				}
				else{
					DEBUG_IF_PUT_METHOD("��� �������� ������������ ���������� ��\n")
					if (getPrStartCount(LOGIC_FUNCTION_SEARCHING) > 1){
						DEBUG_IF_PUT_METHOD("�� ������� ����� ������ �� - ���������� ������ �� ��\n")
						// - ���� �� �� �� ������� �����������, ��� �������� �����������
						// - �� �����������
					}
					else{
						DEBUG_IF_PUT_METHOD("�� ������� �� ����� ������ �� - ������ �� ���������������\n")
						// �� �����������
					}
				}
			}
		}

		setInitSignalIgnorable(signal, true);
	}

	if (newStart)
		return &Logic::phase2StartNewPrControl;
	else
		return &Logic::phase2ControlOffSignal;
}

Fr::CPointer<Logic> Logic::phase2StartNewPrControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2StartNewPrControl()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				default:
					break;
			}
	}

	if (finish)
		return &Logic::phase2GateOpen;

	return &Logic::phase2StartNewPrControl;
}

Fr::CPointer<Logic> Logic::phase2GateOpen(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2GateOpen()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr){
			if (prList[i].action->getState() == Action::STATE_READY){
				SAFE_DELETE(prList[i].action)
				prList[i].action = new ActionGateOpen(prList[i].address);
			}
			else
				SAFE_DELETE(prList[i].action)
		}

	return &Logic::phase2GateOpenControl;
}

Fr::CPointer<Logic> Logic::phase2GateOpenControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::phase2GateOpenControl()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				default:
					break;
			}
	}

	if (finish){
		for (unsigned int i = 0; i < prList.getSize(); ++i)
			if (prList[i].action != nullptr)
				SAFE_DELETE(prList[i].action)

		return &Logic::phase2ControlOffSignal;
	}

	return &Logic::phase2GateOpenControl;
}

Fr::CPointer<Logic> Logic::startDown(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::startDown()\n")
		dbg = false;
	}

	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainTabControl::MAIN_TAB_MESSAGE_SET_MAIN_TAB, 0, 0));

	IOSubsystem::getSingleton().disableAllFireAlarmOutputs();
	IOSubsystem::getSingleton().disableAllHardwareOutputs();
	IOSubsystem::getSingleton().disableAllPumpStationOutputs();

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			SAFE_DELETE(prList[i].action)

		prList[i].action = new ActionStopProgramScan(prList[i].address);
	}

	return &Logic::prStopControl;
}

Fr::CPointer<Logic> Logic::prStopControl(){
	static bool dbg = true;
	if (dbg){
			DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
			DEBUG_IF_PUT("������� �� Logic::prStopControl()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				default:
					break;
			}
	}

	if (finish){
		for (unsigned int i = 0; i < prList.getSize(); ++i)
			if (prList[i].action != nullptr)
				SAFE_DELETE(prList[i].action)

		return &Logic::gateClose;
	}

	return &Logic::prStopControl;
}

Fr::CPointer<Logic> Logic::gateClose(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::gateClose()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			SAFE_DELETE(prList[i].action)

		prList[i].action = new ActionGateClose(prList[i].address);
	}

	return &Logic::gateCloseControl;
}

Fr::CPointer<Logic> Logic::gateCloseControl(){
	static bool dbg = true;
	if (dbg){
		DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
		DEBUG_IF_PUT("������� �� Logic::gateCloseControl()\n")
		dbg = false;
	}

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if (prList[i].action != nullptr)
			prList[i].action->step();

	bool finish = true;

	for (unsigned int i = 0; i < prList.getSize(); ++i){
		if (prList[i].action != nullptr)
			switch (prList[i].action->getState()){
				case Action::STATE_UNDEFINED:
					finish = false;
					break;
				default:
					break;
			}
	}

	if (finish){
		for (unsigned int i = 0; i < prList.getSize(); ++i)
			if (prList[i].action != nullptr)
				SAFE_DELETE(prList[i].action)

		Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, FINISH_OROSHENIE_LOG_TEXT, 0, 0);

		return &Logic::init;
	}

	return &Logic::gateCloseControl;
}

unsigned int Logic::getPrStartCount(LOGIC_FUNCTION function){
	unsigned int count = 0;

	for (unsigned int i = 0; i < prList.getSize(); ++i)
		if ((prList[i].function == function) || (function == LOGIC_FUNCTION_ALL))
			count++;

	return count;
}

bool Logic::isMaximumPrStart(){
	return (getPrStartCount() >= Config::getSingleton().getConfigData()->getConfigDataStructConst()->maxPR);
}

int Logic::getConfigTimeOutWaiting(){
	int time = Config::getSingleton().getConfigData()->getConfigDataStructConst()->timeControlUserAction;

	if (time == 0)
		time = TIME_OUT_WAITING_UNDEFINED;

	DEBUG_PUT_METHOD("timeOut = %is\n", time);

	return time;
}

void Logic::startWaitingConf(){
	timeOutWaiting = getConfigTimeOutWaiting();
}

