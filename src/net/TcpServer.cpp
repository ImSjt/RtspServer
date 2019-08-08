#include <assert.h>

#include "net/TcpServer.h"
#include "base/Logging.h"

TcpServer::TcpServer(UsageEnvironment* env, Ipv4Address& addr) :
    mEnv(env),
    mAddr(addr)
{
    mAcceptor = Acceptor::createNew(env, addr);
    assert(mAcceptor);
    mAcceptor->setNewConnectionCallback(newConnectionCallback, this);
}

TcpServer::~TcpServer()
{
    delete mAcceptor;
}

void TcpServer::start()
{
    mAcceptor->listen();
}

void TcpServer::newConnectionCallback(void* arg, int connfd)
{
    TcpServer* tcpServer = (TcpServer*)arg;
    tcpServer->handleNewConnection(connfd);
}

#if 0
void TcpServer::handleNewConnection(int connfd)
{
    TcpConnection* tcpConn = TcpConnection::createNew(mEnv, connfd);
    tcpConn->setDisconnectionCallback(disconnectionCallback, this);
    mTcpConnections.insert(std::make_pair(connfd, tcpConn));
}

void TcpServer::disconnectionCallback(void* arg, int sockfd)
{
    TcpServer* tcpServer = (TcpServer*)arg;
    tcpServer->handleDisconnection(sockfd);
}

void TcpServer::handleDisconnection(int sockfd)
{
    std::map<int, TcpConnection*>::iterator it = mTcpConnections.find(sockfd);
    if(it == mTcpConnections.end())
    {
        LOG_DEBUG("can't find\n");
        return;
    }
    
    delete it->second; //释放内存，析构函数会删除IO事件，释放内存，socket生命结束关闭文件描述符
    mTcpConnections.erase(sockfd);
}
#endif