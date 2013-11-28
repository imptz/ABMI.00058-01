#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "memory.h"
#include "display.h"
#include "cpu.h"

extern "C" void defaultProcessorInterruptHandler();
extern "C" void defaultHardware0InterruptHandler();
extern "C" void defaultHardware1InterruptHandler();

class Interrupts{
private:
	static const unsigned int IDT_ADDRESS = 0x00002000;
	static const unsigned int IDT_REG_ADDRESS = 0x00003000;
	static const unsigned int IDT_DESCRIPTOR_SIZE = 8;
	static const unsigned int IDT_SIZE = 256 * IDT_DESCRIPTOR_SIZE;

	static void createIdt(){
		memset(reinterpret_cast<unsigned char*>(IDT_ADDRESS), static_cast<unsigned char>(0), IDT_SIZE);

		*(reinterpret_cast<unsigned short*>(IDT_REG_ADDRESS)) = IDT_SIZE - 1;
		*(reinterpret_cast<unsigned int*>(IDT_REG_ADDRESS + 2)) = IDT_ADDRESS;

		Cpu::setIdt(IDT_REG_ADDRESS);
	}

public:
	static void setInterruptHandler(int index, unsigned int offset){
		const unsigned int IDT_DESCRIPTOR_FLAGS = 0x00008e00;
		const unsigned int IDT_SEGMENT_SELECTOR = 0x00080000;

		unsigned int lowDesc = (offset & 0x0000ffff) | IDT_SEGMENT_SELECTOR;
		unsigned int highDesc = IDT_DESCRIPTOR_FLAGS | (offset & 0xffff0000);

		*(reinterpret_cast<unsigned int*>(IDT_ADDRESS + IDT_DESCRIPTOR_SIZE * index)) = lowDesc;
		*(reinterpret_cast<unsigned int*>(IDT_ADDRESS + IDT_DESCRIPTOR_SIZE * index + 4)) = highDesc;
	}

private:
	static void relocationHardwareInterrupts(){
		Cpu::outPort(0x20, static_cast<unsigned char>(0x11));
		Cpu::outPort(0xa0, static_cast<unsigned char>(0x11));

		Cpu::outPort(0x21, static_cast<unsigned char>(0x20));
		Cpu::outPort(0xa1, static_cast<unsigned char>(0x28));

		Cpu::outPort(0x21, static_cast<unsigned char>(0x04));
		Cpu::outPort(0xa1, static_cast<unsigned char>(0x02));

		Cpu::outPort(0x21, static_cast<unsigned char>(0x11));
		Cpu::outPort(0xa1, static_cast<unsigned char>(0x01));

		Cpu::outPort(0x04d0, static_cast<unsigned char>(0x00));
		Cpu::outPort(0x04d1, static_cast<unsigned char>(0x00));

		Cpu::outPort(0x20, static_cast<unsigned char>(0x6b));
		Cpu::outPort(0xa0, static_cast<unsigned char>(0x6b));
	}

public:
	static void initInterrupts(){
		Cpu::cli();
		createIdt();

		setInterruptHandler(0, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(1, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(2, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(3, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(4, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(5, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(6, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(7, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(8, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(9, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(10, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(11, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(12, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(13, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(14, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(15, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(16, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(17, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(18, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(19, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(20, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(21, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(22, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(23, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(24, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(25, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(26, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(27, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(28, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(29, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(30, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));
		setInterruptHandler(31, reinterpret_cast<unsigned int>(defaultProcessorInterruptHandler));


		relocationHardwareInterrupts();
		disablingNmi();
		for (int i = 0; i < 16; ++i)
			Interrupts::disablingHardwareInterrupt(i);

		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 0, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 1, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 2, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 3, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 4, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 5, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 6, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 7, reinterpret_cast<unsigned int>(defaultHardware0InterruptHandler));

		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 8, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 9, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 10, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 11, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 12, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 13, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 14, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
		setInterruptHandler(HARDWARE_VECTOR_OFFSET + 15, reinterpret_cast<unsigned int>(defaultHardware1InterruptHandler));
	}

	static const unsigned int HARDWARE_VECTOR_OFFSET = 32;

	static void enablingHardwareInterrupt(int intNumber)
	{
		unsigned char num;
		if (intNumber < 8)
		{
			num = ~(1 << intNumber);
			unsigned char inV = Cpu::inPort(0x21);
			inV &= num;
			Cpu::outPort(0x21, inV);
		}
		else
		{
			num = ~(1 << (intNumber - 8));
			unsigned char inV = Cpu::inPort(0xa1);
			inV &= num;
			Cpu::outPort(0xa1, inV);
		}
	}

	static void disablingHardwareInterrupt(int intNumber)
	{
		unsigned char num;
		if (intNumber < 8)
		{
			num = 1 << intNumber;
			volatile unsigned char inV = Cpu::inPort(0x21);
			inV |= num;
			Cpu::outPort(0x21, inV);
		}
		else
		{
			num = 1 << (intNumber - 8);
			volatile unsigned char inV = Cpu::inPort(0xa1);
			inV |= num;
			Cpu::outPort(0xa1, inV);
		}
	}

	static void disablingNmi()
	{
		Cpu::outPort(0x70, static_cast<unsigned char>(0x80));
	}

	static void enablingNmi()
	{
		Cpu::outPort(0x70, static_cast<unsigned char>(Cpu::inPort(0x70) & 0x7f));
	}
};
#endif
