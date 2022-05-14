#pragma once
#include <stdint.h>
#include <functional>
#include "NEIPAddress.h"
#include "NESocketError.h"
#include <NEThreadPoll.h>
#include <NEString.h>
#include <NESafeQueue.h>
#include "network_pub.h"

#ifndef evutil_socket_t
#ifdef WIN32
#define evutil_socket_t intptr_t
#else
#define evutil_socket_t int
#endif // WIN32
#endif // !evutil_socket_t

struct sockaddr_in;
struct event_base;
namespace neapu {
	class ByteArray;
	class NEAPU_NETWORK_EXPORT UdpBase {
		friend void cbSocketEvent(evutil_socket_t fd, short events, void* user_data);
		friend void cbSigInt(evutil_socket_t sig, short events, void* user_data);
	public:
		int Init(int _port, int _threads, IPAddress::Type _serverType, bool _enableWriteCallback = false);
		int Send(const ByteArray& _data, const IPAddress& _addr);
		int Run();
		int BindPort() { return m_port; }
		UdpBase& OnRecvData(std::function<void(const ByteArray&, const IPAddress&)> _cb);
		UdpBase& OnWritable(std::function<void()> _cb);

	protected:
		void SetLastError(int _err, String _errstr);
		virtual void OnRecvData(const ByteArray& _data, const IPAddress& _addr) {};
		virtual void OnWritable() {}
#ifndef _WIN32
		virtual void WorkThread();
#endif
		virtual void OnReadReady(int _fd);
		virtual void OnSignalInt();
	protected:
		using UdpBaseCallback = struct {
			std::function<void(const ByteArray&, const IPAddress&)> recvDataCallback;
			std::function<void()> writableCallback;
			uint64_t userData;
		};
		event_base* m_eb = nullptr;
		int m_udpFd = 0;
		UdpBaseCallback m_callback;
		IPAddress::Type m_ipType = IPAddress::Type::IPv4;
		int m_port = 0;
		int m_err = 0;
		String m_errstr;
		bool m_enableWriteCallback = false;
#ifndef _WIN32
		ThreadPoll<std::function<void()>> m_threadPoll;
		SafeQueue<int> m_readQueue;
		int m_running = 0;
#endif
	};
}