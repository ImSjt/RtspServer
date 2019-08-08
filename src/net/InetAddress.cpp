#include "net/InetAddress.h"

Ipv4Address::Ipv4Address()
{

}

Ipv4Address::Ipv4Address(std::string ip, uint16_t port) :
    mIp(ip),
    mPort(port)
{
    mAddr.sin_family = AF_INET;		  
    mAddr.sin_addr.s_addr = inet_addr(ip.c_str()); 
    mAddr.sin_port = htons(port);
}

void Ipv4Address::setAddr(std::string ip, uint16_t port)
{
    mIp = ip;
    mPort = port;
    mAddr.sin_family = AF_INET;		  
    mAddr.sin_addr.s_addr = inet_addr(ip.c_str()); 
    mAddr.sin_port = htons(port);
}

std::string Ipv4Address::getIp()
{
    return mIp;
}

uint16_t Ipv4Address::getPort()
{
    return mPort;
}

struct sockaddr* Ipv4Address::getAddr()
{
    return (struct sockaddr*)&mAddr;
}
