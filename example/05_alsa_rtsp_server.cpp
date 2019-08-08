#include <iostream>

#include "base/Logging.h"
#include "net/UsageEnvironment.h"
#include "base/ThreadPool.h"
#include "net/EventScheduler.h"
#include "net/Event.h"
#include "net/RtspServer.h"
#include "net/MediaSession.h"
#include "net/InetAddress.h"
#include "extend/alsa/AlsaMediaSource.h"
#include "net/AACRtpSink.h"

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
    MediaSource* audioSource = AlsaMediaSource::createNew(env, "hw:0,0"); //默认设备
    RtpSink* audioRtpSink = AACRtpSink::createNew(env, audioSource);

    session->addRtpSink(MediaSession::TrackId0, audioRtpSink);
    //session->startMulticast();

    server->addMeidaSession(session);
    server->start();

    std::cout<<"Play the media using the URL \""<<server->getUrl(session)<<"\""<<std::endl;

    env->scheduler()->loop();

    return 0;
}