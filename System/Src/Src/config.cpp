#include "config.h"
#include "crc32.h"
#include "debug.h"
#include "subsystem_rpk.h"

#include "lowDisplay.h"

Config::Config()
	:	ITimer(TIMER_PERIOD), pConfigData(new ConfigData(MAX_DATA_SIZE)), progress(-1), progressFR(-1), progressFRStep(0), serialPortName(SERIAL_PORT_1),
	 	serialPortSpeed(SERIAL_PORT_SPEED_57600), serialPort(SerialPortManager::getSingleton().getPort(serialPortName)), phase(CONFIG_PHASE_STOP), dataSize(0),
	 	loadDataSize(0), buffer(new unsigned char[MAX_DATA_SIZE]), prNumbers(nullptr), prNumbersCount(0), prNumbersIndex(0), frameId(0), frameId1(0), frameIdRes(0),
	 	frameId1Res(0), debug(false), phaseFR(CONFIG_PHASE_FR_STOP), numberPrWithTrajectories(0), prWithTrajectories(nullptr), prIndex(0), numberTrajectories(0),
	 	trajectoryIndex(0), pTrajectory(nullptr), trajectoryCount(0), pointCount(0), pPressureTable(nullptr), pressureTableCount(0),
	 	pressureTableCountc(0), pressureTableIndex(0), totalPoints(0), accumPointsCount(0)
{
print("Config::Config", 40, getNextPos());
printUInt(reinterpret_cast<unsigned int>(serialPort), 40, getNextPos());
	serialPort->setSpeed(serialPortSpeed);
	serialPort->open();
	SerialDebug::getSingleton().addReceiver(this);
}

Config::~Config()
{
	delete pConfigData;
}

void Config::onMessage(Message message)
{
	if (message.from == MESSAGE_FROM_OFFSET_SERIAL_DEBUG)
	{
		switch (message.msg)
		{
			case SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND:
				switch (message.par1)
				{
					case SerialDebug::COMMAND_DEBUG_CONFIG:
						if (!debug)
						{
							debug = true;
							DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "Config DEBUG ON\n")
						}
						else
						{
							debug = false;
							DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "Config DEBUG OFF\n")
						}
						break;
					case SerialDebug::COMMAND_GET_SETTINGS:
						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_YELLOW, SerialDebug::TEXT_ATTR_BOLD, "������������ ���:\n\n")

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ���������:\n\n")
						DEBUG_PUT("   ������������ ����� ��, ����������� � �������������:              %u\n", pConfigData->getConfigDataStructConst()->maxPR)
						DEBUG_PUT("   �������� ����� ������� ����� ���������:                          %u ���\n", pConfigData->getConfigDataStructConst()->timeOutBeforeStart)
						DEBUG_PUT("   �������� �� ��������� �������������:                             %u ���\n", pConfigData->getConfigDataStructConst()->timeOutBeforeFinish)
						DEBUG_PUT("   ���������� ������������� ���������� ��������� (��� �� �������):  %u\n", pConfigData->getConfigDataStructConst()->numberFireToAnalize)
						DEBUG_PUT("   ����������� ���������� ��� ���������� �����:                     %u �\n", pConfigData->getConfigDataStructConst()->minimumDistanceForCompactJet)
						DEBUG_PUT("   �������� ���������� ���� - X:                                    %f �\n", pConfigData->getConfigDataStructConst()->protectedZone.x / 100.0f)
						DEBUG_PUT("   �������� ���������� ���� - Y:                                    %f �\n\n", pConfigData->getConfigDataStructConst()->protectedZone.y / 100.0f)

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ��������� � ���������� ��:\n\n")
						for (unsigned int i = 0; i < pConfigData->getConfigDataStructPRPositionCount(); ++i)
						{
							DEBUG_PUT("   �����:                                     %u\n", pConfigData->getConfigDataStructPRPositions()[i]->projectNumber)
							DEBUG_PUT("   �����:                                     %u\n", pConfigData->getConfigDataStructPRPositions()[i]->address)
							DEBUG_PUT("   ���������� X:                              %f\n", pConfigData->getConfigDataStructPRPositions()[i]->position.x / 100.0f)
							DEBUG_PUT("   ���������� Y:                              %f\n", pConfigData->getConfigDataStructPRPositions()[i]->position.y / 100.0f)
							DEBUG_PUT("   ���������� Z:                              %f\n", pConfigData->getConfigDataStructPRPositions()[i]->position.z / 100.0f)
							DEBUG_PUT("   ���������� �� � �������������� ���������:  %f\n", pConfigData->getConfigDataStructPRPositions()[i]->orientation.x)
							DEBUG_PUT("   ���������� �� � ������������ ���������:    %f\n", pConfigData->getConfigDataStructPRPositions()[i]->orientation.y)
							DEBUG_PUT("   ���������� ����� �� ����������:            %u\n\n", pConfigData->getConfigDataStructPRPositions()[i]->networkIndexNumber)
						}

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ������������ ����� � ������ ��-16:\n\n")
						for (unsigned int i = 0; i < pConfigData->getConfigDataStructIOBk16Count(); ++i)
						{
							DEBUG_PUT("   ����� ���������� (��16):       %u\n", pConfigData->getConfigDataStructIOBk16()[i]->bkAddress)
							DEBUG_PUT("   ����� �����/������:            %u\n", pConfigData->getConfigDataStructIOBk16()[i]->numberOnDevice)
							DEBUG_PUT("   �������������� ������ ������:  %u\n", pConfigData->getConfigDataStructIOBk16()[i]->outputFunctionGroup)
							DEBUG_PUT("   ����� �� (��������� �������):  %u\n", pConfigData->getConfigDataStructIOBk16()[i]->projectNumber)
							DEBUG_PUT("   ������������� �����:           %u\n", pConfigData->getConfigDataStructIOBk16()[i]->prGateNumber)
							DEBUG_PUT("   ����� ����������:              %u\n\n", pConfigData->getConfigDataStructIOBk16()[i]->projectNumber)
						}

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ������ ������������ ��������:\n\n")
						DEBUG_PUT("   ���������� ��������:  %u\n\n", pConfigData->getConfigDataStructInitSignalsCount())
						for (unsigned int i = 0; i < pConfigData->getConfigDataStructInitSignalsCount(); ++i)
						{
							DEBUG_PUT("   ����� ������������� �������:                   %u\n", pConfigData->getConfigDataStructInitSignals()[i]->number)
							DEBUG_PUT("   ����� �������� ������� �������� ������������:  %u\n", pConfigData->getConfigDataStructInitSignals()[i]->firstInputNumber)
							DEBUG_PUT("   ����� �������� ������� �������� ������������:  %u\n\n", pConfigData->getConfigDataStructInitSignals()[i]->secondInputNumber)
						}

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ������� ��� �� ��� ������ ����� ��������� � ����������:\n\n")
						DEBUG_PUT("   ���������� �������:  %u\n\n", pConfigData->getConfigDataStructProgramCount())
						for (unsigned int i = 0; i < pConfigData->getConfigDataStructProgramCount(); ++i)
						{
							DEBUG_PUT("   ����� ������������� �������:    %u\n", pConfigData->getConfigDataStructPrograms()[i]->initSignalNumber)
							DEBUG_PUT("   ����� ��:                       %u\n", pConfigData->getConfigDataStructPrograms()[i]->prNumber)
							DEBUG_PUT("   �������:                        %u\n", pConfigData->getConfigDataStructPrograms()[i]->function)
							DEBUG_PUT("   1-� �������������� ����������:  %u\n", pConfigData->getConfigDataStructPrograms()[i]->point1.x)
							DEBUG_PUT("   1-� ������������ ����������:    %u\n", pConfigData->getConfigDataStructPrograms()[i]->point1.y)
							DEBUG_PUT("   2-� �������������� ����������:  %u\n", pConfigData->getConfigDataStructPrograms()[i]->point2.x)
							DEBUG_PUT("   2-� ������������ ����������:    %u\n", pConfigData->getConfigDataStructPrograms()[i]->point2.y)
							DEBUG_PUT("   ����� ����������:               %u\n\n", pConfigData->getConfigDataStructPrograms()[i]->nPointProgram)
						}

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ���������� FV300:\n\n")
						DEBUG_PUT("   ���������� �����������:  %u\n\n", pConfigData->getConfigDataStructFv300Count())
						for (unsigned int i = 0; i < pConfigData->getConfigDataStructFv300Count(); ++i)
						{
							DEBUG_PUT("   ����� ����������:  %u\n", pConfigData->getConfigDataStructFv300()[i]->address)
							DEBUG_PUT("   ����� ��:          %u\n", pConfigData->getConfigDataStructFv300()[i]->prNumber)
							DEBUG_PUT("   ����� ����������:  %u\n\n", pConfigData->getConfigDataStructFv300()[i]->projectNumber)
						}

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ���������� ��:\n\n")
						DEBUG_PUT("   ���������� ����� ���� ����������:  %u\n\n", pConfigData->getConfigDataStructTrajectoryCount())
						//for (unsigned int i = 0; i < pConfigData->getConfigDataStructTrajectoryCount(); ++i)
						//{
						//	if ((pConfigData->getConfigDataStructTrajectory()[i]->prNumber == 5) && (pConfigData->getConfigDataStructTrajectory()[i]->pressureNumber > 6))
						//	{
						//		DEBUG_PUT("   ��:      %u ", pConfigData->getConfigDataStructTrajectory()[i]->prNumber)
						//		DEBUG_PUT("����� �������: %u ", pConfigData->getConfigDataStructTrajectory()[i]->pressureNumber)
						//		DEBUG_PUT("����� �����: %u\n\n", pConfigData->getConfigDataStructTrajectory()[i]->pointNumber)
						//	}
						//}

						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_WHITE, SerialDebug::TEXT_ATTR_BOLD, "  ������� ��������:\n\n")
						DEBUG_PUT("   ���������� ������:  %u\n\n", pConfigData->getConfigDataStructPressureCount())
						//for (unsigned int i = 0; i < pConfigData->getConfigDataStructPressureCount(); ++i)
						//{
						//	if (pConfigData->getConfigDataStructPressure()[i]->prNumber == 5)
						//	{
						//		DEBUG_PUT("   ��:      %u ", pConfigData->getConfigDataStructPressure()[i]->prNumber)
						//		DEBUG_PUT("����� �������: %u ", pConfigData->getConfigDataStructPressure()[i]->arNumber)
						//		DEBUG_PUT("��������: %u ", pConfigData->getConfigDataStructPressure()[i]->pressure)
						//		DEBUG_PUT("����: %u\n\n", pConfigData->getConfigDataStructPressure()[i]->delta)
						//	}
						//}
					break;
				}
			break;
		}
	}
}

