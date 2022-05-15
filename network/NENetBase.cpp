#include "NENetBase.h"
#include <event2/event.h>
#include <event2/thread.h>
#ifdef _WIN32
#include <Windows.h>
#endif

static void neapu::cbSocketEvent(evutil_socket_t fd, short events, void* user_data)
{
	auto netBase = static_cast<NetBase*>(user_data);
	if (events & EV_READ) {
#ifndef _WIN32
		netBase->m_readQueue.enqueue(fd);
#else
		netBase->OnReadReady(fd);
#endif
	}
	if (events & EV_WRITE) {
		netBase->OnWriteReady(fd);
	}
}

static void neapu::cbSignalEvent(evutil_socket_t fd, short events, void* user_data)
{
	auto netBase = static_cast<NetBase*>(user_data);
	netBase->OnSignalReady(fd);
}

neapu::NetBase::NetBase(NetBase&& _nb) noexcept
{
	m_eb = _nb.m_eb;
	_nb.m_eb = nullptr;
}

neapu::NetBase::~NetBase() noexcept
{
	Stop();
}

int neapu::NetBase::InitEvent(int _threadNum)
{
	int rc = 0;
	m_threadNum = _threadNum;
	if (_threadNum == 0) {
#ifdef _WIN32
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		m_threadNum = si.dwNumberOfProcessors;
#else
		m_threadNum = 1;
#endif // _WIN32
	}

	
#ifdef _WIN32
	auto cfg = event_config_new();
	rc = evthread_use_windows_threads();
	if (rc) {
		return ERROR_SET_WIN_THREAD;
	}
	//启用IOCP
	event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	event_config_set_num_cpus_hint(cfg, m_threadNum);
	m_eb = event_base_new_with_config(cfg);
	event_config_free(cfg);
#else
	m_eb = event_base_new();
#endif // _WIN32
	if (!m_eb) {
		return ERROR_EVENT_BASE;
	}

#ifndef _WIN32
	//初始化线程池
	m_running = 0;
	m_threadPoll.Init(m_threadNum, std::bind(&NetBase::WorkThread, this));
#endif // !_WIN32
	return 0;
}

int neapu::NetBase::AddSocket(int _fd, short _ev)
{
	if (!m_eb) return ERROR_EB_NULLPTR;
	auto ev = event_new(m_eb, _fd, _ev | EV_PERSIST | EV_ET, cbSocketEvent, this);
	if (!ev) {
		return ERROR_EVENT_NEW;
	}
	if (event_add(ev, nullptr)) {
		event_free(ev);
		return ERROR_EVENT_ADD;
	}
	m_socketEventList[_fd] = ev;
	return 0;
}

int neapu::NetBase::AddSignal(int _signal)
{
	if (!m_eb) return ERROR_EB_NULLPTR;
	auto ev = evsignal_new(m_eb, _signal, cbSignalEvent, this);
	if (!ev) {
		return ERROR_EVENT_NEW;
	}
	if (event_add(ev, nullptr)) {
		event_free(ev);
		return ERROR_EVENT_ADD;
	}
	m_signalEventList[_signal] = ev;
	return 0;
}

void neapu::NetBase::RemoveSocket(int _fd)
{
	if (m_socketEventList.find(_fd) != m_socketEventList.end()) {
		auto ev = m_socketEventList[_fd];
		event_del(ev);
		m_socketEventList.erase(_fd);
		event_free(ev);
	}
}

void neapu::NetBase::RemoveSignal(int _signal)
{
	if (m_signalEventList.find(_signal) != m_signalEventList.end()) {
		auto ev = m_signalEventList[_signal];
		event_del(ev);
		m_signalEventList.erase(_signal);
		event_free(ev);
	}
}

int neapu::NetBase::Run()
{
	if (!m_eb) {
		return ERROR_EVENT_UNINIT;
	}
	int rc = event_base_dispatch(m_eb);
	event_base_free(m_eb);
	m_eb = nullptr;
	Stoped();
	return rc;
}

void neapu::NetBase::Stop()
{
	if (!m_eb)return;
	event_base_loopbreak(m_eb);
#ifndef _WIN32
	m_running = 0;
	m_threadPoll.Join();
#endif
}

void neapu::NetBase::SetLastError(int _err, String _errstr)
{
	m_err.code = _err;
	m_err.str = _errstr;
}

#ifndef _WIN32
void neapu::NetBase::WorkThread()
{
	while (m_running) {
		evutil_socket_t _fd;
		if (m_readQueue.dequeue(_fd))
		{
			OnReadReady(_fd);
		}
	}
}
#endif