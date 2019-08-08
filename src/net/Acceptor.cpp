#include "net/Acceptor.h"
#include "net/SocketsOps.h"
#include "base/Logging.h"

Acceptor* Acceptor::createNew(UsageEnvironment* env, Ipv4Address& addr)
{
    return new Acceptor(env, addr);
}

Acceptor::Acceptor(UsageEnvironment* env, Ipv4Address& addr) :
    mEnv(env),
    mAddr(addr),
    mSocket(sockets::createTcpSock()),
    mNewConnectionCallback(NULL)
{
    mSocket.setReuseAddr(1);
    mSocket.bind(mAddr);
    mAcceptIOEvent = IOEvent::createNew(mSocket.fd(), this);
    mAcceptIOEvent->setReadCallback(readCallback);
    mAcceptIOEvent->enableReadHandling();
}

Acceptor::~Acceptor()
{
    if(mListenning)
        mEnv->scheduler()->removeIOEvent(mAcceptIOEvent);

    delete mAcceptIOEvent;
}

void Acceptor::listen()
{
    mListenning = true;
    mSocket.listen(1024);
    mEnv->scheduler()->addIOEvent(mAcceptIOEvent);
}

void Acceptor::setNewConnectionCallback(NewConnectionCallback cb, void* arg)
{
    mNewConnectionCallback = cb;
    mArg = arg;
}

void Acceptor::readCallback(void* arg)
{
    Acceptor* acceptor = (Acceptor*)arg;
    acceptor->handleRead();
}

void Acceptor::handleRead()
{
    int connfd = mSocket.accept();
    LOG_DEBUG("client connect: %d\n", connfd);
    if(mNewConnectionCallback)
        mNewConnectionCallback(mArg, connfd);
}