ConfigData* Config::getConfigData()
{
	return pConfigData;
}

int Config::getUpdateProgress()
{
	return progress;
}

int Config::getUpdateProgressFR()
{
	return progressFR;
}

void Config::startUpdate()
{
	phase = CONFIG_PHASE_CONNECTION;
	progress = 0;
	serialPort->getRecvFifo()->clear();
}

void Config::stopUpdate()
{
	phase = CONFIG_PHASE_STOP;
	progress = 0;
	serialPort->getRecvFifo()->clear();
}

void Config::timerHandler()
{
}

void Config::sendMessageToPort(unsigned char* pData, unsigned int size)
{
	if (phase == CONFIG_PHASE_STOP)
	{
		serialPort->setNewSendData(pData, size);
		serialPort->startSend();
	}
}

void Config::action()
{
	pcAction();
	frAction();
}

void Config::pcAction()
{
	unsigned int dLen;

	switch (phase)
	{
		case CONFIG_PHASE_CONNECTION:
			if (serialPort->getRecvFifo()->getDataSize() >= 4)
			{
				unsigned int data[2];
				serialPort->getRecvFifo()->get(reinterpret_cast<unsigned char*>(&data[1]), 4);
				if ((data[1] & 0x0000ffff) == DATA_VERSION)
				{
					data[0] = CONNECT_ANSWER_CODE;
					unsigned int yr = data[1] & 0xffff0000;
					data[1] = yr + DATA_VERSION;
					serialPort->setNewSendData(reinterpret_cast<unsigned char*>(&data[0]), 8);

					serialPort->startSend();
					phase = CONFIG_PHASE_LENGTH;
				}
				else
				{
					data[0] = DOWNLOAD_RESULT_FAULT;
					unsigned int yr = data[1] & 0xffff0000;
					data[1] = yr + DATA_VERSION;
					serialPort->setNewSendData(reinterpret_cast<unsigned char*>(&data[0]), 8);
					serialPort->startSend();
					phase = CONFIG_PHASE_DISCONNECT_AFTER_ERROR;
					sendMessage(Message(MESSAGE_FROM_OFFSET_CONFIG, CONFIG_MESSAGE_UPDATE_ERROR, 0, CONFIG_PHASE_CONNECTION));
				}
			}
			break;
		case CONFIG_PHASE_LENGTH:
			if (serialPort->getRecvFifo()->getDataSize() >= LENGTH_AREA_SIZE)
			{
				unsigned int data;
				serialPort->getRecvFifo()->get(reinterpret_cast<unsigned char*>(&data), LENGTH_AREA_SIZE);

				if (data <= MAX_DATA_SIZE - CRC_AREA_SIZE)
				{
					dataSize = data + CRC_AREA_SIZE;
					loadDataSize = 0;
					phase = CONFIG_PHASE_DATA;
				}
				else
				{
					sendMessage(Message(MESSAGE_FROM_OFFSET_CONFIG, CONFIG_MESSAGE_UPDATE_ERROR, 0, CONFIG_PHASE_LENGTH));
					stopUpdate();
				}
			}
			break;
		case CONFIG_PHASE_DATA:
			progress = (loadDataSize * 100) / dataSize;
			dLen = serialPort->getRecvFifo()->getDataSize();
			if (loadDataSize + dLen > dataSize)
				dLen = dataSize - loadDataSize;

			dLen = serialPort->getRecvFifo()->get(&buffer[loadDataSize], dLen);

			loadDataSize += dLen;

			if (loadDataSize == dataSize)
			{
				if (calcCRC32(buffer, dataSize - 4) == *reinterpret_cast<unsigned int*>(&buffer[dataSize - CRC_AREA_SIZE]))
				{
					phase = CONFIG_PHASE_DISCONNECT;
					pConfigData->setNewData(buffer, dataSize - CRC_AREA_SIZE);

					unsigned int data[2];
					data[0] = DISCONNECT_CODE;
					data[1] = DOWNLOAD_RESULT_OK;
					serialPort->setNewSendData(reinterpret_cast<unsigned char*>(&data[0]), 8);
					serialPort->startSend();
				}
				else
				{
					phase = CONFIG_PHASE_DISCONNECT_AFTER_ERROR;

					unsigned int data[2];
					data[0] = DISCONNECT_CODE;
					data[1] = DOWNLOAD_RESULT_FAULT;
					serialPort->setNewSendData(reinterpret_cast<unsigned char*>(&data[0]), 8);
					serialPort->startSend();
				}
			}
			break;
		case CONFIG_PHASE_DISCONNECT:
			if ((!serialPort->isSendActive()) && (pConfigData->isDataSave()))
			{
				stopUpdate();
				sendMessage(Message(MESSAGE_FROM_OFFSET_CONFIG, CONFIG_MESSAGE_UPDATE_FINISH, 0, 0));
			}
			break;
		case CONFIG_PHASE_DISCONNECT_AFTER_ERROR:
			if (!serialPort->isSendActive())
			{
				stopUpdate();
				sendMessage(Message(MESSAGE_FROM_OFFSET_CONFIG, CONFIG_MESSAGE_UPDATE_ERROR, 0, CONFIG_PHASE_DATA));
			}
			break;
        default:
            break;
	}
}

