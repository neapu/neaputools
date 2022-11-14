/**************************************************************************************
* 对sockaddr封装
* 整合IPv4和v6
* 封装从字符串转换
**************************************************************************************/
#pragma once
#include <utility>
#include <NEString.h>
#include "network_pub.h"
#include <NELogger.h>

struct sockaddr_in;
struct sockaddr_in6;
namespace neapu {
	class NEAPU_NETWORK_EXPORT IPAddress{
	public:
		enum class Type
		{
			IPv4,
			IPv6
		};
	private:
		Type type = Type::IPv4;
		union
		{
			unsigned int v4;
			unsigned char v6[16] = {0};
		};
		int port = 0;
	public:
		bool IsIPv4() const { return type == Type::IPv4; }
		bool IsIPv6() const { return type == Type::IPv6; }
		static IPAddress MakeAddress(const sockaddr_in& sin);
		static IPAddress MakeAddress(const sockaddr_in6& sin);
		static IPAddress MakeAddress(Type _type, String _strIPAddress, int _port);
		void ToSockaddr(void* sin) const;
		String ToString() const;
		int Port() const { return port; }
	};
	NEAPU_NETWORK_EXPORT Logger& operator<<(Logger& _logger, const IPAddress& _addr);
	NEAPU_NETWORK_EXPORT Logger& operator<<(Logger& _logger, IPAddress&& _addr);
}