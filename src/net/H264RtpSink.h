#ifndef _H264_MEDIA_SINK_H_
#define _H264_MEDIA_SINK_H_
#include <stdint.h>

#include "net/RtpSink.h"

class H264RtpSink : public RtpSink
{
public:
    static H264RtpSink* createNew(UsageEnvironment* env, MediaSource* mediaSource);
    
    H264RtpSink(UsageEnvironment* env, MediaSource* mediaSource);
    virtual ~H264RtpSink();

    virtual std::string getMediaDescription(uint16_t port);
    virtual std::string getAttribute();
    virtual void handleFrame(AVFrame* frame);

private:
    RtpPacket mRtpPacket;
    int mClockRate;
    int mFps;

};

#endif //_H264_MEDIA_SINK_H_