//=============================================================================================

void Config::startUpdateFR()
{
	numberPrWithTrajectories = getNumberPrWithTrajectories(&prWithTrajectories);
	if (numberPrWithTrajectories > 0)
	{
		SAFE_DELETE_ARRAY(pTrajectory)
		pTrajectory = new Trajectory[TRAJECTORY_COUNT];

		prIndex = 0;
		phaseFR = CONFIG_PHASE_FR_CONNECTION;
		progressFR = 0;
		serialPort->getRecvFifo()->clear();
	}
	else
		frActionError(CONFIG_PHASE_FR_STOP,0);
}

void Config::stopUpdateFR()
{
	phaseFR = CONFIG_PHASE_FR_STOP;
	progressFR = -1;
}

void Config::frActionError(CONFIG_PHASE_FR phase, unsigned char prNumber)
{
	stopUpdateFR();
	sendMessage(Message(MESSAGE_FROM_OFFSET_CONFIG, CONFIG_MESSAGE_UPDATE_ERROR_FR, phaseFR, prNumber));
}

void Config::frActionFinish()
{
	stopUpdateFR();
	sendMessage(Message(MESSAGE_FROM_OFFSET_CONFIG, CONFIG_MESSAGE_UPDATE_FINISH_FR, 0, 0));
}

void Config::frAction()
{
	switch (phaseFR)
	{
		case CONFIG_PHASE_FR_CONNECTION:
			configPhaseFrConnection();
			break;
		case CONFIG_PHASE_FR_CONNECTION_WAIT:
			configPhaseFrConnectionWait();
			break;
		case CONFIG_PHASE_FR_TRAJECTORY_START:
			configPhaseFrTrajectoryStart();
			break;
		case CONFIG_PHASE_FR_TRAJECTORY_START_WAIT:
			configPhaseFrTrajectoryStartWait();
			break;
		case CONFIG_PHASE_FR_TRAJECTORY_SET_POINT:
			configPhaseFrTrajectorySetPoint();
			break;
		case CONFIG_PHASE_FR_TRAJECTORY_SET_POINT_WAIT:
			configPhaseFrTrajectorySetPointWait();
			break;
		case CONFIG_PHASE_FR_TRAJECTORY_STOP:
			configPhaseFrTrajectoryStop();
			break;
		case CONFIG_PHASE_FR_TRAJECTORY_STOP_WAIT:
			configPhaseFrTrajectoryStopWait();
			break;
		case CONFIG_PHASE_FR_PRESSURE_START:
			configPhaseFrPressureStart();
			break;
		case CONFIG_PHASE_FR_PRESSURE_START_WAIT:
			configPhaseFrPressureStartWait();
			break;
		case CONFIG_PHASE_FR_PRESSURE_SET1:
			configPhaseFrPressureSet1();
			break;
		case CONFIG_PHASE_FR_PRESSURE_SET1_WAIT:
			configPhaseFrPressureSet1Wait();
			break;
		case CONFIG_PHASE_FR_PRESSURE_SET2:
			configPhaseFrPressureSet2();
			break;
		case CONFIG_PHASE_FR_PRESSURE_SET2_WAIT:
			configPhaseFrPressureSet2Wait();
			break;
		case CONFIG_PHASE_FR_PRESSURE_STOP:
			configPhaseFrPressureStop();
			break;
		case CONFIG_PHASE_FR_PRESSURE_STOP_WAIT:
			configPhaseFrPressureStopWait();
			break;
        default:
            break;
	}
}

void Config::configPhaseFrConnection()
{
	if (prIndex == numberPrWithTrajectories)
	{
		progressFR = 100;
		frActionFinish();
	}
	else
	{
		for (unsigned int i = 0; i < TRAJECTORY_COUNT; ++i)
			pTrajectory[i].clear();

		unsigned char frame[20];
		unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

		frame[0] = address;
		frame[1] = 0;
		frame[2] = COMMAND_SET_TRAJECTORY;
		frame[3] = 1;
		frame[4] = SUBCOMMAND_GENERAL_START;

		DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4])

		frameId = RpkSubsystem::getSingleton().write(frame);
		if (frameId == IRpkDevice::BAD_FRAME_ID)
		{
			DEBUG_IF_PUT_METHOD("IRpkDevice::BAD_FRAME_ID\n")
			frActionError(CONFIG_PHASE_FR_CONNECTION, address);
		}
		else
		{
			phaseFR = CONFIG_PHASE_FR_CONNECTION_WAIT;
			numberTrajectories = getTrajectoriesForPr(prWithTrajectories[prIndex]);
			pressureTableCount = getPressureTableCountForPr(prWithTrajectories[prIndex]);
			trajectoryIndex = 0;
			trajectoryCount = 0;
			totalPoints = 0;
			accumPointsCount = 0;
			for (unsigned int i = 0; i < TRAJECTORY_COUNT; ++i)
				if (pTrajectory[i].valid)
					totalPoints += pTrajectory[i].pointsCount;
		}
	}
}

void Config::configPhaseFrConnectionWait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_CONNECTION_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_CONNECTION_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_CONNECTION_WAIT, prWithTrajectories[prIndex]);
			}
			else
				phaseFR = CONFIG_PHASE_FR_TRAJECTORY_START;
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

