#include "debug.h"
#include "display.h"
#include "string.h"

const SERIAL_PORT_SPEED SerialDebug::PORT_SPEED = SERIAL_PORT_SPEED_57600;
const SERIAL_PORT SerialDebug::PORT_NAME = SERIAL_PORT_1;

SerialDebug::SerialDebug()
	:	ITimer(TIMER_PERIOD), serialPort(SerialPortManager::getSingleton().getPort(PORT_NAME)), fifo(new Fifo<char>(FIFO_SIZE)), silentDebugOn(false), debugOn(false), debugInput(true)
{
	pTimer->start();
	serialPort->setSpeed(PORT_SPEED);
//	serialPort->open();
}

SerialDebug::~SerialDebug()
{
	serialPort->close();
}

void SerialDebug::timerHandler()
{
	if (!silentDebugOn)
	{
		if (!serialPort->isSendActive() && !fifo->isEmpty())
		{
			serialPort->setNewSendData(fifo);
			serialPort->startSend();
		}
	}

	if (debugInput)
	{
		Fifo<unsigned char>* rFifo = serialPort->getRecvFifo();
		if (!rFifo->isEmpty())
		{
			unsigned char ch;
			if (rFifo->get(&ch) != 0)
				debugCommand(ch);
		}
	}
}

/* Format

specifier	Output							Example
s			String of characters			sample
u			Unsigned decimal integer		7235
x			Unsigned hexadecimal integer	0x145c07fa
i			Signed decimal integer			-392
f			float							-392.0972

*/

