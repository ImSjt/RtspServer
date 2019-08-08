#ifndef _RTP_H_
#define _RTP_H_
#include <stdint.h>
#include <stdlib.h>

#define RTP_VESION              2

#define RTP_PAYLOAD_TYPE_H264   96
#define RTP_PAYLOAD_TYPE_AAC    97

#define RTP_HEADER_SIZE         12
#define RTP_MAX_PKT_SIZE        1400

struct RtpHeader
{
    /* byte 0 */
    uint8_t csrcLen:4;
    uint8_t extension:1;
    uint8_t padding:1;
    uint8_t version:2;

    /* byte 1 */
    uint8_t payloadType:7;
    uint8_t marker:1;
    
    /* bytes 2,3 */
    uint16_t seq;
    
    /* bytes 4-7 */
    uint32_t timestamp;
    
    /* bytes 8-11 */
    uint32_t ssrc;

    /* data */
    uint8_t payload[0];
};

class RtpPacket
{
public:
    RtpPacket() :
        _mBuffer(new uint8_t[RTP_MAX_PKT_SIZE+RTP_HEADER_SIZE+100]),
        mBuffer(_mBuffer+4),
        mRtpHeadr((RtpHeader*)mBuffer),
        mSize(0)
    {
        
    }

    ~RtpPacket()
    {
        delete _mBuffer;
    }

    uint8_t* _mBuffer;
    uint8_t* mBuffer;
    RtpHeader* const mRtpHeadr;
    int mSize;
};

#endif //_RTP_H_