void Config::configPhaseFrTrajectoryStart()
{
	trajectoryIndex = getNextTrajectoryIndex();

	if (trajectoryCount != numberTrajectories)
	{
		unsigned char frame[20];
		unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

		frame[0] = address;
		frame[1] = 0;
		frame[2] = COMMAND_SET_TRAJECTORY;
		frame[3] = 6;
		frame[4] = SUBCOMMAND_TRAJECTORY_START;

		frame[5] = trajectoryIndex;
		frame[6] = pTrajectory[trajectoryIndex].pointsCount >> 8;
		frame[7] = pTrajectory[trajectoryIndex].pointsCount;

		unsigned short crc = pTrajectory[trajectoryIndex].getCrc();

		frame[8] = crc >> 8;
		frame[9] = static_cast<unsigned char>(crc);

		DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9])

		frameId = RpkSubsystem::getSingleton().write(frame);
		if (frameId == IRpkDevice::BAD_FRAME_ID)
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_START, address);
		else
		{
			phaseFR = CONFIG_PHASE_FR_TRAJECTORY_START_WAIT;
			pointCount = 0;
		}
	}
	else
		phaseFR = CONFIG_PHASE_FR_PRESSURE_START;
}

void Config::configPhaseFrTrajectoryStartWait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_START_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_START_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_TRAJECTORY_START_WAIT, prWithTrajectories[prIndex]);
			}
			else
				phaseFR = CONFIG_PHASE_FR_TRAJECTORY_SET_POINT;
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

void Config::configPhaseFrTrajectorySetPoint()
{
	if (pointCount != pTrajectory[trajectoryIndex].pointsCount)
	{
		progressFR = static_cast<int>((100.0f / static_cast<float>(numberPrWithTrajectories)) *
			(static_cast<float>(prIndex) + (static_cast<float>(accumPointsCount)) / (static_cast<float>(totalPoints + pressureTableCount))));

		unsigned char frame[20];
		unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

		frame[0] = address;
		frame[1] = 0;
		frame[2] = COMMAND_SET_TRAJECTORY;
//      === ��� �������� ����� ������ ���������
//		frame[3] = 12;
//
		frame[3] = 11;
		frame[4] = SUBCOMMAND_SET_POINT;

		frame[5] = trajectoryIndex;
		frame[6] = pTrajectory[trajectoryIndex].points[pointCount].number >> 8;
		frame[7] = static_cast<unsigned char>(pTrajectory[trajectoryIndex].points[pointCount].number);
		frame[8] = pTrajectory[trajectoryIndex].points[pointCount].x >> 8;
		frame[9] = static_cast<unsigned char>(pTrajectory[trajectoryIndex].points[pointCount].x);
		frame[10] = pTrajectory[trajectoryIndex].points[pointCount].y >> 8;
		frame[11] = static_cast<unsigned char>(pTrajectory[trajectoryIndex].points[pointCount].y);
		frame[12] = pTrajectory[trajectoryIndex].points[pointCount].nasadok >> 8;
		frame[13] = static_cast<unsigned char>(pTrajectory[trajectoryIndex].points[pointCount].nasadok);
//      === ��� �������� ����� ������ ���������
//		frame[14] = pTrajectory[trajectoryIndex].points[pointCount].pressureTableNumber >> 8;
//		frame[15] = pTrajectory[trajectoryIndex].points[pointCount].pressureTableNumber;
//      ===
		frame[14] = static_cast<unsigned char>(pTrajectory[trajectoryIndex].points[pointCount].pressureTableNumber);

		DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9], frame[10], frame[11], frame[12], frame[13], frame[14])

		frameId = RpkSubsystem::getSingleton().write(frame);
		if (frameId == IRpkDevice::BAD_FRAME_ID)
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_SET_POINT, address);
		else
		{
			phaseFR = CONFIG_PHASE_FR_TRAJECTORY_SET_POINT_WAIT;
		}
	}
	else
	{
		phaseFR = CONFIG_PHASE_FR_TRAJECTORY_STOP;
	}
}

void Config::configPhaseFrTrajectorySetPointWait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_SET_POINT_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_SET_POINT_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_TRAJECTORY_SET_POINT_WAIT, prWithTrajectories[prIndex]);
			}
			else
			{
				phaseFR = CONFIG_PHASE_FR_TRAJECTORY_SET_POINT;
				++pointCount;
				++accumPointsCount;
			}
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

void Config::configPhaseFrTrajectoryStop()
{
	unsigned char frame[20];
	unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

	frame[0] = address;
	frame[1] = 0;
	frame[2] = COMMAND_SET_TRAJECTORY;
	frame[3] = 2;
	frame[4] = SUBCOMMAND_TRAJECTORY_STOP;

	frame[5] = trajectoryIndex;

	DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5])

	frameId = RpkSubsystem::getSingleton().write(frame);
	if (frameId == IRpkDevice::BAD_FRAME_ID)
		frActionError(CONFIG_PHASE_FR_TRAJECTORY_STOP, address);
	else
	{
		phaseFR = CONFIG_PHASE_FR_TRAJECTORY_STOP_WAIT;
	}
}

void Config::configPhaseFrTrajectoryStopWait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_STOP_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_TRAJECTORY_STOP_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_TRAJECTORY_STOP_WAIT, prWithTrajectories[prIndex]);
			}
			else
			{
				phaseFR = CONFIG_PHASE_FR_TRAJECTORY_START;
				++trajectoryCount;
			}
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

void Config::configPhaseFrPressureStart()
{
	unsigned char frame[20];
	unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

	pressureTableCountc = 0;
	pressureTableIndex = 0;

	frame[0] = address;
	frame[1] = 0;
	frame[2] = COMMAND_SET_TRAJECTORY;
//      === ��� �������� ����� ������ ���������
//	frame[3] = 5;
//
	frame[3] = 4;
	frame[4] = SUBCOMMAND_PRESSURE_TABLE_START;
//      === ��� �������� ����� ������ ���������
//	frame[5] = pressureTableCount >> 8;
//	frame[6] = pressureTableCount;
//
	frame[5] = pressureTableCount;

	unsigned short crc = getPressureTableCrc(prWithTrajectories[prIndex]);

//      === ��� �������� ����� ������ ���������
//	frame[7] = crc >> 8;
//	frame[8] = crc;
//
	frame[6] = crc >> 8;
	frame[7] = static_cast<unsigned char>(crc);
	DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7])

	frameId = RpkSubsystem::getSingleton().write(frame);
	if (frameId == IRpkDevice::BAD_FRAME_ID)
		frActionError(CONFIG_PHASE_FR_TRAJECTORY_START, address);
	else
	{
		phaseFR = CONFIG_PHASE_FR_PRESSURE_START_WAIT;
	}
}

