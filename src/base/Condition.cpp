#include <sys/time.h>
#include <time.h>

#include "Condition.h"

Condition* Condition::createNew()
{
    return new Condition();
}

Condition::Condition()
{
    pthread_cond_init(&mCond, NULL);
}

Condition::~Condition()
{
    pthread_cond_destroy(&mCond);
}

void Condition::wait(Mutex* mutex)
{
    pthread_cond_wait(&mCond, mutex->get());
}

bool Condition::waitTimeout(Mutex* mutex, int ms)
{
    struct timespec abstime;
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);

    abstime.tv_sec = now.tv_sec + ms/1000;
    abstime.tv_nsec = now.tv_nsec + ms%1000*1000*1000;

    if(pthread_cond_timedwait(&mCond, mutex->get(), &abstime) == 0)
        return true;
    else
        return false;
    
}

void Condition::signal()
{
    pthread_cond_signal(&mCond);
}

void Condition::broadcast()
{
    pthread_cond_broadcast(&mCond);
}
