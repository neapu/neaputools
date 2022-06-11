#include "NENetBase.h"
#ifdef _WIN32
#include <Windows.h>
#endif

using namespace neapu;

int neapu::NetBase::Init(int _threadNum)
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

#ifndef _WIN32
	//初始化线程池
	m_running = 1;
	m_threadPoll.Init(m_threadNum, std::bind(&NetBase::WorkThread, this));
#endif // !_WIN32
	return EventBase::Init(m_threadNum);
}


int neapu::NetBase::Run()
{
	return EventBase::EventLoop();
}

void neapu::NetBase::Stop()
{
	(void)EventBase::EventLoopBreak();
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
		if (m_writeQueue.dequeue(_fd)) {
			OnWriteReady(_fd);
		}
	}
}
#endif

void neapu::NetBase::OnReadReady(SocketEventCallback _cb)
{
	m_callback.readReady = _cb;
}

void neapu::NetBase::OnWriteReady(SocketEventCallback _cb)
{
	m_callback.writeReady = _cb;
}

void neapu::NetBase::OnReadReady(evutil_socket_t _socket, EventHandle _handle)
{
	if (m_callback.readReady) {
		m_callback.readReady(_socket, _handle);
	}
}

void neapu::NetBase::OnWriteReady(evutil_socket_t _socket, EventHandle _handle)
{
	if (m_callback.writeReady) {
		m_callback.writeReady(_socket, _handle);
	}
}

void neapu::NetBase::OnFileDescriptorCallback(evutil_socket_t _fd, EventType _type, EventHandle _handle)
{
	if (_type & EventType::Read) {
		OnReadReady(_fd, _handle);
		if (m_eventList.find(_handle) != m_eventList.end() && m_eventList[_handle]->_cb) {
			m_eventList[_handle]->_cb(_fd, _handle);
		}
	}
	if (_type & EventType::Write) {
		OnWriteReady(_fd, _handle);
		if (m_eventList.find(_handle) != m_eventList.end() && m_eventList[_handle]->_cb) {
			m_eventList[_handle]->_cb(_fd, _handle);
		}
	}
}

EventHandle neapu::NetBase::AddSocket(evutil_socket_t _fd, EventType _events, bool _persist, SocketEventCallback _cb)
{
	EventHandle rst = AddEvent(_fd, _events, _persist);
	if (EmptyHandle == rst) {
		return rst;
	}
	m_eventList[rst] = std::shared_ptr<Event>(new Event{ _fd, rst, _cb });
	return rst;
}