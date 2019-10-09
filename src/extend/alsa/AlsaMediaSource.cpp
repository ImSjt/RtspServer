#include <assert.h>

#include "extend/alsa/AlsaMediaSource.h"
#include "base/Logging.h"
#include "base/New.h"

AlsaMediaSource* AlsaMediaSource::createNew(UsageEnvironment* env, std::string dev)
{
    //return new AlsaMediaSource(env, dev);
    return New<AlsaMediaSource>::allocate(env, dev);
}

AlsaMediaSource::AlsaMediaSource(UsageEnvironment* env, const std::string& dev) :
    MediaSource(env),
    mEnv(env),
    mDev(dev),
    mPcmChannels(2),
    mSampleRate(44100),
    mFrames(1024)
{
    bool ret;

    ret = faacInit();
    assert(ret);

    ret = alsaInit();
    assert(ret);

    setFps(mSampleRate/mFrames); //修改帧率

    for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
        mEnv->threadPool()->addTask(mTask);
}

AlsaMediaSource::~AlsaMediaSource()
{
    alsaExit();
    faacExit();
}

void AlsaMediaSource::readFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);

    if(mAVFrameInputQueue.empty())
        return;

    AVFrame* frame = mAVFrameInputQueue.front();

    int ret = 0;
    while(ret <= 0)
    {
        ret = snd_pcm_readi(mPcmHandle, mPcmBuffer, mFrames);
        if (ret == -EPIPE)
        {
            /* EPIPE means overrun */
            LOG_WARNING("overrun occurred\n");
            snd_pcm_prepare(mPcmHandle);
        }
        else if (ret < 0)
        {
            LOG_ERROR("error from read: %s\n", snd_strerror(ret));
        }
        else if (ret != (int)mFrames)
        {
            LOG_WARNING("short read, read %d frames\n", ret);
        }

        ret = faacEncEncode(mFaacEncHandle, (int*)mPcmBuffer, mFrames*mPcmChannels,
                                mAACBuffer, mMaxOutputBytes);
    }

    memcpy(frame->mBuffer, mAACBuffer, ret);
    frame->mFrame = frame->mBuffer;
    frame->mFrameSize = ret;

    mAVFrameInputQueue.pop();
    mAVFrameOutputQueue.push(frame);
}

bool AlsaMediaSource::alsaInit()
{
    int ret;
    uint32_t val;
    int dir;
    snd_pcm_uframes_t frames;

	/* 打开设备的捕获功能 */
	ret = snd_pcm_open(&mPcmHandle, mDev.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if(ret < 0)
    {
        LOG_ERROR("failed to open snd dev\n");
        return false;
    }

    /* 分配参数设置 */
    snd_pcm_hw_params_alloca(&mPcmParams);
    
    /* 获取默认的参数设置 */
    snd_pcm_hw_params_any(mPcmHandle, mPcmParams);

    /* 设置采样为交互模式 */
    snd_pcm_hw_params_set_access(mPcmHandle, mPcmParams, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* 设置采样为：有符号，16为，小端 */
	snd_pcm_hw_params_set_format(mPcmHandle, mPcmParams, SND_PCM_FORMAT_S16_LE);

    /* 设置通道数 */
    snd_pcm_hw_params_set_channels(mPcmHandle, mPcmParams, mPcmChannels);

    /* 设置采样频率 */
    val = mSampleRate;
    snd_pcm_hw_params_set_rate_near(mPcmHandle, mPcmParams, &val, &dir);

	frames = mFrames;
	snd_pcm_hw_params_set_period_size_near(mPcmHandle, mPcmParams, &frames, &dir);

	ret = snd_pcm_hw_params(mPcmHandle, mPcmParams);
	if (ret < 0)
    {
		LOG_ERROR("unable to set hw parameters: %s\n", snd_strerror(ret));
		return false;
	}

	snd_pcm_hw_params_get_period_size(mPcmParams, &frames, &dir);
    
    //mFrames = frames;
    mFrames = mFrames;
    mFrameSize = mFrames*4;

    mPcmBuffer = new uint8_t[mFrameSize];

	snd_pcm_hw_params_get_period_time(mPcmParams, &val, &dir);

    return true;
}

void AlsaMediaSource::alsaExit()
{
	snd_pcm_drain(mPcmHandle);
	snd_pcm_close(mPcmHandle);
    delete[] mPcmBuffer;
}

bool AlsaMediaSource::faacInit()
{
	long unsigned int nInputSamples   = 0;
	long unsigned int nMaxOutputBytes = 0;

    mFaacEncHandle = faacEncOpen(mSampleRate, mPcmChannels, &nInputSamples, &nMaxOutputBytes);
    mFrames = nInputSamples/mPcmChannels;

    mMaxOutputBytes = nMaxOutputBytes;

    mAACBuffer = new uint8_t[nMaxOutputBytes];

    faacEncConfigurationPtr configuration;
    configuration = faacEncGetCurrentConfiguration(mFaacEncHandle);

    configuration->inputFormat = FAAC_INPUT_16BIT;
    configuration->aacObjectType = LOW;
    configuration->bitRate = mSampleRate;
    configuration->bandWidth = 64000;

    configuration->allowMidside = 1;
	configuration->useLfe = 0;
	configuration->useTns = 0;

    configuration->quantqual = 100;
	configuration->outputFormat = 1; //加上adts
	configuration->shortctl = SHORTCTL_NORMAL;  
    
    faacEncSetConfiguration(mFaacEncHandle, configuration);

    return true;
}

void AlsaMediaSource::faacExit()
{
    faacEncClose(mFaacEncHandle);
    delete[] mAACBuffer;
}
