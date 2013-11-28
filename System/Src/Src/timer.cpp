#include "timer.h"
#include "low.h"
#include "interrupt.h"
#include "cpu.h"

extern "C" void asmTimerInterruptHandler();

Timer::Timer(unsigned int _index, long _period, ITimer *_client)
	: index(_index), period(_period), counter(0), enable(false), client(_client)
{}

Timer::~Timer(){
	TimerManager::getSingleton().deleteTimer(index);
}

void Timer::start(){
	enable = true;
}

void Timer::stop(){
	enable = false;
	counter = 0;
}

bool Timer::isStart(){
	return enable;
}

//================================================================================

Timer* TimerManager::timers[TimerManager::MAX_TIMERS_COUNT];

const unsigned short TimerManager::PIT_CONTROL_WORD = 0x0043;
const unsigned short TimerManager::PIT_COUNTER_0 = 0x0040;
const unsigned short TimerManager::PIT_DIVIDER = 1193;

TimerManager::TimerManager()
	:	timersCount(0){
	for (unsigned int i = 0; i < MAX_TIMERS_COUNT; ++i)
		timers[i] = nullptr;

	Cpu::outPort(PIT_CONTROL_WORD, static_cast<unsigned char>(0x36));

	Cpu::outPort(PIT_COUNTER_0, static_cast<unsigned char>(PIT_DIVIDER));
	Cpu::outPort(PIT_COUNTER_0, static_cast<unsigned char>(PIT_DIVIDER >> 8));


	Interrupts::setInterruptHandler(Interrupts::HARDWARE_VECTOR_OFFSET + 0, reinterpret_cast<unsigned int>(asmTimerInterruptHandler));
	Interrupts::enablingHardwareInterrupt(0);
}

extern "C" void staticTimerIrqHandler(){
	Cpu::cli();
	asm volatile ("pushfl");
	asm volatile ("pushal");

	for (unsigned int i = 0; i < TimerManager::MAX_TIMERS_COUNT; ++i){
		if (TimerManager::timers[i] != nullptr){
			if (TimerManager::timers[i]->enable)
				if (++TimerManager::timers[i]->counter == TimerManager::timers[i]->period){
					TimerManager::timers[i]->client->timerHandler();
					TimerManager::timers[i]->counter = 0;
				}
		}
	}

	asm volatile ("popal");
	asm volatile ("popfl");
	Cpu::sti();
	asm volatile ("iretl");
}

Timer* TimerManager::createTimer(long _period, ITimer *_client){
	for (unsigned int i = 0; i < MAX_TIMERS_COUNT; ++i)
		if (timers[i] == nullptr){
			timersCount++;
			timers[i] = new Timer(i, _period, _client);
			return timers[i];
		}

	return nullptr;
}

void TimerManager::deleteTimer(unsigned int index){
	if (index < MAX_TIMERS_COUNT){
		timersCount--;
		timers[index] = nullptr;
		//TODO ������ ������� �� ��������� - ������ ������
	}
}

int TimerManager::getTimersCount(){
	return timersCount;
}

//================================================================================

ITimer::ITimer(long period){
	pTimer = TimerManager::getSingleton().createTimer(period, this);
}

ITimer::~ITimer(){
	delete pTimer;
}
