#ifndef TIMER_H
#define TIMER_H

#include "singleton.h"

class ITimer;
class TimerManager;

//! ����� �������.
class Timer{
friend class TimerManager;
private:
public:
    unsigned int index; /*!< ������ ������� � ������� */
    long period; /*!< ������ ������� � ������� */
    long counter; /*!< ������� ������� ������� */
    bool enable; /*!< ������� �������� �������: true - �������, false - �������� */
    ITimer *client; /*!< ��������� �� ������ ���������� ����� ���������� ������� */

    Timer();
    Timer(unsigned int _index, long _period, ITimer *_client);
    Timer(const Timer& copy);
    Timer& operator =(const Timer &a);

public:
    ~Timer();
    //! ��������� �������
    void start();
    //! ���������� �������
    void stop();

    //! �������� ������� �� ������
    /*!
        @return true - �������, false - ��������
    */
    bool isStart();
    //! �������� ������ ������� � �������
    /*!
        @return ������ ������� � �������
    */
    unsigned int getIndex();
};

//! ����� ��������� ��������.
/*!
��������� �����.
����������� ��������� ����, ��������, ������� � ������� �������, �������� �������� �������� ��������.
*/
class TimerManager : public Singleton<TimerManager>{
private:
    static const unsigned short PIT_CONTROL_WORD; /*!<  */
    static const unsigned short PIT_COUNTER_0; /*!<  */
    static const unsigned short PIT_DIVIDER; /*!<  */

public:
    static const unsigned int MAX_TIMERS_COUNT = 300; /*!< ������������ ����� �������� � ������� */
    static Timer* timers[MAX_TIMERS_COUNT]; /*!< ������ �������� */

    //! ����������� ���������� ���������� ���������� �������
    /*!
        ������������ ���������� ���������� �������.
        ����������� ������� � ���� ���������� ��������. ��� ���������� �����-������ �������� ������ ������� ��� ������� ���������� � ���������� �����-���������� � �������.
    */

public:
    TimerManager();
    //! ������� ������
    /*!
        ���� � ������� �������� ���� �����, �� ������� ����� ������ � �������� ��������� �� ���� � ������

        @param period  - ������ ������� � �������
        @param _client - ��������� �� ������ ���������� ���������� �������
        @return ��������� �� ��������� ������ ��� nullptr, ���� ������ ������� �� �������.
    */
    Timer* createTimer(long period, ITimer *_client);
    //! ������� ������
    /*!
        ������� ������ � ������� ��������� �� ���� �� �������.

        @param index  - ������ �������. ������ �� ����������� �� ����� �� ������� �������.
    */
    void deleteTimer(unsigned int index);

private:
    int timersCount; /*!< ����� �������� � �������. */
public:
    //! �������� ����� �������� � �������
    /*!
        @return ����� �������� � �������
    */
    int getTimersCount();
};

//! ��������� ������� �������
/*!
�� ����� ������ ����������� ������, ������� ������ ��������� ������������� ��������.
*/
class ITimer{
public:
    Timer *pTimer; /*!< ��������� �� ������. */
    //! �����������
    /*!
        �������� ������ �� TimerManager � ��������� ��� � pTimer

        @param period - ������ ������� � �������
    */
    ITimer(long period);
    virtual ~ITimer();
    //! ����� ����������� ����� ��������� ������� ������� � ���������� �������.
    /*!
        ����� �������� TimerManager ����� ������� ������� ������ �������������� �������.
    */
    virtual void timerHandler() = 0;
};
#endif
