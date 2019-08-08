#include <algorithm>
#include <assert.h>

#include "net/poller/PollPoller.h"
#include "base/Logging.h"

static const int pollTimeout = 10000;

PollPoller* PollPoller::createNew()
{
    return new PollPoller();
}

PollPoller::PollPoller()
{
    
}

PollPoller::~PollPoller()
{

}

bool PollPoller::addIOEvent(IOEvent* event)
{
    return updateIOEvent(event);
}

bool PollPoller::updateIOEvent(IOEvent* event)
{
    int fd = event->getFd();
    if(fd < 0)
    {
        LOG_WARNING("failed to add io event\n");
        return false;
    }

    IOEventMap::iterator it = mEventMap.find(fd);
    if(it != mEventMap.end())
    {
        PollFdMap::iterator it = mPollFdMap.find(fd);
        if(it == mPollFdMap.end())
        {
            LOG_WARNING("can't find fd in map\n");
            return false;
        }

        int index = it->second;
        struct pollfd& pfd = mPollFdList.at(index);
        pfd.events = 0;
        pfd.revents = 0;

        if(event->isReadHandling())
            pfd.events |= POLLIN;
        if(event->isWriteHandling())
            pfd.events |= POLLOUT;
        if(event->isErrorHandling())
            pfd.events |= POLLERR;
    }
    else
    {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = 0;
        pfd.revents = 0;

        if(event->isReadHandling())
            pfd.events |= POLLIN;
        if(event->isWriteHandling())
            pfd.events |= POLLOUT;
        if(event->isErrorHandling())
            pfd.events |= POLLERR;
        
        mPollFdList.push_back(pfd);
        mEventMap.insert(std::make_pair(fd, event));
        mPollFdMap.insert(std::make_pair(fd, mPollFdList.size()-1));
    }

    return true;
}

bool PollPoller::removeIOEvent(IOEvent* event)
{
    int fd = event->getFd();

    /* 查看该任务是否存在 */
    if(mEventMap.find(fd) == mEventMap.end())
        return false;
    
    /* 找到该任务该数组的下标 */
    PollFdMap::iterator it = mPollFdMap.find(fd);
    if(it == mPollFdMap.end())
        return false;
    int index = it->second;
    
    /* 如果不是在数组的最后 */
    if(index != mPollFdList.size() - 1)
    {
        /* 将要删除的元素和最后的元素交换 */
        iter_swap(mPollFdList.begin()+index, mPollFdList.end()-1); 

        /* 更改交换后的元素对应的下标 */
        int tmpFd = mPollFdList.at(index).fd; 
        it = mPollFdMap.find(tmpFd);
        it->second = index;
    }
    
    /* 将事件删除 */
    mPollFdList.pop_back();
    mPollFdMap.erase(fd);
    mEventMap.erase(fd);

    return true;
}

void PollPoller::handleEvent()
{
    int nums, fd, events, revents;

    if(mPollFdList.empty())
        return;

    nums = poll(&*mPollFdList.begin(), mPollFdList.size(), pollTimeout);
    if(nums < 0)
    {
        LOG_ERROR("poll err\n");
        return;
    }

    for(PollFdList::iterator it = mPollFdList.begin();
            it != mPollFdList.end() && nums > 0; ++it)
    {
        events = it->revents;
        if(events > 0)
        {
            revents = 0;
            fd = it->fd;
            IOEventMap::iterator it = mEventMap.find(fd);
            assert(it != mEventMap.end());
            
            if(events & POLLIN || events & POLLPRI || events & POLLRDHUP)
                revents |= IOEvent::EVENT_READ;
            if(events & POLLOUT)
                revents |= IOEvent::EVENT_WRITE;
            if(events & POLLERR)
                revents |= IOEvent::EVENT_ERROR;

            it->second->setREvent(revents);
            mEvents.push_back(it->second);

            --nums;
        }
    }

    for(std::vector<IOEvent*>::iterator it = mEvents.begin(); it != mEvents.end(); ++it)
    {
        (*it)->handleEvent();
    }
    
    mEvents.clear();
}
