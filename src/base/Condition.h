#ifndef _CONDITION_H_
#define _CONDITION_H_
#include <pthread.h>

#include "Mutex.h"

class Condition
{
public:
    static Condition* createNew();    
    ~Condition();

    void wait(Mutex* mutex);
    bool waitTimeout(Mutex* mutex, int ms);
    void signal();
    void broadcast();

private:
    Condition();

private:
    pthread_cond_t mCond;
};

#endif //_CONDITION_H_