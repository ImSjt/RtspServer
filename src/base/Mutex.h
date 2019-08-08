#ifndef _MUTEX_H_
#define _MUTEX_H_
#include <pthread.h>

class Mutex
{
public:
    static Mutex* createNew();
    ~Mutex();

    void lock();
    void unlock();
    
    pthread_mutex_t* get() { return &mMutex; };

private:
    Mutex();

private:
    pthread_mutex_t mMutex;

};

class MutexLockGuard
{
public:
    MutexLockGuard(Mutex* mutex);
    ~MutexLockGuard();

private:
    Mutex* mMutex;

};

#endif //_MUTEX_H_