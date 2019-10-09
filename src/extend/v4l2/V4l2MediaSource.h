#ifndef _V4L2_MEDIA_SOURCE_H_
#define _V4L2_MEDIA_SOURCE_H_
#include <string>
#include <queue>
#include <stdint.h>
#include <x264.h>

#include "net/MediaSource.h"
#include "extend/v4l2/V4l2.h"

class V4l2MediaSource : public MediaSource
{
public:
    static V4l2MediaSource* createNew(UsageEnvironment* env, std::string dev);
    
    V4l2MediaSource(UsageEnvironment* env, const std::string& dev);
    virtual ~V4l2MediaSource();

protected:
    virtual void readFrame();

private:
    struct Nalu
    {
        Nalu(uint8_t* data, int size) : mData(data), mSize(size)
        { }

        uint8_t* mData;
        int mSize;
    };

    bool videoInit();
    bool videoExit();
    bool x264Init();
    bool x264Exit();

private:
    UsageEnvironment* mEnv;
    std::string mDev;
    int mFd;
    int mWidth;
    int mHeight;
    struct v4l2_buf* mV4l2Buf;
    struct v4l2_buf_unit* mV4l2BufUnit;

	x264_nal_t* mNals;
	x264_t* mX264Handle;
	x264_picture_t* mPicIn;
	x264_picture_t* mPicOut;
	x264_param_t* mParam;
    int mCsp;
    int mPts;

    std::queue<Nalu> mNaluQueue;
};

#endif //_V4L2_MEDIA_SOURCE_H_