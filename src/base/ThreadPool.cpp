#include "base/ThreadPool.h"
#include "base/Logging.h"

ThreadPool* ThreadPool::createNew(int num)
{
    return new ThreadPool(num);
}

ThreadPool::ThreadPool(int num) :
    mThreads(num),
    mQuit(false)
{
    mMutex = Mutex::createNew();
    mCondition = Condition::createNew();

    createThreads();
}

ThreadPool::~ThreadPool()
{
    cancelThreads();
    delete mMutex;
    delete mCondition;
}

void ThreadPool::addTask(ThreadPool::Task& task)
{
    MutexLockGuard mutexLockGuard(mMutex);
    mTaskQueue.push(task);
    mCondition->signal();
}

void ThreadPool::handleTask()
{
    while(mQuit != true)
    {
        Task task;
        {
            MutexLockGuard mutexLockGuard(mMutex);
            if(mTaskQueue.empty())
                mCondition->wait(mMutex);
        
            if(mQuit == true)
                break;

            if(mTaskQueue.empty())
                continue;

            task = mTaskQueue.front();

            mTaskQueue.pop();
        }

        task.handle();
    }
}

void ThreadPool::createThreads()
{
    MutexLockGuard mutexLockGuard(mMutex);

    for(std::vector<MThread>::iterator it = mThreads.begin(); it != mThreads.end(); ++it)
        (*it).start(this);
}

void ThreadPool::cancelThreads()
{
    MutexLockGuard mutexLockGuard(mMutex);

    mQuit = true;
    mCondition->broadcast();
    for(std::vector<MThread>::iterator it = mThreads.begin(); it != mThreads.end(); ++it)
        (*it).join();

    mThreads.clear();
}

void ThreadPool::MThread::run(void* arg)
{
    ThreadPool* threadPool = (ThreadPool*)arg;
    threadPool->handleTask();    
}