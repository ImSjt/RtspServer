#include <unistd.h>

#include "net/EventScheduler.h"
#include "net/UsageEnvironment.h"
#include "net/Event.h"
#include "base/Logging.h"

static void readCallback(void*)
{
    char buf[100];
    int ret;

    ret = read(0, buf, 100);
    buf[ret] = '\0';
    LOG_DEBUG("IOEvent: %s\n", buf);
}

static void triggerCallback(void*)
{
    LOG_DEBUG("TriggerEvent\n");
}

static void timeoutCallback(void*)
{
    LOG_DEBUG("TimerEvent\n");
}

int main(int argc, char* argv[])
{
    /* 默认是/dev/stdout */
    //Logger::setLogFile("xxx.log");

    /* 创建调度器 */
    EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);
    //EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_POLL);
    //EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_EPOLL);

    /* 创建环境变量 */
    UsageEnvironment* env = UsageEnvironment::createNew(scheduler, NULL);

    /* 创建并添加IO事件 */
    IOEvent* ioEvent = IOEvent::createNew(0, NULL); //监听标准输入
    ioEvent->setReadCallback(readCallback);
    ioEvent->enableReadHandling();
    env->scheduler()->addIOEvent(ioEvent);
    
    /* 创建并添加触发事件 */
    TriggerEvent* triggerEvent = TriggerEvent::createNew(NULL);
    triggerEvent->setTriggerCallback(triggerCallback);
    env->scheduler()->addTriggerEvent(triggerEvent);

    /* 创建并添加定时事件 */
    Timer::TimerId timerId;
    TimerEvent* timerEvent = TimerEvent::createNew(NULL);
    timerEvent->setTimeoutCallback(timeoutCallback);
    timerId = env->scheduler()->addTimedEventRunEvery(timerEvent, 3000);

    /* 开始调度 */
    env->scheduler()->loop();

    /* 清除工作 */
    env->scheduler()->removeIOEvent(ioEvent);
    env->scheduler()->removeTimedEvent(timerId);

    return 0;
}