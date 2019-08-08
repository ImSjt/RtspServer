#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net/RtspConnection.h"
#include "net/MediaSession.h"
#include "base/Logging.h"

static void getPeerIp(int sockfd, std::string& ip)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    getpeername(sockfd, (struct sockaddr*)&addr, &addrlen);
    ip = inet_ntoa(addr.sin_addr);
}

RtspConnection* RtspConnection::createNew(RtspServer* rtspServer, int sockfd)
{
    return new RtspConnection(rtspServer, sockfd);
}

RtspConnection::RtspConnection(RtspServer* rtspServer, int sockfd) :
    TcpConnection(rtspServer->envir(), sockfd),
    mRtspServer(rtspServer),
    mMethod(NONE),
    mTrackId(MediaSession::TrackIdNone),
    mSessionId(rand()),
    mIsRtpOverTcp(false)
{
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        mRtpInstances[i] = NULL;
        mRtcpInstances[i] = NULL;
    }

    getPeerIp(sockfd, mPeerIp);
}

RtspConnection::~RtspConnection()
{
    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        if(mRtpInstances[i])
        {
            if(mSession)
                mSession->removeRtpInstance(mRtpInstances[i]);
            delete mRtpInstances[i];
        }

        if(mRtcpInstances[i])
        {
            delete mRtcpInstances[i];
        }
    }
}

void RtspConnection::handleReadBytes()
{
    bool ret;

    if(mIsRtpOverTcp)
    {
        if(mInputBuffer.peek()[0] == '$')
        {
            handleRtpOverTcp();
            return;
        }
    }

    ret = parseRequest();
    if(ret != true)
    {
        LOG_WARNING("failed to parse request\n");
        goto err;
    }

    switch (mMethod)
    {
    case OPTIONS:
        if(handleCmdOption() != true)
            goto err;
        break;
    case DESCRIBE:
        if(handleCmdDescribe() != true)
            goto err;
        break;
    case SETUP:
        if(handleCmdSetup() != true)
            goto err;
        break;
    case PLAY:
        if(handleCmdPlay() != true)
            goto err;
        break;
    case TEARDOWN:
        if(handleCmdTeardown() != true)
            goto err;
        break;
    case GET_PARAMETER:
        if(handleCmdGetParamter() != true)
            goto err;
        break;
    default:
        goto err;
        break;
    }

    return;
err:
    handleDisconnection();
}

bool RtspConnection::parseRequest()
{
    bool ret;

    /* 解析第一行 */
    const char* crlf = mInputBuffer.findCRLF();
    if(crlf == NULL)
    {
        mInputBuffer.retrieveAll();
        return false;
    }
    ret = parseRequest1(mInputBuffer.peek(), crlf);
    if(ret == false)
    {
        mInputBuffer.retrieveAll();
        return false;
    }
    mInputBuffer.retrieveUntil(crlf+2);

    /* 解析剩下的内容 */
    crlf = mInputBuffer.findLastCrlf();
    if(crlf == NULL)
    {
        mInputBuffer.retrieveAll();
        return false;
    }
    ret = parseRequest2(mInputBuffer.peek(), crlf);
    if(ret == false)
    {
        mInputBuffer.retrieveAll();
        return false;
    }
    mInputBuffer.retrieveUntil(crlf + 2);

    return true;
}

