#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "net/AACFileMediaSource.h"
#include "base/Logging.h"

AACFileMeidaSource* AACFileMeidaSource::createNew(UsageEnvironment* env, std::string file)
{
    return new AACFileMeidaSource(env, file);
}

AACFileMeidaSource::AACFileMeidaSource(UsageEnvironment* env, std::string& file) :
    MediaSource(env),
    mFile(file)
{
    mFd = ::open(file.c_str(), O_RDONLY);
    assert(mFd > 0);

    setFps(43);

    for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
        mEnv->threadPool()->addTask(mTask);
}

AACFileMeidaSource::~AACFileMeidaSource()
{
    ::close(mFd);
}

void AACFileMeidaSource::readFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);

    if(mAVFrameInputQueue.empty())
        return;

    AVFrame* frame = mAVFrameInputQueue.front();

    frame->mFrameSize = getFrameFromAACFile(mFd, frame->mBuffer, FRAME_MAX_SIZE);
    if(frame->mFrameSize < 0)
        return;
    frame->mFrame = frame->mBuffer;

    mAVFrameInputQueue.pop();
    mAVFrameOutputQueue.push(frame);
}

bool AACFileMeidaSource::parseAdtsHeader(uint8_t* in, struct AdtsHeader* res)
{
    memset(res,0,sizeof(*res));

    if ((in[0] == 0xFF)&&((in[1] & 0xF0) == 0xF0))
    {
        res->id = ((unsigned int) in[1] & 0x08) >> 3;
        res->layer = ((unsigned int) in[1] & 0x06) >> 1;
        res->protectionAbsent = (unsigned int) in[1] & 0x01;
        res->profile = ((unsigned int) in[2] & 0xc0) >> 6;
        res->samplingFreqIndex = ((unsigned int) in[2] & 0x3c) >> 2;
        res->privateBit = ((unsigned int) in[2] & 0x02) >> 1;
        res->channelCfg = ((((unsigned int) in[2] & 0x01) << 2) | (((unsigned int) in[3] & 0xc0) >> 6));
        res->originalCopy = ((unsigned int) in[3] & 0x20) >> 5;
        res->home = ((unsigned int) in[3] & 0x10) >> 4;
        res->copyrightIdentificationBit = ((unsigned int) in[3] & 0x08) >> 3;
        res->copyrightIdentificationStart = (unsigned int) in[3] & 0x04 >> 2;
        res->aacFrameLength = (((((unsigned int) in[3]) & 0x03) << 11) |
                                (((unsigned int)in[4] & 0xFF) << 3) |
                                    ((unsigned int)in[5] & 0xE0) >> 5) ;
        res->adtsBufferFullness = (((unsigned int) in[5] & 0x1f) << 6 |
                                        ((unsigned int) in[6] & 0xfc) >> 2);
        res->numberOfRawDataBlockInFrame = ((unsigned int) in[6] & 0x03);

        return true;
    }
    else
    {
        LOG_WARNING("failed to parse adts header\n");
        return false;
    }
}

int AACFileMeidaSource::getFrameFromAACFile(int fd, uint8_t* buf, int size)
{
    uint8_t tmpBuf[7];
    int ret;

    ret = read(fd, tmpBuf, 7);
    if(ret <= 0)
    {
        lseek(fd, 0, SEEK_SET);
        ret = read(fd, tmpBuf, 7);
        if(ret <= 0)
            return -1;
    }

    if(parseAdtsHeader(tmpBuf, &mAdtsHeader) != true)
    {
        LOG_WARNING("parse err\n");
        return -1;
    }

    if(mAdtsHeader.aacFrameLength > size)
        return -1;

    memcpy(buf, tmpBuf, 7);
    ret = read(fd, buf+7, mAdtsHeader.aacFrameLength-7);
    if(ret < 0)
    {
        LOG_WARNING("read err\n");
        return -1;
    }

    return mAdtsHeader.aacFrameLength;
}