void Config::configPhaseFrPressureStartWait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_START_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_START_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_PRESSURE_START_WAIT, prWithTrajectories[prIndex]);
			}
			else
			{
				phaseFR = CONFIG_PHASE_FR_PRESSURE_SET1;
				pressureTableIndex = getPressureTableIndexForPr(0, prWithTrajectories[prIndex]);
			}
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

void Config::configPhaseFrPressureSet1()
{
	if (pressureTableCount == pressureTableCountc)
	{
		phaseFR = CONFIG_PHASE_FR_PRESSURE_STOP;
		return;
	}

	progressFR = static_cast<int>((100.0f / static_cast<float>(numberPrWithTrajectories)) *
		(static_cast<float>(prIndex) + (static_cast<float>(accumPointsCount + pressureTableCountc)) / (static_cast<float>(totalPoints + pressureTableCount))));

	unsigned char frame[20];
	unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();

	frame[0] = address;
	frame[1] = 0;
	frame[2] = COMMAND_SET_TRAJECTORY;
//      === ��� �������� ����� ������ ���������
//	frame[3] = 14;
//
	frame[3] = 13;
	frame[4] = SUBCOMMAND_SET_PRESSURE_TABLE;

//      === ��� �������� ����� ������ ���������
//	frame[5] = pressureConfig[pressureTableIndex]->arNumber >> 8;
//	frame[6] = pressureConfig[pressureTableIndex]->arNumber;
	frame[5] = static_cast<unsigned char>(pressureConfig[pressureTableIndex]->arNumber);

//      === ��� �������� ����� ������ ���������
	//frame[7] = 0;
	//frame[8] = pressureConfig[pressureTableIndex]->pressure;
	//frame[9] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);
	//frame[10] = pressureConfig[pressureTableIndex]->pressure;
	//frame[11] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);
	//frame[12] = pressureConfig[pressureTableIndex]->pressure;
	//frame[13] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);
	//frame[14] = pressureConfig[pressureTableIndex]->pressure;
	//frame[15] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);
	//frame[16] = pressureConfig[pressureTableIndex]->pressure;
	//frame[17] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);


	frame[6] = 0;
	frame[7] = pressureConfig[pressureTableIndex]->pressure;
	frame[8] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);
	frame[9] = pressureConfig[pressureTableIndex]->pressure;
	frame[10] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);
	frame[11] = pressureConfig[pressureTableIndex]->pressure;
	frame[12] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);
	frame[13] = pressureConfig[pressureTableIndex]->pressure;
	frame[14] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);
	frame[15] = pressureConfig[pressureTableIndex]->pressure;
	frame[16] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);

	DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9], frame[10], frame[11], frame[12], frame[13], frame[14], frame[15], frame[16])

	frameId = RpkSubsystem::getSingleton().write(frame);
	if (frameId == IRpkDevice::BAD_FRAME_ID)
		frActionError(CONFIG_PHASE_FR_PRESSURE_SET1, address);
	else
	{
		phaseFR = CONFIG_PHASE_FR_PRESSURE_SET1_WAIT;
	}
}

void Config::configPhaseFrPressureSet1Wait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_SET1_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_SET1_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_PRESSURE_SET1_WAIT, prWithTrajectories[prIndex]);
			}
			else
			{
				phaseFR = CONFIG_PHASE_FR_PRESSURE_SET2;
			}
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

void Config::configPhaseFrPressureSet2()
{
	unsigned char frame[20];
	unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();

	frame[0] = address;
	frame[1] = 0;
	frame[2] = COMMAND_SET_TRAJECTORY;
//      === ��� �������� ����� ������ ���������
//	frame[3] = 14;
//
	frame[3] = 13;
	frame[4] = SUBCOMMAND_SET_PRESSURE_TABLE;

//      === ��� �������� ����� ������ ���������
//	frame[5] = pressureConfig[pressureTableIndex]->arNumber >> 8;
//	frame[6] = pressureConfig[pressureTableIndex]->arNumber;
//
	frame[5] = static_cast<unsigned char>(pressureConfig[pressureTableIndex]->arNumber);

//      === ��� �������� ����� ������ ���������
	//frame[7] = 5;
	//frame[8] = pressureConfig[pressureTableIndex]->pressure;
	//frame[9] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);
	//frame[10] = pressureConfig[pressureTableIndex]->pressure;
	//frame[11] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);
	//frame[12] = pressureConfig[pressureTableIndex]->pressure;
	//frame[13] = pressureConfig[pressureTableIndex++]->delta;
	//pressureTableIndex = getPressureTableIndex(pressureTableIndex, prWithTrajectories[prIndex]);
	//frame[14] = pressureConfig[pressureTableIndex]->pressure;
	//frame[15] = pressureConfig[pressureTableIndex++]->delta;

	//frame[16] = 0;//pressureConfig[pressureTableIndex]->pressure;
	//frame[17] = 0;//pressureConfig[pressureTableIndex++]->delta;

	frame[6] = 5;
	frame[7] = pressureConfig[pressureTableIndex]->pressure;
	frame[8] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);
	frame[9] = pressureConfig[pressureTableIndex]->pressure;
	frame[10] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);
	frame[11] = pressureConfig[pressureTableIndex]->pressure;
	frame[12] = pressureConfig[pressureTableIndex++]->delta;
	pressureTableIndex = getPressureTableIndexForPr(pressureTableIndex, prWithTrajectories[prIndex]);
	frame[13] = pressureConfig[pressureTableIndex]->pressure;
	frame[14] = pressureConfig[pressureTableIndex++]->delta;

	frame[15] = 0;//pressureConfig[pressureTableIndex]->pressure;
	frame[16] = 0;//pressureConfig[pressureTableIndex++]->delta;

	DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9], frame[10], frame[11], frame[12], frame[13], frame[14], frame[15], frame[16])

	frameId = RpkSubsystem::getSingleton().write(frame);
	if (frameId == IRpkDevice::BAD_FRAME_ID)
		frActionError(CONFIG_PHASE_FR_PRESSURE_SET2, address);
	else
	{
		phaseFR = CONFIG_PHASE_FR_PRESSURE_SET2_WAIT;
	}

	pressureTableCountc++;
}

void Config::configPhaseFrPressureSet2Wait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_SET2_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_SET2_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_PRESSURE_SET2_WAIT, prWithTrajectories[prIndex]);
			}
			else
			{
				phaseFR = CONFIG_PHASE_FR_PRESSURE_SET1;
			}
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

void Config::configPhaseFrPressureStop()
{
	unsigned char frame[20];
	unsigned char address = getConfigData()->getPRAddressByNumber(prWithTrajectories[prIndex]);

	frame[0] = address;
	frame[1] = 0;
	frame[2] = COMMAND_SET_TRAJECTORY;
	frame[3] = 1;
	frame[4] = SUBCOMMAND_PRESSURE_TABLE_STOP;

	DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u\n", frame[0], frame[1], frame[2], frame[3], frame[4])

	frameId = RpkSubsystem::getSingleton().write(frame);
	if (frameId == IRpkDevice::BAD_FRAME_ID)
		frActionError(CONFIG_PHASE_FR_PRESSURE_STOP, address);
	else
	{
		phaseFR = CONFIG_PHASE_FR_PRESSURE_STOP_WAIT;
	}
}

