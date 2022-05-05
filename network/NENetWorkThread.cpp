#include "NENetWorkThread.h"
#include <event2/event.h>
#include <functional>
#include "NENetBase.h"

using namespace neapu;

neapu::NetWorkThread::NetWorkThread(NetBase* _netBase)
	: m_eb(nullptr)
	, m_netBase(_netBase)
{
	evutil_socketpair(AF_INET, SOCK_STREAM, 0, m_fds);
	m_thread = std::thread(std::bind(&NetWorkThread::Proc, this));
}

neapu::NetWorkThread::~NetWorkThread()
{
	this->Exit();
}

int neapu::NetWorkThread::dispatch(int fd)
{
	if (!m_eb)return -1;
	auto ev = event_new(m_eb, fd, EV_READ | EV_PERSIST, neapu::cbRead, this);
	if (!ev) {
		if (ev)event_free(ev);
		return -1;
	}
	m_eventList[fd]=ev;
	return event_add(ev, nullptr);
}

void neapu::NetWorkThread::WaitInit()
{
	std::unique_lock<std::mutex> lck(m_condMutex);
	m_cond.wait(lck);
}

void neapu::NetWorkThread::Exit()
{
	if (m_eb) {
		event_base_loopbreak(m_eb);
	}
	if (m_thread.joinable()) {
		m_thread.join();
	}
	m_eb = 0;
}

void neapu::NetWorkThread::Proc()
{
	m_eb = event_base_new();
	auto evExit = event_new(m_eb, m_fds[1], EV_READ,
		[](evutil_socket_t fd, short events, void* user_data) {
			event_base_loopbreak((event_base*)user_data);
		},
		m_eb);
	if (!evExit || event_add(evExit, nullptr) < 0) {
		event_base_free(m_eb);
		m_eb = nullptr;
		m_condMutex.unlock();
		return;
	}
	m_cond.notify_one();
	event_base_dispatch(m_eb);
	event_base_free(m_eb);
	m_eb = nullptr;
}

void neapu::NetWorkThread::OnRead(int fd)
{
	if (m_netBase->OnFdReadReady(fd) < 0) {
		event_del(m_eventList[fd]);
		event_free(m_eventList[fd]);
		m_eventList.erase(fd);
	}
}

void neapu::cbRead(evutil_socket_t fd, short events, void* user_data)
{
	static_cast<NetWorkThread*>(user_data)->OnRead(fd);
}
