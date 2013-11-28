#include "serialPort.h"
#include "low.h"
#include "interrupt.h"
#include "cpu.h"

#include "lowDisplay.h"

extern "C" void asmSerialPort1InterruptHandler();
extern "C" void asmSerialPort2InterruptHandler();
extern "C" void asmSerialPort3InterruptHandler();

extern "C" void staticSerialPort1IrqHandler(){
	Cpu::cli();
	asm volatile ("pushfl");
	asm volatile ("pushal");

	unsigned char irqCom_i = Cpu::inPort(SerialPortManager::pSerialPorts[0]->baseAddress + SERIAL_PORT_REG_IIR);

	while ((irqCom_i & SERIAL_PORT_IIR_IP) == 0){
		if ((irqCom_i & SERIAL_PORT_IIR_RDAI) != 0)
			SerialPortManager::pSerialPorts[0]->irqHandlerRecv();
		if ((irqCom_i & SERIAL_PORT_IIR_THREI) != 0)
			SerialPortManager::pSerialPorts[0]->irqHandlerSend();
		irqCom_i = Cpu::inPort(SerialPortManager::pSerialPorts[0]->baseAddress + SERIAL_PORT_REG_IIR);
	}

	asm volatile ("popal");
	asm volatile ("popfl");
	Cpu::sti();
	asm volatile ("iretl");

}

extern "C" void staticSerialPort2IrqHandler(){
	Cpu::cli();
	asm volatile ("pushfl");
	asm volatile ("pushal");

	unsigned char irqCom_i = Cpu::inPort(SerialPortManager::pSerialPorts[1]->baseAddress + SERIAL_PORT_REG_IIR);

	while ((irqCom_i & SERIAL_PORT_IIR_IP) == 0){
		if ((irqCom_i & SERIAL_PORT_IIR_RDAI) != 0)
			SerialPortManager::pSerialPorts[1]->irqHandlerRecv();
		if ((irqCom_i & SERIAL_PORT_IIR_THREI) != 0)
			SerialPortManager::pSerialPorts[1]->irqHandlerSend();
		irqCom_i = Cpu::inPort(SerialPortManager::pSerialPorts[1]->baseAddress + SERIAL_PORT_REG_IIR);
	}

	asm volatile ("popal");
	asm volatile ("popfl");
	Cpu::sti();
	asm volatile ("iretl");

}

extern "C" void staticSerialPort3IrqHandler(){
	Cpu::cli();
	asm volatile ("pushfl");
	asm volatile ("pushal");

	unsigned char irqCom_i = Cpu::inPort(SerialPortManager::pSerialPorts[2]->baseAddress + SERIAL_PORT_REG_IIR);

	while ((irqCom_i & SERIAL_PORT_IIR_IP) == 0){
		if ((irqCom_i & SERIAL_PORT_IIR_RDAI) != 0)
			SerialPortManager::pSerialPorts[2]->irqHandlerRecv();
		if ((irqCom_i & SERIAL_PORT_IIR_THREI) != 0)
			SerialPortManager::pSerialPorts[2]->irqHandlerSend();
		irqCom_i = Cpu::inPort(SerialPortManager::pSerialPorts[2]->baseAddress + SERIAL_PORT_REG_IIR);
	}

	asm volatile ("popal");
	asm volatile ("popfl");
	Cpu::sti();
	asm volatile ("iretl");

}

Fifo<unsigned char>* SerialPort::getRecvFifo(){
	return fifoRecv;
}

Fifo<unsigned char>* SerialPort::getSendFifo(){
	return fifoSend;
}

