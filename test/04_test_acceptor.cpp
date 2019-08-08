#include <iostream>

#include "base/Logging.h"
#include "net/UsageEnvironment.h"
#include "net/EventScheduler.h"
#include "net/Acceptor.h"
#include "net/SocketsOps.h"

int main(int argc, char* argv[])
{
    /* 默认是/dev/stdout */
    //Logger::setLogFile("xxx.log");
 
    /* 创建调度器 */
    EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);
    //EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_POLL);
    //EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_EPOLL);
    UsageEnvironment* env = UsageEnvironment::createNew(scheduler, NULL);

    uint16_t port = 8554;
    Ipv4Address addr("0.0.0.0", port);
    Acceptor* acceptor = Acceptor::createNew(env, addr);
    acceptor->listen();

    std::cout<<"ip:"<<sockets::getLocalIp()<<"; port:"<<port<<std::endl;

    env->scheduler()->loop();

    return 0;
}