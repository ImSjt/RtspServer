#ifndef _SOCKET_H_
#define _SOCKET_H_
#include <string>
#include <stdint.h>

#include "net/InetAddress.h"

class  TcpSocket
{
public:
    explicit TcpSocket(int sockfd) :
        mSockfd(sockfd) { }

    ~TcpSocket();

    int fd() const { return mSockfd; }
    bool bind(Ipv4Address& addr);
    bool listen(int backlog);
    int accept();
    void setReuseAddr(int on);

private:
    int mSockfd;
};

#endif //_SOCKET_H_