#ifndef _EPOLLPOLLER_H_
#define _EPOLLPOLLER_H_
#include <sys/epoll.h>
#include <vector>

#include "net/poller/Poller.h"

class EPollPoller : public Poller
{
public:
    static EPollPoller* createNew();
    ~EPollPoller();
    virtual bool addIOEvent(IOEvent* event);
    virtual bool updateIOEvent(IOEvent* event);
    virtual bool removeIOEvent(IOEvent* event);
    virtual void handleEvent();

private:
    EPollPoller();

private:
    int mEPollFd;

    typedef std::vector<struct epoll_event> EPollEventList;
    EPollEventList mEPollEventList;
    std::vector<IOEvent*> mEvents;
};

#endif //_EPOLLPOLLER_H_