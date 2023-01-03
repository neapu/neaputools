#include "NEIPAddress.h"
#include <arpa/inet.h>
#ifdef USE_LIBEVENT
#include <event2/util.h>
#endif
#ifdef WIN32
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif
using namespace neapu;

constexpr auto V4_BUF = 16;
constexpr auto V6_BUF = 128;

neapu::IPAddress neapu::IPAddress::MakeAddress(const sockaddr_in& sin)
{
    neapu::IPAddress addr;
    addr.type = IPAddress::Type::IPv4;
    memcpy(&addr.v4, &sin.sin_addr.s_addr, sizeof(v4));
    addr.port = ntohs(sin.sin_port);
    return addr;
}

neapu::IPAddress neapu::IPAddress::MakeAddress(const sockaddr_in6& sin)
{
    neapu::IPAddress addr;
    addr.type = IPAddress::Type::IPv6;
    memcpy(addr.v6, &sin.sin6_addr, sizeof(v6));
    addr.port = ntohs(sin.sin6_port);
    return addr;
}

IPAddress neapu::IPAddress::MakeAddress(Type _type, String _strIPAddress, int _port)
{
    IPAddress addr;
    addr.type = _type;
    if (!_strIPAddress.IsEmpty()) {
        int rc = 0;
        if (_type == Type::IPv4) {
            rc = inet_pton(AF_INET, _strIPAddress.ToCString(), &addr.v4);
        } else if (_type == Type::IPv6) {
            rc = inet_pton(AF_INET6, _strIPAddress.ToCString(), addr.v6);
        }
        if (1 != rc) {
            Logger(LM_ERROR) << "evutil_inet_pton error:" << rc;
        }
    }
    addr.port = _port;
    return addr;
}

IPAddress IPAddress::MakeAddress(const sockaddr_storage& sin)
{
	if(sin.ss_family == AF_INET){
		return MakeAddress((const sockaddr_in&)sin);
	} else if(sin.ss_family == AF_INET6) {
		return MakeAddress((const sockaddr_in6&)sin);
	}
    return IPAddress();
}

void neapu::IPAddress::ToSockaddr(void* _sin) const
{
    if (this->IsIPv4()) {
        auto sin = static_cast<sockaddr_in*>(_sin);
        sin->sin_family = AF_INET;
        memcpy(&sin->sin_addr.s_addr, &v4, sizeof(v4));
        sin->sin_port = htons(port);
    } else if (this->IsIPv6()) {
        auto sin = static_cast<sockaddr_in6*>(_sin);
        sin->sin6_family = AF_INET6;
        sin->sin6_port = htons(port);
        memcpy(&sin->sin6_addr, v6, sizeof(v6));
    }
}

neapu::String neapu::IPAddress::ToString() const
{
    String rst;
    if (this->IsIPv4()) {
        char buf[V4_BUF] = {0};
        inet_ntop(AF_INET, &v4, buf, V4_BUF);
        rst.Append(buf, strlen(buf));
    } else if (this->IsIPv6()) {
        char buf[V6_BUF] = {0};
        inet_ntop(AF_INET6, v6, buf, V6_BUF);
        rst.Append(buf, strlen(buf));
    }
    return rst;
}

bool neapu::IPAddress::IsEmptyAddr() const
{
    if (IsIPv4()) {
        return ToString() == "0.0.0.0";
    } else {
        return ToString() == "::";
    }
}

socklen_t neapu::IPAddress::SockaddrLen() const
{
	if(IsIPv4()){
		return sizeof(struct sockaddr_in);
	}else{
		return sizeof(struct sockaddr_in6);
	}
}

NEAPU_NETWORK_EXPORT Logger& operator<<(Logger& _logger, const IPAddress& _addr)
{
    return _logger << "[" << _addr.ToString() << "][" << _addr.Port() << "]";
}

NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::IPAddress& _addr)
{
    return _logger << "[" << _addr.ToString() << "][" << _addr.Port() << "]";
}

NEAPU_NETWORK_EXPORT Logger& operator<<(Logger& _logger, IPAddress&& _addr)
{
    return _logger << "[" << _addr.ToString() << "][" << _addr.Port() << "]";
}