void Config::configPhaseFrPressureStopWait()
{
	unsigned char* pFrame = nullptr;
	IRpkDevice::FRAME_RESULT result;

	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
	switch (result)
	{
		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ID_NOT_FOUND\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_STOP_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_ERROR:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_ERROR\n")
			frActionError(CONFIG_PHASE_FR_PRESSURE_STOP_WAIT, prWithTrajectories[prIndex]);
			break;
		case IRpkDevice::FRAME_RESULT_READY:
			DEBUG_IF_PUT_METHOD("FRAME_RESULT_READY\n")
			if (trajectoryResultErrorAction(pFrame[6]))
			{
				frActionError(CONFIG_PHASE_FR_PRESSURE_STOP_WAIT, prWithTrajectories[prIndex]);
			}
			else
			{
				++prIndex;
				phaseFR = CONFIG_PHASE_FR_CONNECTION;
			}
			break;
        default:
            break;
	}
	SAFE_DELETE_ARRAY(pFrame)
}

bool Config::trajectoryResultErrorAction(unsigned char result)
{
	if (result == 0)
		return false;

	DEBUG_IF_PUT_METHOD("Error = %u\n", result)
	return true;
}

unsigned int Config::getNextTrajectoryIndex()
{
	++trajectoryIndex;
	for (; trajectoryIndex < TRAJECTORY_COUNT; ++trajectoryIndex)
		if (pTrajectory[trajectoryIndex].valid)
			break;

	return trajectoryIndex;
}

unsigned int Config::getNumberPrWithTrajectories(unsigned char** ppPrWithTrajectories)
{
	SAFE_DELETE_ARRAY(*ppPrWithTrajectories);
	unsigned int count = getConfigData()->getConfigDataStructTrajectoryCount();
	ConfigDataStructTrajectory** trajectoryConfig = getConfigData()->getConfigDataStructTrajectory();

	const unsigned int NUMBERS_COUNT = ConfigDataStruct::MAX_NUMBER_PR_POSITION + 1;
	bool numbers[NUMBERS_COUNT];
	memset(numbers, false, NUMBERS_COUNT);

	unsigned int resultCount = 0;
	for (unsigned int i = 0; i < count; i++)
		if (!numbers[trajectoryConfig[i]->prNumber])
		{
			DEBUG_IF_PUT_METHOD("�� � ����������� -  %u\n", trajectoryConfig[i]->prNumber);
			numbers[trajectoryConfig[i]->prNumber] = true;
			resultCount++;
		}

	if (resultCount > 0)
	{
		*ppPrWithTrajectories = new unsigned char[resultCount];
		DEBUG_IF_PUT_METHOD("����� �� � ������������ = %u\n", resultCount);
		unsigned int _prIndex = 0;
		for (unsigned int i = 0; i < NUMBERS_COUNT; i++)
			if (numbers[i])
			{
				(*ppPrWithTrajectories)[_prIndex++] = i;
				DEBUG_IF_PUT_METHOD("�� � ����������� - %u\n", i)
			}
	}

	return resultCount;
}

unsigned int Config::getTrajectoriesForPr(unsigned char prNumber)
{
	pressureTableCount = 0;
	unsigned int count = getConfigData()->getConfigDataStructTrajectoryCount();
	ConfigDataStructTrajectory** trajectoryConfig = getConfigData()->getConfigDataStructTrajectory();

	for (unsigned int i = 0; i < count; i++)
		if (trajectoryConfig[i]->prNumber == prNumber)
		{
			pTrajectory[trajectoryConfig[i]->trajectoryNumber].valid = true;
			pTrajectory[trajectoryConfig[i]->trajectoryNumber].addPoint(trajectoryConfig[i]->pointNumber, trajectoryConfig[i]->position.x, trajectoryConfig[i]->position.y, trajectoryConfig[i]->nasadok, trajectoryConfig[i]->pressureNumber);
			//DEBUG_IF_PUT_METHOD("����� ���������� = %u, ����� ����� = %u, x = %u, y = %u, ������� = %u, ����� ������� ��������� = %u\n", trajectoryConfig[i]->trajectoryNumber, trajectoryConfig[i]->pointNumber, trajectoryConfig[i]->position.x, trajectoryConfig[i]->position.y, trajectoryConfig[i]->nasadok, trajectoryConfig[i]->pressureNumber)
		}

	numberTrajectories = 0;
	for (unsigned int i = 0; i < TRAJECTORY_COUNT; ++i)
		if (pTrajectory[i].valid)
			++numberTrajectories;

	DEBUG_IF_PUT_METHOD("��%u ������ ����������: ", prNumber)
	for (unsigned int i = 0; i < Trajectory::MAX_POINTS_COUNT; ++i)
		if (pTrajectory[i].valid)
			DEBUG_IF_PUT("%u ", i)

	DEBUG_IF_PUT("\n")

	return numberTrajectories;
}

unsigned int Config::getPressureTableIndexForPr(unsigned int startIndex, unsigned int prNumber)
{
	unsigned int count = getConfigData()->getConfigDataStructPressureCount();
	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();

	for (unsigned int i = startIndex; i < count; i++)
		if (pressureConfig[i]->prNumber == prNumber)
			return i;

	return 0;
}

unsigned int Config::getPressureTableIndex(unsigned int startIndex, unsigned int arNumber)
{
	unsigned int count = getConfigData()->getConfigDataStructPressureCount();
	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();

	for (unsigned int i = startIndex; i < count; i++)
		if (pressureConfig[i]->arNumber == arNumber)
			return i;

	return 0;
}

unsigned int Config::getPressureTableCountForPr(unsigned int prNumber)
{
	unsigned int count = 0;
	unsigned int pressureCount = getConfigData()->getConfigDataStructPressureCount();
	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();

	for (unsigned int i = 0; i < pressureCount; i++)
		if (pressureConfig[i]->prNumber == prNumber)
			count++;

	count /= 9;

	DEBUG_IF_PUT_METHOD("����� ������ ��������� ��� ��%u = %u\n", prNumber, count)

	return count;
}

unsigned int Config::getPressureTableCrc(unsigned int prNumber)
{
	unsigned short crc = 0;
	unsigned int count = getConfigData()->getConfigDataStructPressureCount();
	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();

	for (unsigned int i = 0; i < count; i++)
		if (pressureConfig[i]->prNumber == prNumber)
			crc += (pressureConfig[i]->pressure + pressureConfig[i]->delta);

	return crc;
}

//---------------------------------------------------------------







