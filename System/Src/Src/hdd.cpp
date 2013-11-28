#include "hdd.h"
#include "cpu.h"
#include "interrupt.h"

unsigned short Hdd::ideBaseAddress;
volatile unsigned int Hdd::actionCounter;
Hdd::RW_FLAG Hdd::rwFlag;
Hdd::PRDF Hdd::prdf;

unsigned char Hdd::sectorsCount;
unsigned char* Hdd::pData;

unsigned char Hdd::buffer[Hdd::PRDF_BUFFER_SIZE];

volatile unsigned char _wwr1;
volatile unsigned char _wwr2;
volatile unsigned char _aassww = 0;

extern "C" void asmHddInterruptHandler();

extern "C" void staticHddIrqHandler(){
	Cpu::cli();
	asm volatile ("pushfl");
	asm volatile ("pushal");

	Cpu::outPort(Hdd::ideBaseAddress, static_cast<unsigned char>(0x00));
	Cpu::inPort(IDE_STATUS_COMMAND_R);

	if (Hdd::rwFlag == Hdd::RW_FLAG_READ)
		memcpy(Hdd::pData, Hdd::buffer, Hdd::sectorsCount * 512);

	Hdd::actionCounter--;

	Cpu::outPort(0xa0, static_cast<unsigned char>(0x20));

	asm volatile ("popal");
	asm volatile ("popfl");
	Cpu::sti();
	asm volatile ("iretl");
}

Hdd::Hdd(){
	actionCounter = 0;
	rwFlag = RW_FLAG_READ;

	prdf.MRPBA = reinterpret_cast<unsigned int>(buffer);
	prdf.EOT_SIZE = PRDF_EOT;

	asm volatile("\tmovl	$0x51300008, %ecx\n");
	asm volatile("\trdmsr\n");
	asm volatile("\tandl	$0xfffffff0, %eax\n");
	asm volatile("\tmovl	%%eax, %0\n": : "m" (Hdd::ideBaseAddress));

	Interrupts::setInterruptHandler(Interrupts::HARDWARE_VECTOR_OFFSET + 14, reinterpret_cast<unsigned int>(asmHddInterruptHandler));
	Interrupts::enablingHardwareInterrupt(14);
	Interrupts::enablingHardwareInterrupt(2);
}

Hdd::~Hdd(){
	Interrupts::disablingHardwareInterrupt(2);
	Interrupts::disablingHardwareInterrupt(14);
}

unsigned int Hdd::getActionCounter(){
	return actionCounter;
}

/**
������������ ����� �������� ��������: 65536 / 512 = 128
*/
bool Hdd::read(unsigned int _startSectorNumber, unsigned char _sectorsCount, unsigned char* _pData){
	actionCounter++;
	if (actionCounter > 1){
		actionCounter--;
		return false;
	}

	pData = _pData;
	sectorsCount = _sectorsCount;

	if ((_sectorsCount == 0) ||(_sectorsCount > MAX_SECTOR_COUNT)){
		actionCounter--;
		return false;
	}

	rwFlag = RW_FLAG_READ;

	if (_sectorsCount == MAX_SECTOR_COUNT)
		prdf.EOT_SIZE = PRDF_EOT;
	else
		prdf.EOT_SIZE = PRDF_EOT + _sectorsCount * 512;

	PRDF* pPrdf = &prdf;

	Cpu::outPort(IDE_DEVICE_CONTROL_R, static_cast<unsigned char>(0x00));
	while ((Cpu::inPort(IDE_STATUS_COMMAND_R) & IDE_BUSY) != 0){}

	Cpu::outPort(IDE_LBA_3_R, static_cast<unsigned char>(0xf0)); // slave
//	Cpu::outPort(IDE_LBA_3_R, static_cast<unsigned char>(0x70)); // master

	do{
		while ((Cpu::inPort(IDE_STATUS_COMMAND_R) & IDE_BUSY) != 0){}
	}
	while((Cpu::inPort(IDE_STATUS_COMMAND_R) & IDE_DRDY) == 0);

	Cpu::outPort(IDE_LBA_0_R, _startSectorNumber);
	Cpu::outPort(IDE_LBA_1_R, _startSectorNumber >> 8);
	Cpu::outPort(IDE_LBA_2_R, _startSectorNumber >> 16);
	Cpu::outPort(IDE_SECTOR_COUNT_R, _sectorsCount);

	Cpu::outPort(ideBaseAddress + 4, reinterpret_cast<unsigned int>(pPrdf));
	Cpu::outPort(ideBaseAddress, static_cast<unsigned char>(IDE_BM_CMD_RWCTL));
	Cpu::outPort(ideBaseAddress + 2, static_cast<unsigned char>(0x06));
	Cpu::outPort(IDE_STATUS_COMMAND_R, static_cast<unsigned char>(READ_DMA_WITH_RETRY));
	Cpu::outPort(ideBaseAddress, static_cast<unsigned char>(IDE_BM_CMD_RWCTL | IDE_BM_CMD_BMCTL));

	return true;
}

/**
������������ ����� ������������ ��������: 65536 / 512 = 128
*/
bool Hdd::write(unsigned int _startSectorNumber, unsigned char _sectorsCount, unsigned char* _pData){
	actionCounter++;
	if (actionCounter > 1){
		actionCounter--;
		return false;
	}

	if ((_sectorsCount == 0) || (_sectorsCount > MAX_SECTOR_COUNT)){
		actionCounter--;
		return false;
	}

	rwFlag = RW_FLAG_WRITE;

	if (_sectorsCount == MAX_SECTOR_COUNT)
		prdf.EOT_SIZE = PRDF_EOT;
	else
		prdf.EOT_SIZE = PRDF_EOT + _sectorsCount * 512;

	PRDF* pPrdf = &prdf;

	memcpy(buffer, _pData, _sectorsCount * 512);

	Cpu::outPort(IDE_DEVICE_CONTROL_R, static_cast<unsigned char>(0x00));
	while ((Cpu::inPort(IDE_STATUS_COMMAND_R) & IDE_BUSY) != 0){}
	Cpu::outPort(IDE_LBA_3_R, static_cast<unsigned char>(0xf0)); // slave

	do{
		while ((Cpu::inPort(IDE_STATUS_COMMAND_R) & IDE_BUSY) != 0){}
	}
	while((Cpu::inPort(IDE_STATUS_COMMAND_R) & IDE_DRDY) == 0);

	Cpu::outPort(IDE_LBA_0_R, _startSectorNumber);
	Cpu::outPort(IDE_LBA_1_R, _startSectorNumber >> 8);
	Cpu::outPort(IDE_LBA_2_R, _startSectorNumber >> 16);
	Cpu::outPort(IDE_SECTOR_COUNT_R, _sectorsCount);

	Cpu::outPort(ideBaseAddress + 4, reinterpret_cast<unsigned int>(pPrdf));
	Cpu::outPort(ideBaseAddress, static_cast<unsigned char>(0x00));
	Cpu::outPort(ideBaseAddress + 2, static_cast<unsigned char>(0x06));
	Cpu::outPort(IDE_STATUS_COMMAND_R, static_cast<unsigned char>(READ_DMA_WITH_RETRY));
	Cpu::outPort(ideBaseAddress, static_cast<unsigned char>(IDE_BM_CMD_BMCTL));

	return true;
}