bool RtspConnection::parseRequest1(const char* begin, const char* end)
{
    std::string message(begin, end);
    char method[64] = {0};
    char url[512] = {0};
    char version[64] = {0};

    if(sscanf(message.c_str(), "%s %s %s", method, url, version) != 3)
    {
        return false; 
    }

    if(!strcmp(method, "OPTIONS"))
        mMethod = OPTIONS;
    else if(!strcmp(method, "DESCRIBE"))
        mMethod = DESCRIBE;
    else if(!strcmp(method, "SETUP"))
        mMethod = SETUP;
    else if(!strcmp(method, "PLAY"))
        mMethod = PLAY;
    else if(!strcmp(method, "TEARDOWN"))
        mMethod = TEARDOWN;
    else if(!strcmp(method, "GET_PARAMETER"))
        mMethod = GET_PARAMETER;
    else
    {
        mMethod = NONE;
        return false;
    }
    
    if(strncmp(url, "rtsp://", 7) != 0)
    {
        return false;
    }

    uint16_t port = 0;
    char ip[64] = {0};
    char suffix[64] = {0};
    
    if(sscanf(url+7, "%[^:]:%hu/%s", ip, &port, suffix) == 3)
    {

    }
    else if(sscanf(url+7, "%[^/]/%s", ip, suffix) == 2)
    {
        port = 554;
    }
    else
    {
        return false;
    }

    mUrl = url;
    mSuffix = suffix;

    return true;
}

bool RtspConnection::parseCSeq(std::string& message)
{
    std::size_t pos = message.find("CSeq");
    if (pos != std::string::npos)
    {
        uint32_t cseq = 0;
        sscanf(message.c_str()+pos, "%*[^:]: %u", &cseq);
        mCSeq = cseq;
        return true;
    }

    return false;
}

bool RtspConnection::parseAccept(std::string& message)
{
    if ((message.rfind("Accept")==std::string::npos)
        || (message.rfind("sdp")==std::string::npos))
    {
        return false;
    }

    return true;
}

bool RtspConnection::parseTransport(std::string& message)
{
    std::size_t pos = message.find("Transport");
    if(pos != std::string::npos)
    {
        if((pos=message.find("RTP/AVP/TCP")) != std::string::npos)
        {
            uint8_t rtpChannel, rtcpChannel;
            mIsRtpOverTcp = true;

            if(sscanf(message.c_str()+pos, "%*[^;];%*[^;];%*[^=]=%hhu-%hhu",
                        &rtpChannel, &rtcpChannel) != 2)
            {
                return false;
            }

            mRtpChannel = rtpChannel;

            return true;
        }
        else if((pos=message.find("RTP/AVP")) != std::string::npos)
        {
            uint16_t rtpPort = 0, rtcpPort = 0;
            if(((message.find("unicast", pos)) != std::string::npos))
            {
                if(sscanf(message.c_str()+pos, "%*[^;];%*[^;];%*[^=]=%hu-%hu",
                     &rtpPort, &rtcpPort) != 2)
                {
                    return false;
                }
            }
            else if((message.find("multicast", pos)) != std::string::npos)
            {
                return true;
            }
            else
                return false;

            mPeerRtpPort = rtpPort;
            mPeerRtcpPort = rtcpPort;
        }
        else
        {
            return false;
        }

        return true;
    }

    return false;
}

bool RtspConnection::parseMediaTrack()
{
    std::size_t pos = mUrl.find("track0");
    if(pos != std::string::npos)
    {
        mTrackId = MediaSession::TrackId0;
        return true;
    }

    pos = mUrl.find("track1");
    if(pos != std::string::npos)
    {
        mTrackId = MediaSession::TrackId1;
        return true;
    }

    return false;
}

bool RtspConnection::parseSessionId(std::string& message)
{
    std::size_t pos = message.find("Session");
    if (pos != std::string::npos)
    {
        uint32_t sessionId = 0;
        if(sscanf(message.c_str()+pos, "%*[^:]: %u", &sessionId) != 1)
            return false;
        return true;
    }

    return false;
}

bool RtspConnection::parseRequest2(const char* begin, const char* end)
{
    std::string message(begin, end);

    if(parseCSeq(message) != true)
        return false;
    
    if(mMethod == OPTIONS)
        return true;
    
    if(mMethod == DESCRIBE)
        return parseAccept(message);

    if(mMethod == SETUP)
    {
        if(parseTransport(message) != true)
            return false;

        return parseMediaTrack();
    }

    if(mMethod == PLAY)
        return parseSessionId(message);

    if(mMethod == TEARDOWN)
        return true;
    
    if(mMethod == GET_PARAMETER)
        return true;

    return false;
}