SerialPort::SerialPort(SERIAL_PORT_BASE_ADDRESS _baseAddress, SERIAL_PORT_IRQ _irq)
	:	baseAddress(_baseAddress), irq(_irq), fifoRecv(new Fifo<unsigned char>(RECV_FIFO_SIZE)), fifoSend(new Fifo<unsigned char>(SEND_FIFO_SIZE)), pSendData(nullptr), sendDataSize(0), sendDataCount(0),
	portSpeed(SERIAL_PORT_SPEED_57600){
	switch (baseAddress){
		case SERIAL_PORT_BASE_ADDRESS_1:
			Interrupts::setInterruptHandler(Interrupts::HARDWARE_VECTOR_OFFSET + irq, reinterpret_cast<unsigned int>(asmSerialPort1InterruptHandler));
			break;
		case SERIAL_PORT_BASE_ADDRESS_2:
			Interrupts::setInterruptHandler(Interrupts::HARDWARE_VECTOR_OFFSET + irq, reinterpret_cast<unsigned int>(asmSerialPort2InterruptHandler));
			break;
		case SERIAL_PORT_BASE_ADDRESS_3:
			Interrupts::setInterruptHandler(Interrupts::HARDWARE_VECTOR_OFFSET + irq, reinterpret_cast<unsigned int>(asmSerialPort3InterruptHandler));
			break;
		default:
			break;
	}

	Cpu::outPort(baseAddress + SERIAL_PORT_REG_LCR, static_cast<unsigned char>(0x80));
	Cpu::outPort(baseAddress + 0, static_cast<unsigned char>(SERIAL_PORT_SPEED_57600));
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_IER, static_cast<unsigned char>(SERIAL_PORT_SPEED_57600 >> 8));
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_LCR, static_cast<unsigned char>(0x03));
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_IIR, static_cast<unsigned char>(0x00));
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_MCR, static_cast<unsigned char>(0x0b));
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_IER, static_cast<unsigned char>(0x03));

	close();
}

SerialPort::~SerialPort(){
	close();
	delete fifoSend;
	delete fifoRecv;
}


void SerialPort::open(){
print("SerialPort::open", 40, getNextPos());
printUInt(irq, 40, getNextPos());
	Interrupts::enablingHardwareInterrupt(irq);
}

void SerialPort::close(){
	Interrupts::disablingHardwareInterrupt(irq);
}

void SerialPort::setSpeed(SERIAL_PORT_SPEED speed){
	portSpeed = speed;

	Cpu::cli();
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_LCR, static_cast<unsigned char>(0x80));
	Cpu::outPort(baseAddress + 0, static_cast<unsigned char>(speed));
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_IER, static_cast<unsigned char>(speed >> 8));
	Cpu::outPort(baseAddress + SERIAL_PORT_REG_LCR, static_cast<unsigned char>(0x03));
	Cpu::sti();
}

void SerialPort::irqHandlerRecv(){
	unsigned char lsr = Cpu::inPort(baseAddress + SERIAL_PORT_REG_LSR);

	while ((lsr & SERIAL_PORT_LSR_DR) != 0){
		unsigned char value = Cpu::inPort(baseAddress);
		if ((lsr & (SERIAL_PORT_LSR_OE | SERIAL_PORT_LSR_FE)) == 0)
			fifoRecv->put(reinterpret_cast<unsigned char*>(&value));

		lsr = Cpu::inPort(baseAddress + SERIAL_PORT_REG_LSR);
	}
}

void SerialPort::irqHandlerSend(){
	if (sendDataCount == sendDataSize)
		sendActive = false;
	else{
		Cpu::outPort(baseAddress, static_cast<unsigned char>(pSendData[sendDataCount]));
		sendDataCount++;
	}
}

bool SerialPort::isSendActive(){
	return sendActive;
}

void SerialPort::startSend(){
	if (!sendActive){
		Cpu::outPort(baseAddress, static_cast<unsigned char>(pSendData[0]));
		sendDataCount++;
		sendActive = true;
	}
}

void SerialPort::setNewSendData(unsigned char* _pData, unsigned int _size){
	SAFE_DELETE_ARRAY(pSendData)
	pSendData = new unsigned char[_size];
	memcpy(pSendData, _pData, _size);
	sendDataCount = 0;
	sendDataSize = _size;
}

unsigned int SerialPort::getSpeed(){
	switch (portSpeed){
		case 1:
			return 115200;
			break;
		case 2:
			return 57600;
			break;
		default:
			break;
	}

	return 115200;
}

SerialPort* SerialPortManager::pSerialPorts[3];

SerialPortManager::SerialPortManager(){
	pSerialPorts[0] = new SerialPort(SERIAL_PORT_BASE_ADDRESS_1, SERIAL_PORT_IRQ_1);
	pSerialPorts[1] = new SerialPort(SERIAL_PORT_BASE_ADDRESS_2, SERIAL_PORT_IRQ_2);
	pSerialPorts[2] = new SerialPort(SERIAL_PORT_BASE_ADDRESS_3, SERIAL_PORT_IRQ_3);
}

SerialPort* SerialPortManager::getPort(SERIAL_PORT port){
	return pSerialPorts[port];
}

