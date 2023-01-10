/**************************************************************************************
 * 对sockaddr封装
 * 整合IPv4和v6
 * 封装从字符串转换
 **************************************************************************************/
#pragma once
#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <utility>
#include <base/NEString.h>
#include "network/network_pub.h"
#include <logger/logger.h>

struct sockaddr_in;
struct sockaddr_in6;
namespace neapu {
class NEAPU_NETWORK_EXPORT IPAddress {
public:
    enum class Type {
        IPv4,
        IPv6
    };

private:
    Type type = Type::IPv4;
    union {
        unsigned int v4;
        unsigned char v6[16] = {0};
    };
    int port = 0;

public:
    bool IsIPv4() const
    {
        return type == Type::IPv4;
    }
    bool IsIPv6() const
    {
        return type == Type::IPv6;
    }
    static IPAddress MakeAddress(const sockaddr_in& sin);
    static IPAddress MakeAddress(const sockaddr_in6& sin);
    static IPAddress MakeAddress(const sockaddr_storage& sin);
    static IPAddress MakeAddress(Type _type, String _strIPAddress, int _port);

    /*
    *  使用IP:Port格式
    *  例如:
    *  IPv4: 127.0.0.1:8888
    *  IPv6: [::1]:8888
    */
    static IPAddress MakeAddress(String _strAddr);

    
    void ToSockaddr(void* sin) const;
    String ToString() const;
    int Port() const
    {
        return port;
    }
    bool IsEmptyAddr() const;
    socklen_t SockaddrLen() const;
};

} // namespace neapu
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, const neapu::IPAddress& _addr);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::IPAddress& _addr);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::IPAddress&& _addr);