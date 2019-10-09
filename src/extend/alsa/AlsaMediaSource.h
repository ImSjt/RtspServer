#ifndef _ALSA_MEDIA_SOURCE_H_
#define _ALSA_MEDIA_SOURCE_H_
#include <string>
#include <queue>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <faac.h>

#include "net/MediaSource.h"

class AlsaMediaSource : public MediaSource
{
public:
    static AlsaMediaSource* createNew(UsageEnvironment* env, std::string dev);
    
    AlsaMediaSource(UsageEnvironment* env, const std::string& dev);
    virtual ~AlsaMediaSource();

protected:
    virtual void readFrame();

private:
    bool alsaInit();
    void alsaExit();
    bool faacInit();
    void faacExit();

private:
    UsageEnvironment* mEnv;
    std::string mDev;

	snd_pcm_t *mPcmHandle;
	snd_pcm_hw_params_t *mPcmParams;
    int mPcmChannels; //通道数
    int mSampleRate; //采样频率
    int mFrames; //一帧音频采样数
    int mFrameSize;
    faacEncHandle mFaacEncHandle;
    uint8_t* mPcmBuffer;
	uint8_t* mAACBuffer;
    uint32_t mMaxOutputBytes;
};

#endif //_ALSA_MEDIA_SOURCE_H_