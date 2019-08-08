#include <unistd.h>

#include "net/TcpSocket.h"
#include "net/SocketsOps.h"

TcpSocket::~TcpSocket()
{
    sockets::close(mSockfd);
}

bool TcpSocket::bind(Ipv4Address& addr)
{
    return sockets::bind(mSockfd, addr.getIp(), addr.getPort());
}

bool TcpSocket::listen(int backlog)
{
    return sockets::listen(mSockfd, backlog);
}

int TcpSocket::accept()
{
    return sockets::accept(mSockfd);
}

void TcpSocket::setReuseAddr(int on)
{
    sockets::setReuseAddr(mSockfd, on);
}