bool RtspConnection::handleCmdOption()
{
    snprintf(mBuffer, sizeof(mBuffer),
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %u\r\n"
            "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n"
            "\r\n", mCSeq);

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;

    return true;
}

bool RtspConnection::handleCmdDescribe()
{
    MediaSession* session = mRtspServer->loopupMediaSession(mSuffix);
    if(!session)
    {
        LOG_DEBUG("can't loop up %s session\n", mSuffix.c_str());
        return false;
    }

    mSession = session;
    std::string sdp = session->generateSDPDescription();

    memset((void*)mBuffer, 0, sizeof(mBuffer));
    snprintf((char*)mBuffer, sizeof(mBuffer),
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %u\r\n"
            "Content-Length: %u\r\n"
            "Content-Type: application/sdp\r\n"
            "\r\n"
            "%s",
            mCSeq,
            sdp.size(),
            sdp.c_str());

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
            return false;

    return true;
}

bool RtspConnection::handleCmdSetup()
{
    char sessionName[100];
    if(sscanf(mSuffix.c_str(), "%[^/]/", sessionName) != 1)
    {
        return false;
    }

    MediaSession* session = mRtspServer->loopupMediaSession(sessionName);
    if(!session)
    {
        LOG_DEBUG("can't loop up %s session\n", sessionName);
        return false;
    }

    if(mTrackId >= MEDIA_MAX_TRACK_NUM || mRtpInstances[mTrackId] || mRtcpInstances[mTrackId])
        return false;

    if(session->isStartMulticast())
    {
        snprintf((char*)mBuffer, sizeof(mBuffer),
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Transport: RTP/AVP;multicast;"
                    "destination=%s;source=%s;port=%d-%d;ttl=255\r\n"
                    "Session: %08x\r\n"
                    "\r\n",
                    mCSeq,
                    session->getMulticastDestAddr().c_str(),
                    sockets::getLocalIp().c_str(),
                    session->getMulticastDestRtpPort(mTrackId),
                    session->getMulticastDestRtpPort(mTrackId)+1,
                    mSessionId);
    }
    else
    {
        if(mIsRtpOverTcp) //rtp over tcp
        {
            /* 创建rtp over tcp */
            createRtpOverTcp(mTrackId, mSocket.fd(), mRtpChannel);
            mRtpInstances[mTrackId]->setSessionId(mSessionId);
            session->addRtpInstance(mTrackId, mRtpInstances[mTrackId]);

            snprintf((char*)mBuffer, sizeof(mBuffer),
                        "RTSP/1.0 200 OK\r\n"
                        "CSeq: %d\r\n"
                        "Transport: RTP/AVP/TCP;unicast;interleaved=%hhu-%hhu\r\n"
                        "Session: %08x\r\n"
                        "\r\n",
                        mCSeq,
                        mRtpChannel,
                        mRtpChannel+1,
                        mSessionId);
        }
        else //rtp over udp
        {
            if(createRtpRtcpOverUdp(mTrackId, mPeerIp, mPeerRtpPort, mPeerRtcpPort) != true)
            {
                LOG_WARNING("failed to create rtp and rtcp\n");
                return false;
            }

            mRtpInstances[mTrackId]->setSessionId(mSessionId);
            mRtcpInstances[mTrackId]->setSessionId(mSessionId);

            /* 添加到会话中 */
            session->addRtpInstance(mTrackId, mRtpInstances[mTrackId]);

            snprintf((char*)mBuffer, sizeof(mBuffer),
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %u\r\n"
                    "Transport: RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu\r\n"
                    "Session: %08x\r\n"
                    "\r\n",
                    mCSeq,
                    mPeerRtpPort,
                    mPeerRtcpPort,
                    mRtpInstances[mTrackId]->getLocalPort(), 
                    mRtcpInstances[mTrackId]->getLocalPort(),
                    mSessionId);
        }
        
    }

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;

    return true;
}

