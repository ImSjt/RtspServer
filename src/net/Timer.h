#ifndef _TIMER_H_
#define _TIMER_H_
#include <map>
#include <stdint.h>

#include "net/poller/PollPoller.h"
#include "net/Event.h"

class Timer
{
public:
    typedef uint32_t TimerId;
    typedef int64_t Timestamp; //ms
    typedef uint32_t TimeInterval; //ms

    ~Timer();

    static Timestamp getCurTime();

private:
    friend class TimerManager;
    Timer(TimerEvent* event, Timestamp timestamp, TimeInterval timeInterval);
    void handleEvent();

private:
    TimerEvent* mTimerEvent;
    Timestamp mTimestamp;
    TimeInterval mTimeInterval;
    bool mRepeat;
};

class TimerManager
{
public:
    static TimerManager* createNew(Poller* poller);
    ~TimerManager();

    Timer::TimerId addTimer(TimerEvent* event, Timer::Timestamp timestamp,
                                Timer::TimeInterval timeInterval);
    bool removeTimer(Timer::TimerId timerId);

private:
    TimerManager(int timerFd, Poller* poller);
    void modifyTimeout();
    static void handleRead(void*);
    void handleTimerEvent();

private:
    Poller* mPoller;
    int mTimerFd;
    std::map<Timer::TimerId, Timer> mTimers;
    
    typedef std::pair<Timer::Timestamp, Timer::TimerId> TimerIndex;
    std::multimap<TimerIndex, Timer> mEvents;
    uint32_t mLastTimerId;
    IOEvent* mTimerIOEvent;
};

#endif //_TIMER_H_