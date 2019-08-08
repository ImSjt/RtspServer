#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_
#include "net/UsageEnvironment.h" 
#include "net/Event.h"
#include "net/InetAddress.h"
#include "net/TcpSocket.h"

class Acceptor
{
public:
    typedef void(*NewConnectionCallback)(void* data, int connfd);

    static Acceptor* createNew(UsageEnvironment* env, Ipv4Address& addr);
    ~Acceptor();

    bool listenning() const { return mListenning; }
    void listen();
    void setNewConnectionCallback(NewConnectionCallback cb, void* arg);

private:
    Acceptor(UsageEnvironment* env, Ipv4Address& addr);
    static void readCallback(void*);
    void handleRead();

private:
    UsageEnvironment* mEnv;
    Ipv4Address mAddr;
    IOEvent* mAcceptIOEvent;
    TcpSocket mSocket;
    bool mListenning;
    NewConnectionCallback mNewConnectionCallback;
    void* mArg;
};

#endif //_ACCEPTOR_H_