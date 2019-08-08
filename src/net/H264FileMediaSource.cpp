#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "net/H264FileMediaSource.h"
#include "base/Logging.h"

static inline int startCode3(uint8_t* buf);
static inline int startCode4(uint8_t* buf);

H264FileMediaSource* H264FileMediaSource::createNew(UsageEnvironment* env, std::string file)
{
    return new H264FileMediaSource(env, file);
}

H264FileMediaSource::H264FileMediaSource(UsageEnvironment* env, std::string& file) :
    MediaSource(env),
    mFile(file)
{
    mFd = ::open(file.c_str(), O_RDONLY);
    assert(mFd > 0);

    setFps(25);

    for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
        mEnv->threadPool()->addTask(mTask);
}

H264FileMediaSource::~H264FileMediaSource()
{
    ::close(mFd);
}

void H264FileMediaSource::readFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);

    if(mAVFrameInputQueue.empty())
        return;

    AVFrame* frame = mAVFrameInputQueue.front();

    frame->mFrameSize = getFrameFromH264File(mFd, frame->mBuffer, FRAME_MAX_SIZE);
    if(frame->mFrameSize < 0)
        return;

    if(startCode3(frame->mBuffer))
    {
        frame->mFrame = frame->mBuffer+3;
        frame->mFrameSize -= 3;
    }
    else
    {
        frame->mFrame = frame->mBuffer+4;
        frame->mFrameSize -= 4;
    }

    mAVFrameInputQueue.pop();
    mAVFrameOutputQueue.push(frame);
}

static inline int startCode3(uint8_t* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

static inline int startCode4(uint8_t* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}

static uint8_t* findNextStartCode(uint8_t* buf, int len)
{
    int i;

    if(len < 3)
        return NULL;

    for(i = 0; i < len-3; ++i)
    {
        if(startCode3(buf) || startCode4(buf))
            return buf;
        
        ++buf;
    }

    if(startCode3(buf))
        return buf;

    return NULL;
}

int H264FileMediaSource::getFrameFromH264File(int fd, uint8_t* frame, int size)
{
    int rSize, frameSize;
    uint8_t* nextStartCode;

    if(fd < 0)
        return fd;

    rSize = read(fd, frame, size);
    if(!startCode3(frame) && !startCode4(frame))
        return -1;
    
    nextStartCode = findNextStartCode(frame+3, rSize-3);
    if(!nextStartCode)
    {
        lseek(fd, 0, SEEK_SET);
        frameSize = rSize;
    }
    else
    {
        frameSize = (nextStartCode-frame);
        lseek(fd, frameSize-rSize, SEEK_CUR);
    }

    return frameSize;
}