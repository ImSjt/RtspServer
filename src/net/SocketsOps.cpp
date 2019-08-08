#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>

#include "net/SocketsOps.h"

int sockets::createTcpSock()
{
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);

  return sockfd;
}

int sockets::createUdpSock()
{
  int sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);

  return sockfd;
}

bool sockets::bind(int sockfd, std::string ip, uint16_t port)
{
    struct sockaddr_in addr = {0};			  
    addr.sin_family = AF_INET;		  
    addr.sin_addr.s_addr = inet_addr(ip.c_str()); 
    addr.sin_port = htons(port);

    if(::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {      
        return false;
    }

    return true;
}

bool sockets::listen(int sockfd, int backlog)
{
    if(::listen(sockfd, backlog) < 0)
        return false;
    
    return true;
}

int sockets::accept(int sockfd)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);

    int connfd = ::accept(sockfd, (struct sockaddr*)&addr, &addrlen);
    setNonBlockAndCloseOnExec(connfd);
    ignoreSigPipeOnSocket(connfd);

    return connfd;
}

int sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}

int sockets::write(int sockfd, const void* buf, int size)
{
    return ::write(sockfd, buf, size);
}

int sockets::sendto(int sockfd, const void* buf, int len, 
                        const struct sockaddr *destAddr)
{
    socklen_t addrLen = sizeof(struct sockaddr);
    return ::sendto(sockfd, buf, len, 0, destAddr, addrLen);
}

void sockets::setNonBlock(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void sockets::setBlock(int sockfd, int writeTimeout)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags&(~O_NONBLOCK));

    if (writeTimeout > 0)
    {
        struct timeval tv = {writeTimeout/1000, (writeTimeout%1000)*1000};
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    }
}

void sockets::setReuseAddr(int sockfd, int on)
{
    int optval = on ? 1 : 0;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
}

void sockets::setReusePort(int sockfd)
{
#ifdef SO_REUSEPORT
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&on, sizeof(on));
#endif
}

void sockets::setNonBlockAndCloseOnExec(int sockfd)
{
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
}

void sockets::ignoreSigPipeOnSocket(int socketfd)
{
    int option = 1;
    setsockopt(socketfd, SOL_SOCKET, MSG_NOSIGNAL, &option, sizeof(option));
}

void sockets::setNoDelay(int sockfd)
{
#ifdef TCP_NODELAY
    int on = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
#endif
}

void sockets::setKeepAlive(int sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
}

void sockets::setNoSigpipe(int sockfd)
{
#ifdef SO_NOSIGPIPE
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&on, sizeof(on));
#endif
}

void sockets::setSendBufSize(int sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
}

void sockets::setRecvBufSize(int sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));
}

std::string sockets::getPeerIp(int sockfd)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return inet_ntoa(addr.sin_addr);
    }

    return "0.0.0.0";
}

int16_t sockets::getPeerPort(int sockfd)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return ntohs(addr.sin_port);
    }

    return 0;
}

int sockets::getPeerAddr(int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    return getpeername(sockfd, (struct sockaddr *)addr, &addrlen);
}

void sockets::close(int sockfd)
{
    int ret = ::close(sockfd);
}

bool sockets::connect(int sockfd, std::string ip, uint16_t port, int timeout)
{
	bool isConnected = true;
	if (timeout > 0)
	{
		sockets::setNonBlock(sockfd);
	}

	struct sockaddr_in addr = { 0 };
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	if (::connect(sockfd, (struct sockaddr*)&addr, addrlen) < 0)
	{
		if (timeout > 0)
		{
			isConnected = false;
			fd_set fdWrite;
			FD_ZERO(&fdWrite);
			FD_SET(sockfd, &fdWrite);
			struct timeval tv = { timeout / 1000, timeout % 1000 * 1000 };
			select(sockfd + 1, NULL, &fdWrite, NULL, &tv);
			if (FD_ISSET(sockfd, &fdWrite))
			{
				isConnected = true;
			}
			sockets::setBlock(sockfd, 0);
		}
		else
		{
			isConnected = false;
		}		
	}
	
	return isConnected;
}

std::string sockets::getLocalIp()
{
    int sockfd = 0;
    char buf[512] = { 0 };
    struct ifconf ifconf;
    struct ifreq  *ifreq;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        close(sockfd);
        return "0.0.0.0";
    }

    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifconf) < 0)
    {
        close(sockfd);
        return "0.0.0.0";
    }

    close(sockfd);

    ifreq = (struct ifreq*)ifconf.ifc_buf;
    for (int i = (ifconf.ifc_len / sizeof(struct ifreq)); i>0; i--)
    {
        if (ifreq->ifr_flags == AF_INET)
        {
            if (strcmp(ifreq->ifr_name, "lo") != 0)
            {
                return inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
            }
            ifreq++;
        }
    }
    return "0.0.0.0";
}