#include "NEIPAddress.h"
#include <event2/util.h>
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
			rc = evutil_inet_pton(AF_INET, _strIPAddress.data(), &addr.v4);
		}
		else if (_type == Type::IPv6) {
			rc = evutil_inet_pton(AF_INET6, _strIPAddress.data(), addr.v6);
		}
		if (1 != rc) {
			Logger(LM_ERROR) << "evutil_inet_pton error:" << rc;
		}
	}
	addr.port = _port;
	return addr;
}

void neapu::IPAddress::ToSockaddr(void* _sin) const
{
	if (this->IsIPv4()) {
		auto sin = static_cast<sockaddr_in*>(_sin);
		sin->sin_family = AF_INET;
		memcpy(&sin->sin_addr.s_addr, &v4, sizeof(v4));
		sin->sin_port = htons(port);
	}
	else if (this->IsIPv6()) {
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
		char buf[V4_BUF] = { 0 };
		evutil_inet_ntop(AF_INET, &v4, buf, V4_BUF);
		rst.append(buf, strlen(buf));
	} else if (this->IsIPv6()) {
		char buf[V6_BUF] = { 0 };
		evutil_inet_ntop(AF_INET6, v6, buf, V6_BUF);
		rst.append(buf, strlen(buf));
	}
	return rst;
}

NEAPU_NETWORK_EXPORT Logger& neapu::operator<<(Logger& _logger, const IPAddress& _addr)
{
	return _logger << "[" << _addr.ToString() << "][" << _addr.Port() << "]";
}
