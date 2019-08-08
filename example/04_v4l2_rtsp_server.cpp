#include <iostream>

#include "base/Logging.h"
#include "net/UsageEnvironment.h"
#include "base/ThreadPool.h"
#include "net/EventScheduler.h"
#include "net/Event.h"
#include "net/RtspServer.h"
#include "net/MediaSession.h"
#include "net/InetAddress.h"
#include "net/AACFileMediaSource.h"
#include "net/AACRtpSink.h"
#include "extend/v4l2/V4l2MediaSource.h"
#include "net/H264RtpSink.h"

int main(int argc, char* argv[])
{
    //Logger::setLogFile();
    Logger::setLogLevel(Logger::LogWarning);

    EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT); //创建调度器
    ThreadPool* threadPool = ThreadPool::createNew(2); //创建线程池
    UsageEnvironment* env = UsageEnvironment::createNew(scheduler, threadPool); //创建环境变量

    Ipv4Address ipAddr("0.0.0.0", 8554);
    RtspServer* server = RtspServer::createNew(env, ipAddr);
    MediaSession* session = MediaSession::createNew("live");
    MediaSource* videoSource = V4l2MediaSource::createNew(env, "/dev/video0");
    RtpSink* videoRtpSink = H264RtpSink::createNew(env, videoSource);

    session->addRtpSink(MediaSession::TrackId0, videoRtpSink);
    //session->startMulticast();

    server->addMeidaSession(session);
    server->start();

    std::cout<<"Play the media using the URL \""<<server->getUrl(session)<<"\""<<std::endl;

    env->scheduler()->loop();

    return 0;
}