bool RtspConnection::handleCmdPlay()
{
    snprintf((char*)mBuffer, sizeof(mBuffer),
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %d\r\n"
            "Range: npt=0.000-\r\n"
            "Session: %08x; timeout=60\r\n"
            "\r\n",
            mCSeq,
            mSessionId);

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
        return false;

    for(int i = 0; i < MEDIA_MAX_TRACK_NUM; ++i)
    {
        if(mRtpInstances[i])
            mRtpInstances[i]->setAlive(true);
        
        if(mRtcpInstances[i])
            mRtcpInstances[i]->setAlive(true);
    }

    return true;
}

bool RtspConnection::handleCmdTeardown()
{
    snprintf((char*)mBuffer, sizeof(mBuffer),
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %d\r\n"
            "\r\n",
            mCSeq);

    if(sendMessage(mBuffer, strlen(mBuffer)) < 0)
    {
        return false;
    }
    
    return true;
}

bool RtspConnection::handleCmdGetParamter()
{
    
}

int RtspConnection::sendMessage(void* buf, int size)
{
    int ret;

    mOutBuffer.append(buf, size);
    ret = mOutBuffer.write(mSocket.fd());
    mOutBuffer.retrieveAll();

    return ret;
}

int RtspConnection::sendMessage()
{
    int ret;

    ret = mOutBuffer.write(mSocket.fd());
    mOutBuffer.retrieveAll();

    return ret;
}

bool RtspConnection::createRtpRtcpOverUdp(MediaSession::TrackId trackId, std::string peerIp,
                                    uint16_t peerRtpPort, uint16_t peerRtcpPort)
{
    int rtpSockfd, rtcpSockfd;
    int16_t rtpPort, rtcpPort;
    bool ret;

    if(mRtpInstances[trackId] || mRtcpInstances[trackId])
        return false;

    int i;
    for(i = 0; i < 10; ++i)
    {
        rtpSockfd = sockets::createUdpSock();
        if(rtpSockfd < 0)
        {
            return false;
        }

        rtcpSockfd = sockets::createUdpSock();
        if(rtcpSockfd < 0)
        {
            close(rtpSockfd);
            return false;
        }

        uint16_t port = rand() & 0xfffe;
        if(port < 10000)
            port += 10000;
        
        rtpPort = port;
        rtcpPort = port+1;

        ret = sockets::bind(rtpSockfd, "0.0.0.0", rtpPort);
        if(ret != true)
        {
            sockets::close(rtpSockfd);
            sockets::close(rtcpSockfd);
            continue;
        }

        ret = sockets::bind(rtcpSockfd, "0.0.0.0", rtcpPort);
        if(ret != true)
        {
            sockets::close(rtpSockfd);
            sockets::close(rtcpSockfd);
            continue;
        }

        break;
    }

    if(i == 10)
        return false;

    mRtpInstances[trackId] = RtpInstance::createNewOverUdp(rtpSockfd, rtpPort,
                                                        peerIp, peerRtpPort);
    mRtcpInstances[trackId] = RtcpInstance::createNew(rtcpSockfd, rtcpPort,
                                                        peerIp, peerRtcpPort);

    return true;
}

bool RtspConnection::createRtpOverTcp(MediaSession::TrackId trackId, int sockfd,
                                            uint8_t rtpChannel)
{
    mRtpInstances[trackId] = RtpInstance::createNewOverTcp(sockfd, rtpChannel);

    return true;
}

void RtspConnection::handleRtpOverTcp()
{
    uint8_t* buf = (uint8_t*)mInputBuffer.peek();
    uint16_t size;

    size = (buf[2]<<8) | buf[3];
    if(mInputBuffer.readableBytes() < size+4)
        return;

    mInputBuffer.retrieve(size+4);
}