//
//
//
//void Config::configPhaseFrTrajectoryWait()
//{
//	unsigned char* pFrame = nullptr;
//	IRpkDevice::FRAME_RESULT result;
//
//	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
//	SAFE_DELETE_ARRAY(pFrame)
//	switch (result)
//	{
//		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
//			frActionError(CONFIG_PHASE_FR_TRAJECTORY, prNumbers[prNumbersIndex]);
//			break;
//		case IRpkDevice::FRAME_RESULT_ERROR:
//			frActionError(CONFIG_PHASE_FR_TRAJECTORY, prNumbers[prNumbersIndex]);
//			break;
//		case IRpkDevice::FRAME_RESULT_READY:
//			trajectoryPointIndex++;
//			phaseFR = CONFIG_PHASE_FR_TRAJECTORY;
//			break;
//	}
//}
//
//void Config::configPhaseFrPressure()
//{
//	if (pressureTableNumbersIndex == pressureTableNumbersCount)
//	{
//		phaseFR = CONFIG_PHASE_FR_DISCONNECTION;
//	}
//	else
//	{
//		frameIdRes = 0;
//		frameId1Res = 0;
//		unsigned char frame[20];
//
//		unsigned char address = getConfigData()->getPRAddressByNumber(prNumbers[prNumbersIndex]);
//
//		unsigned int* pIndex;
//
//		if (!getPressureTableItemsIndex(pressureTableNumbers[pressureTableNumbersIndex], &pIndex))
//			frActionError(CONFIG_PHASE_FR_PRESSURE, address);
//		else
//		{
//			ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();
//
//			frame[0] = address;
//			frame[1] = 0;
//			frame[2] = COMMAND_SET_TRAJECTORY;
//			frame[3] = 13;
//			//frame[4] = SUBCOMMAND_PRESSURE;
//			frame[5] = pressureConfig[pIndex[0]]->arNumber;
//			frame[6] = 0;
//			frame[7] = pressureConfig[pIndex[0]]->pressure;
//			frame[8] = pressureConfig[pIndex[0]]->delta;
//			frame[9] = pressureConfig[pIndex[1]]->pressure;
//			frame[10] = pressureConfig[pIndex[1]]->delta;
//			frame[11] = pressureConfig[pIndex[2]]->pressure;
//			frame[12] = pressureConfig[pIndex[2]]->delta;
//			frame[13] = pressureConfig[pIndex[3]]->pressure;
//			frame[14] = pressureConfig[pIndex[3]]->delta;
//			frame[15] = pressureConfig[pIndex[4]]->pressure;
//			frame[16] = pressureConfig[pIndex[4]]->delta;
//
////SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("Config::configPhaseFrPressure...1: ");
////SERIAL_DEBUG_ADD_DEBUG_MESSAGE_CHAR_ARRAY(frame, 17);
////SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("        __         ", trajectoryPointIndex);
//
//			frameId = RpkSubsystem::getSingleton().write(frame);
//			if (frameId == IRpkDevice::BAD_FRAME_ID)
//			{
//				delete[] pIndex;
//				frActionError(CONFIG_PHASE_FR_PRESSURE, address);
//			}
//			else
//			{
//				frame[0] = address;
//				frame[1] = 0;
//				frame[2] = COMMAND_SET_TRAJECTORY;
//				frame[3] = 11;
//				//frame[4] = SUBCOMMAND_PRESSURE;
//				frame[5] = pressureConfig[pIndex[0]]->arNumber;
//				frame[6] = 5;
//				frame[7] = pressureConfig[pIndex[5]]->pressure;
//				frame[8] = pressureConfig[pIndex[5]]->delta;
//				frame[9] = pressureConfig[pIndex[6]]->pressure;
//				frame[10] = pressureConfig[pIndex[6]]->delta;
//				frame[11] = pressureConfig[pIndex[7]]->pressure;
//				frame[12] = pressureConfig[pIndex[7]]->delta;
//				frame[13] = pressureConfig[pIndex[8]]->pressure;
//				frame[14] = pressureConfig[pIndex[8]]->delta;
//
////SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("Config::configPhaseFrPressure...2: ");
////SERIAL_DEBUG_ADD_DEBUG_MESSAGE_CHAR_ARRAY(frame, 15);
////SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("        __         ", trajectoryPointIndex);
//
//				frameId1 = RpkSubsystem::getSingleton().write(frame);
//				if (frameId1 == IRpkDevice::BAD_FRAME_ID)
//				{
//					delete[] pIndex;
//					frActionError(CONFIG_PHASE_FR_PRESSURE, address);
//				}
//				else
//				{
//					delete[] pIndex;
//					phaseFR = CONFIG_PHASE_FR_PRESSURE_WAIT;
//				}
//			}
//		}
//	}
//}
//
//void Config::configPhaseFrPressureWait()
//{
//	unsigned char* pFrame = nullptr;
//	IRpkDevice::FRAME_RESULT result;
//
//	if (frameIdRes == 0)
//	{
//		result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
//		SAFE_DELETE_ARRAY(pFrame)
//		switch (result)
//		{
//			case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
//				frameIdRes = 2;
//				break;
//			case IRpkDevice::FRAME_RESULT_ERROR:
//				frameIdRes = 2;
//				break;
//			case IRpkDevice::FRAME_RESULT_READY:
//				frameIdRes = 1;
//				break;
//		}
//	}
//
//	if (frameId1Res == 0)
//	{
//		result = RpkSubsystem::getSingleton().read(frameId1, &pFrame);
//		SAFE_DELETE_ARRAY(pFrame)
//		switch (result)
//		{
//			case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
//				frameId1Res = 2;
//				break;
//			case IRpkDevice::FRAME_RESULT_ERROR:
//				frameId1Res = 2;
//				break;
//			case IRpkDevice::FRAME_RESULT_READY:
//				frameId1Res = 1;
//				break;
//		}
//	}
//
//	if ((frameIdRes == 2) && (frameId1Res == 2))
//	{
//		frActionError(CONFIG_PHASE_FR_PRESSURE, prNumbers[prNumbersIndex]);
//	}
//	else
//		if ((frameIdRes == 1) && (frameId1Res == 1))
//		{
//			pressureTableNumbersIndex++;
//			phaseFR = CONFIG_PHASE_FR_PRESSURE;
//		}
//}
//
//void Config::configPhaseFrDisconnection()
//{
//	unsigned char frame[20];
//
//	unsigned char address = getConfigData()->getPRAddressByNumber(prNumbers[prNumbersIndex]);
//
//	frame[0] = address;
//	frame[1] = 0;
//	frame[2] = COMMAND_SET_TRAJECTORY;
//	frame[3] = 1;
//	//frame[4] = SUBCOMMAND_FINISH;
//
//	frameId = RpkSubsystem::getSingleton().write(frame);
//	if (frameId == IRpkDevice::BAD_FRAME_ID)
//		frActionError(CONFIG_PHASE_FR_DISCONNECTION, address);
//	else
//		phaseFR = CONFIG_PHASE_FR_DISCONNECTION_WAIT;
//}
//
//void Config::configPhaseFrDisconnectionWait()
//{
//	unsigned char* pFrame = nullptr;
//	IRpkDevice::FRAME_RESULT result;
//
//	result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
//	SAFE_DELETE_ARRAY(pFrame)
//	switch (result)
//	{
//		case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
//			frActionError(CONFIG_PHASE_FR_DISCONNECTION_WAIT, prNumbers[prNumbersIndex]);
//			break;
//		case IRpkDevice::FRAME_RESULT_ERROR:
//			frActionError(CONFIG_PHASE_FR_DISCONNECTION_WAIT, prNumbers[prNumbersIndex]);
//			break;
//		case IRpkDevice::FRAME_RESULT_READY:
//			prNumbersIndex++;
//			progressFR += progressFRStep;
//			phaseFR = CONFIG_PHASE_FR_CONNECTION;
//			break;
//	}
//}
//
//unsigned int Config::createPrNumbers()
//{
//	SAFE_DELETE_ARRAY(prNumbers);
//	unsigned int count = getConfigData()->getConfigDataStructTrajectoryCount();
//	ConfigDataStructTrajectory** trajectoryConfig = getConfigData()->getConfigDataStructTrajectory();
//
//	const unsigned int NUMBERS_COUNT = ConfigDataStruct::MAX_NUMBER_PR_POSITION + 1;
//	bool numbers[NUMBERS_COUNT];
//	memset(numbers, false, NUMBERS_COUNT);
//
//	unsigned int resultCount = 0;
//	for (unsigned int i = 0; i < count; i++)
//		if (!numbers[trajectoryConfig[i]->prNumber])
//		{
//			numbers[trajectoryConfig[i]->prNumber] = true;
//			resultCount++;
//		}
//
//	if (resultCount > 0)
//	{
//		prNumbers = new unsigned char[resultCount];
//		unsigned int _prIndex = 0;
//		for (unsigned int i = 0; i < NUMBERS_COUNT; i++)
//			if (numbers[i])
//				prNumbers[_prIndex++] = i;
//	}
//
//	return resultCount;
//}
//
//unsigned int Config::createTrajectoryNumbers(unsigned char prNumber)
//{
//	SAFE_DELETE_ARRAY(trajectoryNumbers);
//	unsigned int count = getConfigData()->getConfigDataStructTrajectoryCount();
//	ConfigDataStructTrajectory** trajectoryConfig = getConfigData()->getConfigDataStructTrajectory();
//
//	const unsigned int NUMBERS_COUNT = ConfigDataStruct::MAX_NUMBER_Trajectory;
//	bool* numbers = new bool[NUMBERS_COUNT];
//	memset(numbers, false, NUMBERS_COUNT);
//
//	unsigned int resultCount = 0;
//	for (unsigned int i = 0; i < count; i++)
//		if (trajectoryConfig[i]->prNumber == prNumber)
//			if (!numbers[trajectoryConfig[i]->trajectoryNumber])
//			{
//				numbers[trajectoryConfig[i]->trajectoryNumber] = true;
//				resultCount++;
//			}
//
//	if (resultCount > 0)
//	{
//		trajectoryNumbers = new unsigned char[resultCount];
//		unsigned int _trajectoryIndex = 0;
//		for (unsigned int i = 0; i < NUMBERS_COUNT; i++)
//			if (numbers[i])
//				trajectoryNumbers[_trajectoryIndex++] = i;
//	}
//
//	delete[] numbers;
//
//	DEBUG_IF_PUT_METHOD("����� ���������� ��� ��%u = %u\n", prNumber, resultCount)
//	return resultCount;
//}
//
//unsigned int Config::createPressureTableNumbers(unsigned char prNumber)
//{
//	SAFE_DELETE_ARRAY(pressureTableNumbers);
//	unsigned int count = getConfigData()->getConfigDataStructPressureCount();
//	ConfigDataStructPressure** pressureTableConfig = getConfigData()->getConfigDataStructPressure();
//
//	const unsigned int NUMBERS_COUNT = ConfigDataStruct::MAX_NUMBER_PRESSURE;
//	bool* numbers = new bool[NUMBERS_COUNT];
//	memset(numbers, false, NUMBERS_COUNT);
//
//	unsigned int resultCount = 0;
//	for (unsigned int i = 0; i < count; i++)
//		if (pressureTableConfig[i]->prNumber == prNumber)
//			if (!numbers[pressureTableConfig[i]->arNumber])
//			{
//				numbers[pressureTableConfig[i]->arNumber] = true;
//				resultCount++;
//			}
//
//	if (resultCount > 0)
//	{
//		pressureTableNumbers = new unsigned char[resultCount];
//		unsigned int _pressureTableIndex = 0;
//		for (unsigned int i = 0; i < NUMBERS_COUNT; i++)
//			if (numbers[i])
//				pressureTableNumbers[_pressureTableIndex++] = i;
//	}
//
//	delete[] numbers;
//	return resultCount;
//}
//
//unsigned int Config::getTrajectoryPointsCount(unsigned char prNumber, unsigned char trajectoryNumber)
//{
//	unsigned int count = getConfigData()->getConfigDataStructTrajectoryCount();
//	ConfigDataStructTrajectory** trajectoryConfig = getConfigData()->getConfigDataStructTrajectory();
//
//	unsigned int resultCount = 0;
//	for (unsigned int i = 0; i < count; i++)
//		if (trajectoryConfig[i]->prNumber == prNumber)
//			if (trajectoryNumber != 255)
//			{
//				if (trajectoryConfig[i]->trajectoryNumber == trajectoryNumber)
//					resultCount++;
//			}
//			else
//				resultCount++;
//
//	return resultCount;
//}
//
//unsigned int Config::getPressureCount(unsigned char prNumber)
//{
//	unsigned int count = getConfigData()->getConfigDataStructPressureCount();
//	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();
//
//	unsigned int resultCount = 0;
//	for (unsigned int i = 0; i < count; i++)
//		if (pressureConfig[i]->prNumber == prNumber)
//			resultCount++;
//
//	return resultCount;
//}
//
//int Config::getNextTrajectoryPointsIndex(unsigned int firstIndex, unsigned char prNumber)
//{
//	unsigned int count = getConfigData()->getConfigDataStructTrajectoryCount();
//	ConfigDataStructTrajectory** trajectoryConfig = getConfigData()->getConfigDataStructTrajectory();
//
//	for (unsigned int i = firstIndex; i < count; i++)
//	{
//		if (trajectoryConfig[i]->prNumber == prNumber)
//		{
//			return i;
//		}
//	}
//
//	return -1;
//}
//
//bool Config::getPressureTableItemsIndex(unsigned char pressureTableNumber, unsigned int** pIndex)
//{
//	*pIndex = nullptr;
//	unsigned int count = getConfigData()->getConfigDataStructPressureCount();
//	ConfigDataStructPressure** pressureConfig = getConfigData()->getConfigDataStructPressure();
//
//	unsigned int ptCount = 0;
//	for (unsigned int i = 0; i < count; i++)
//		if (pressureConfig[i]->arNumber == pressureTableNumber)
//			ptCount++;
//
//	if (ptCount == 0)
//		return false;
//
//	*pIndex = new unsigned int[ptCount];
//
//	unsigned int index = 0;
//	for (unsigned int i = 0; i < count; i++)
//		if (pressureConfig[i]->arNumber == pressureTableNumber)
//			(*pIndex)[index++] = i;
//
//	return true;
//}
//
