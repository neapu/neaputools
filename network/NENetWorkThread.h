#pragma once
#include <thread>
#include <map>
#include <condition_variable>
#include <mutex>
#ifndef evutil_socket_t
#ifdef WIN32
#define evutil_socket_t intptr_t
#else
#define evutil_socket_t int
#endif // WIN32
#endif // !evutil_socket_t
struct event_base;
struct event;
namespace neapu {
class TcpBase;
class NetWorkThread {
	friend void cbRead(evutil_socket_t fd, short events, void* user_data);
public:
	NetWorkThread(TcpBase* _netBase);
	~NetWorkThread();
	int dispatch(int fd);
	void WaitInit();
	void Exit();
private:
	void Proc();
	void OnRead(int fd);
private:
	event_base* m_eb;
	std::thread m_thread;
	std::map<int, struct event*> m_eventList;
	TcpBase* m_netBase;
	evutil_socket_t m_fds[2];
	std::condition_variable m_cond;
	std::mutex m_condMutex;
};
}