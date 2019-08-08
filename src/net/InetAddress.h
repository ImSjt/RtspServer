#ifndef _INET_ADDRESS_H_
#define _INET_ADDRESS_H_
#include <string>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Ipv4Address
{
public:
    Ipv4Address();
    Ipv4Address(std::string ip, uint16_t port);
    void setAddr(std::string ip, uint16_t port);
    std::string getIp();
    uint16_t getPort();
    struct sockaddr* getAddr();

private:
    std::string mIp;
    uint16_t mPort;
    struct sockaddr_in mAddr;
};

#endif //_INET_ADDRESS_H_