void SerialDebug::put(const char* str, ...)
{
	if (!debugOn)
		return;

	va_list args;
	va_start(args, str);

	while (*str != 0)
	{
		if (*str == '%')
		{
			++str;

			if (*str == 0)
				break;

			if (*str == 's')
			{
				char *substr = va_arg(args, char*);

				if (substr != nullptr)
				{
					while (*substr != 0)
					{
						fifo->put(*substr);
						++substr;
					}
				}
			}
			else
				if (*str == 'u')
				{
					unsigned int value = va_arg(args, unsigned int);
					char _str[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

					char* _pstr = Display::getSingleton().toStringDecimal(value, _str);

					while (*_pstr != 0)
						fifo->put(*_pstr++);
				}
			else
				if (*str == 'x')
				{
					unsigned int value = va_arg(args, unsigned int);
					char _str[9];

					Display::getSingleton().toString(value, _str);

					fifo->put('0');
					fifo->put('x');

					for (unsigned int i = 0; i < 8; ++i)
						fifo->put(_str[i]);
				}
				else
					if (*str == 'i')
					{
						int value = va_arg(args, int);
						char _str[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

						char* _pstr = Display::getSingleton().toStringDecimalSigned(value, _str);

						while (*_pstr != 0)
							fifo->put(*_pstr++);
					}
					else
						if (*str == 'f')
						{
							float value = static_cast<float>(va_arg(args, double));
							char _str[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

							char* _pstr = Display::getSingleton().toStringFloat(value, _str);

							while (*_pstr != 0)
								fifo->put(*_pstr++);
						}
						else
							fifo->put(*str);
		}
		else
			fifo->put(*str);

		++str;
	}

	va_end(args);
}

void SerialDebug::debugCommand(unsigned char command)
{
	switch (command)
	{
		case CTRL_KEY('d'): // ���������/���������� ���������� ���������
			if (debugOn)
			{
				DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_CYAN, SerialDebug::TEXT_ATTR_BOLD, "����� ���������� ��������� ��������.\n");
				debugOn = false;
			}
			else
			{
				debugOn = true;
				DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_CYAN, SerialDebug::TEXT_ATTR_BOLD, "����� ���������� ��������� �������.\n");
			}
			break;
		case 'D': // ���������/���������� ������ ������ ���������� ��������� (��������� �� ������������ �� ��������, �� ������������ � ������)
			if (silentDebugOn)
			{
				silentDebugOn = false;
			}
			else
			{
				silentDebugOn = true;
			}
			break;
		case CTRL_KEY('c'): // ���������/���������� ������ ��������� Config
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_CONFIG, 0));
			break;
		case 'c': // ����� ������ ������������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_GET_SETTINGS, 0));
			break;
		case CTRL_KEY('l'): // ���������/���������� ������ ��������� ������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_LOGIC, 0));
			break;
		case CTRL_KEY('a'): // ��������� ������ ��������� Actions
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_ACTIONS_ON, 0));
			break;
		case 'A': // ���������� ������ ��������� Actions
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_ACTIONS_OFF, 0));
			break;
		case CTRL_KEY('e'): // ���������/���������� ������ ��������� ExtensionSystem
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_EXTENSION_SYSTEM, 0));
			break;
		case CTRL_KEY('i'): // ���������/���������� ������ ��������� IoSubSystem
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_IOSUBSYSTEM, 0));
			break;
		case CTRL_KEY('m'): // ���������/���������� ������ ��������� MonitoringSubSystem
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_MONITORINGSUBSYSTEM, 0));
			break;
		case CTRL_KEY('r'): // ���������/���������� ������ ��������� RpkSubSystem
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_RPKSUBSYSTEM, 0));
			break;
		case CTRL_KEY('s'): // ���������/���������� ������ ��������� DetectionSubSystem
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_DETECTIONSUBSYSTEM, 0));
			break;
		case CTRL_KEY('n'): // ���������/���������� ����� SCAN_PROGRAM_FLAGS � actionStartProgramScanPoint
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_ACTIONS_NASADOK_ON_OFF, 0));
			break;
		case 'm':
			//for (unsigned int i = 0; i < MainMemoryList::BIG_BLOCKS_SIZE; i++)
			//	DEBUG_PUT_METHOD("bigBlocks[%u].address = %x bigBlocks[%u].count = %u\n", i, MemoryAllocator::getSingleton().mainMemoryList.bigBlocks[i].address, i, MemoryAllocator::getSingleton().mainMemoryList.bigBlocks[i].count)

			DEBUG_PUT_METHOD("Allocator::getFreeMemory = %u\n", MemoryAllocator::getSingleton().getFreeMemory())
			break;
		case CTRL_KEY('t'): // ��������� ������ ���������� ��������� ������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_TOUCHPAD_CALIBRATION_ON, 0));
			break;
		case 't': // ������ ������ ���������� ��� ���������� �����������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_TOUCHPAD_CALIBRATION_CANCEL, 0));
			break;
		case 'T': // ���������� ����������� ���������� � ���������� ������ ����������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_TOUCHPAD_CALIBRATION_SAVE, 0));
			break;
		case 'y': // ����� ������ ����������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_TOUCHPAD_CALIBRATION_GET_DATA, 0));
			break;
		case CTRL_KEY('y'): // ��������� ����� ������� ����� ��� ����������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_TOUCHPAD_CALIBRATION_SET_LEFT_UP_POINT, 0));
			break;
		case 'Y': // ��������� ������ ������ ����� ��� ����������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_TOUCHPAD_CALIBRATION_SET_RIGHT_DOWN_POINT, 0));
			break;
		case '1': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 1));
			break;
		case '2': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 2));
			break;
		case '3': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 3));
			break;
		case '4': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 4));
			break;
		case '5': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 5));
			break;
		case '6': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 6));
			break;
		case '7': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 7));
			break;
		case '8': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 8));
			break;
		case '9': // ��������� �����
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_SET_INPUTS, 9));
			break;
		case CTRL_KEY('k'): // ����� ���� ������ � �������������� ���������
			sendMessage(Message(MESSAGE_FROM_OFFSET_SERIAL_DEBUG, SERIAL_DEBUG_MESSAGE_RECV_COMMAND, COMMAND_DEBUG_BK16_RESET_INPUTS, 0));
			break;
		case 'h':
			DEBUG_PUT_COLOR_ATTR(COLOR_BLUE, TEXT_ATTR_BOLD, "�������:\n\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + d  -  ���������/���������� ���������� ���������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          D  -  ���������/���������� ������ ������ ���������� ���������\n                 (��������� �� ������������ �� ��������, �� ������������ � ������)\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + l  -  ���������/���������� ������ ��������� ������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + a  -  ��������� ������ ��������� Actions\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          A  -  ���������� ������ ��������� Actions\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + e  -  ���������/���������� ������ ��������� ExtensionSystem\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + i  -  ���������/���������� ������ ��������� IoSubSystem\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + m  -  ���������/���������� ������ ��������� MonitoringSubSystem\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + r  -  ���������/���������� ������ ��������� RpkSubSystem\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + c  -  ���������/���������� ������ ��������� Config\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + s  -  ���������/���������� ������ ��������� DetectionSubSystem\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + n  -  ���������/���������� ����� SCAN_PROGRAM_FLAGS_NASADOK_ON/OFF � actionStartProgramScanPoint\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          c  -  ����� ������ ������������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          m  -  ����� ���������� � ������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + t  -  ��������� ������ ���������� ��������� ������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          t  -  ������ ������ ���������� ��� ���������� �����������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          T  -  ���������� ����������� ���������� � ���������� ������ ����������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          y  -  ����� ������ ����������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + y  -  ��������� ����� ������� ����� ��� ����������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          Y  -  ��������� ������ ������ ����� ��� ����������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "       1..9  -  ���.���� ����� ��16 ����� 1..9\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "   CTRL + k  -  ����� ���� ������ � �������������� ���������\n")
			DEBUG_PUT_COLOR(COLOR_BLUE, "          h  -  ����� �������\n")
			break;
	}
}

bool SerialDebug::isOn()
{
	return debugOn;
}
