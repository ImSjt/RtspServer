#ifndef _THREAD_H_
#define _THREAD_H_
#include <pthread.h>

class Thread
{
public:
    virtual ~Thread();
    
    bool start(void *arg);
    bool detach();
    bool join();
    bool cancel();
    pthread_t getThreadId() const;

protected:
    Thread();

    virtual void run(void *arg) = 0;

private:
    static void *threadRun(void *);

private:
    void *mArg;
    bool mIsStart;
    bool mIsDetach;
    pthread_t mThreadId;
};

#endif //_THREAD_H_

