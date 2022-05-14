#pragma once
#include <utility>
#include <NEString.h>
#include "network_pub.h"

struct sockaddr_in;
struct sockaddr_in6;
namespace neapu {
	using IPAddress = struct NEAPU_NETWORK_EXPORT tagIPAddress{
		enum class Type
		{
			IPv4,
			IPv6
		};
		Type type;
		union
		{
			unsigned int v4;
			unsigned char v6[16];
		};
		int port;
		bool IsIPv4() const { return type == Type::IPv4; }
		bool IsIPv6() const { return type == Type::IPv6; }
		static tagIPAddress MakeAddress(const sockaddr_in& sin);
		static tagIPAddress MakeAddress(const sockaddr_in6& sin);
		void ToSockaddr(void* sin) const;
		String ToString() const;
	};
}