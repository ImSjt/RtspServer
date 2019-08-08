#ifndef _AAC_RTP_SINK_H_
#define _AAC_RTP_SINK_H_

#include "net/UsageEnvironment.h"
#include "net/RtpSink.h"
#include "net/MediaSource.h"

class AACRtpSink : public RtpSink
{
public:
    static AACRtpSink* createNew(UsageEnvironment* env, MediaSource* mediaSource);;
    virtual ~AACRtpSink();

    virtual std::string getMediaDescription(uint16_t port);
    virtual std::string getAttribute();

protected:
    virtual void handleFrame(AVFrame* frame);

private:
    AACRtpSink(UsageEnvironment* env, MediaSource* mediaSource, int payloadType);

private:
    RtpPacket mRtpPacket;
    uint32_t mSampleRate;   // 采样频率
    uint32_t mChannels;         // 通道数
    int mFps;
};

#endif //_AAC_RTP_SINK_H_