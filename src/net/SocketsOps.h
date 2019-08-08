#ifndef _SOCKETSOPS_H_
#define _SOCKETSOPS_H_
#include <string>
#include <arpa/inet.h>
#include <sys/uio.h>

namespace sockets
{
int createTcpSock();
int createUdpSock();
bool bind(int sockfd, std::string ip, uint16_t port);
bool listen(int sockfd, int backlog);
int accept(int sockfd);
int readv(int sockfd, const struct iovec *iov, int iovcnt);
int write(int sockfd, const void* buf, int size);
int sendto(int sockfd, const void* buf, int len, const struct sockaddr *destAddr);
void setNonBlock(int sockfd);
void setBlock(int sockfd, int writeTimeout);
void setReuseAddr(int sockfd, int on);
void setReusePort(int sockfd);
void setNonBlockAndCloseOnExec(int sockfd);
void ignoreSigPipeOnSocket(int socketfd);
void setNoDelay(int sockfd);
void setKeepAlive(int sockfd);
void setNoSigpipe(int sockfd);
void setSendBufSize(int sockfd, int size);
void setRecvBufSize(int sockfd, int size);
std::string getPeerIp(int sockfd);
int16_t getPeerPort(int sockfd);
int getPeerAddr(int sockfd, struct sockaddr_in *addr);
void close(int sockfd);
bool connect(int sockfd, std::string ip, uint16_t port, int timeout);
std::string getLocalIp();
}

#endif //_SOCKETSOPS_H_