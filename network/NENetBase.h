/************************************************************
* NetBase
* 对Libevent进行初步封装，并实现线程池。
* 由于Windows下使用IOCP，Libevent内部会实现线程池调度，
* 所以Windows下不提供线程池
************************************************************/
#pragma once
#include <stdint.h>
#include <NEThreadPoll.h>
#include <NEString.h>
#include <NESafeQueue.h>
#include <functional>
#include <map>
#include "NENetworkError.h"
#include "network_pub.h"

#ifndef evutil_socket_t
#ifdef WIN32
#define evutil_socket_t intptr_t
#else
#define evutil_socket_t int
#endif // WIN32
#endif // !evutil_socket_t
#ifndef EV_READ
#define EV_READ 0x02
#endif
#ifndef EV_WRITE
#define EV_WRITE 0x04
#endif

struct event_base;
struct event;
namespace neapu {
	class NEAPU_NETWORK_EXPORT NetBase {
		friend void cbSocketEvent(evutil_socket_t fd, short events, void* user_data);
		friend void cbSignalEvent(evutil_socket_t fd, short events, void* user_data);
	public:
		NetBase() {}
		NetBase(const NetBase&) = delete;
		NetBase(NetBase&&) noexcept;
		virtual ~NetBase() noexcept;
		int Run();
		void Stop();
		NetworkError GetError() { return m_err; }
	protected:
		int InitEvent(int _threadNum);
		int AddEvent(int _fd, short _ev);
		int AddSignal(int _signal);
		void RemoveSocket(int _fd);
		void RemoveSignal(int _signal);
		void SetLastError(int _err, String _errstr);
#ifndef _WIN32
		virtual void WorkThread();
#endif
		virtual void OnReadReady(int _fd) = 0;
		virtual void OnWriteReady(int _fd) = 0;
		virtual void OnSignalReady(int _signal) = 0;
		virtual void Stoped() {}
	protected:
		event_base* m_eb = nullptr;
		int m_threadNum = 0;
		std::map<int, event*> m_socketEventList;
		std::map<int, event*> m_signalEventList;
		NetworkError m_err;
#ifndef _WIN32
		ThreadPoll<std::function<void()>> m_threadPoll;
		SafeQueue<evutil_socket_t> m_readQueue;
		SafeQueue<evutil_socket_t> m_writeQueue;
		int m_running = 0;
#endif
	};
	void cbSocketEvent(evutil_socket_t fd, short events, void* user_data);
	void cbSignalEvent(evutil_socket_t fd, short events, void* user_data);
}