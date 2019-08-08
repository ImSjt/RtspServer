#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "net/poller/EPollPoller.h"
#include "base/Logging.h"

static const int InitEventListSize = 16;
static const int epollTimeout = 10000;

EPollPoller* EPollPoller::createNew()
{
    return new EPollPoller();
}

EPollPoller::EPollPoller() :
    mEPollEventList(InitEventListSize)
{
    mEPollFd = ::epoll_create1(EPOLL_CLOEXEC);
}

EPollPoller::~EPollPoller()
{
    ::close(mEPollFd);
}

bool EPollPoller::addIOEvent(IOEvent* event)
{
    return updateIOEvent(event);
}

bool EPollPoller::updateIOEvent(IOEvent* event)
{
    struct epoll_event epollEvt;
    int fd = event->getFd();

    memset(&epollEvt, 0, sizeof(epollEvt));
    epollEvt.data.fd = fd;
    if(event->isReadHandling())
        epollEvt.events |= EPOLLIN;
    if(event->isWriteHandling())
        epollEvt.events |= EPOLLOUT;
    if(event->isErrorHandling())
        epollEvt.events |= EPOLLERR;

    IOEventMap::iterator it = mEventMap.find(fd);
    if(it != mEventMap.end())
    {
        epoll_ctl(mEPollFd, EPOLL_CTL_MOD, fd, &epollEvt);
    }
    else
    {
        epoll_ctl(mEPollFd, EPOLL_CTL_ADD, fd, &epollEvt);
        mEventMap.insert(std::make_pair(fd, event));
        if(mEventMap.size() >= mEPollEventList.size())
            mEPollEventList.resize(mEPollEventList.size() * 2);
    }

    return true;
}

bool EPollPoller::removeIOEvent(IOEvent* event)
{
    int fd = event->getFd();
    IOEventMap::iterator it = mEventMap.find(fd);
    if(it == mEventMap.end())
        return false;
    
    epoll_ctl(mEPollFd, EPOLL_CTL_DEL, fd, NULL);
    mEventMap.erase(fd);

    return true;
}

void EPollPoller::handleEvent()
{
    int nums, fd, event, revent;
    IOEventMap::iterator it;

    nums = epoll_wait(mEPollFd, &*mEPollEventList.begin(), mEPollEventList.size(), epollTimeout);
    if(nums < 0)
    {
        LOG_DEBUG("epoll wait err\n");
        return;
    }

    for(int i = 0; i < nums; ++i)
    {
        revent = 0;
        fd = mEPollEventList.at(i).data.fd;
        event = mEPollEventList.at(i).events;
        if(event & EPOLLIN || event & EPOLLPRI || event & EPOLLRDHUP)
            revent |= IOEvent::EVENT_READ;
        if(event & EPOLLOUT)
            revent |= IOEvent::EVENT_WRITE;
        if(event & EPOLLERR)
            revent |= IOEvent::EVENT_ERROR;

        it = mEventMap.find(fd);
        assert(it != mEventMap.end());

        it->second->setREvent(revent);
        mEvents.push_back(it->second);
    }

    for(std::vector<IOEvent*>::iterator it = mEvents.begin(); it != mEvents.end(); ++it)
        (*it)->handleEvent();
    
    mEvents.clear();
}

