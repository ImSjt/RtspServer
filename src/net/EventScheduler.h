#ifndef _EVENT_SCHEDULER_H_
#define _EVENT_SCHEDULER_H_
#include <vector>

#include "net/poller/PollPoller.h"
#include "net/Timer.h"

class EventScheduler
{
public:
    enum PollerType
    {
        POLLER_SELECT,
        POLLER_POLL,
        POLLER_EPOLL
    };

    static EventScheduler* createNew(PollerType type);
    virtual ~EventScheduler();

    bool addTriggerEvent(TriggerEvent* event);
    Timer::TimerId addTimedEventRunAfater(TimerEvent* event, Timer::TimeInterval delay);
    Timer::TimerId addTimedEventRunAt(TimerEvent* event, Timer::Timestamp when);
    Timer::TimerId addTimedEventRunEvery(TimerEvent* event, Timer::TimeInterval interval);
    bool removeTimedEvent(Timer::TimerId timerId);
    bool addIOEvent(IOEvent* event);
    bool updateIOEvent(IOEvent* event);
    bool removeIOEvent(IOEvent* event);

    void loop();
    void wakeup();

private:
    EventScheduler(PollerType type, int fd);
    void handleTriggerEvents();
    static void handleReadCallback(void*);
    void handleRead();

private:
    bool mQuit;
    Poller* mPoller;
    TimerManager* mTimerManager;
    std::vector<TriggerEvent*> mTriggerEvents;
    int mWakeupFd;
    IOEvent* mWakeIOEvent;
};

#endif //_EVENT_SCHEDULER_H_