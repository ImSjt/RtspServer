#ifndef _EVENT_H_
#define _EVENT_H_

#include "base/Logging.h"

typedef void (*EventCallback)(void*);

class TriggerEvent
{
public:
    static TriggerEvent* createNew(void* arg);
    static TriggerEvent* createNew();
    ~TriggerEvent() {  };

    void setArg(void* arg) { mArg = arg; }
    void setTriggerCallback(EventCallback cb) { mTriggerCallback = cb; }
    void handleEvent();

private:
    TriggerEvent(void* arg);

private:
    void* mArg;
    EventCallback mTriggerCallback;
};

class TimerEvent
{
public:
    static TimerEvent* createNew(void* arg);
    static TimerEvent* createNew();

    ~TimerEvent() { }

    void setArg(void* arg) { mArg = arg; }
    void setTimeoutCallback(EventCallback cb) { mTimeoutCallback = cb; }
    void handleEvent();

private:
    TimerEvent(void* arg);

private:
    void* mArg;
    EventCallback mTimeoutCallback;
};

class IOEvent
{
public:
    enum IOEventType
    {
        EVENT_NONE = 0,
        EVENT_READ = 1,
        EVENT_WRITE = 2,
        EVENT_ERROR = 4,
    };
    
    static IOEvent* createNew(int fd, void* arg);
    static IOEvent* createNew(int fd);

    ~IOEvent() { }

    int getFd() const { return mFd; }
    int getEvent() const { return mEvent; }
    void setREvent(int event) { mREvent = event; }
    void setArg(void* arg) { mArg = arg; }

    void setReadCallback(EventCallback cb) { mReadCallback = cb; };
    void setWriteCallback(EventCallback cb) { mWriteCallback = cb; };
    void setErrorCallback(EventCallback cb) { mErrorCallback = cb; };

    void enableReadHandling() { mEvent |= EVENT_READ; }
    void enableWriteHandling() { mEvent |= EVENT_WRITE; }
    void enableErrorHandling() { mEvent |= EVENT_ERROR; }
    void disableReadeHandling() { mEvent &= ~EVENT_READ; }
    void disableWriteHandling() { mEvent &= ~EVENT_WRITE; }
    void disableErrorHandling() { mEvent &= ~EVENT_ERROR; }

    bool isNoneHandling() const { return mEvent == EVENT_NONE; }
    bool isReadHandling() const { return (mEvent & EVENT_READ) != 0; }
    bool isWriteHandling() const { return (mEvent & EVENT_WRITE) != 0; }
    bool isErrorHandling() const { return (mEvent & EVENT_ERROR) != 0; };

    void handleEvent();

private:
    IOEvent(int fd, void* arg);

private:
    int mFd;
    void* mArg;
    int mEvent;
    int mREvent;
    EventCallback mReadCallback;
    EventCallback mWriteCallback;
    EventCallback mErrorCallback;
};

#endif //_EVENT_H_