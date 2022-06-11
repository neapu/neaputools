#include "NEEventBase.h"
#include <event2/event.h>
#include <event2/thread.h>
#include "NENetworkError.h"

void neapu::EventBase::FileDescriptorCallback(evutil_socket_t fd, short events, void* user_data)
{
    auto ev = reinterpret_cast<Event*>(user_data);
    if (ev->fdcb) {
        ev->fdcb(fd, static_cast<EventType>(events & (EventType::Read | EventType::Write)), ev->handle);
    }
    ev->eventBase->OnFileDescriptorCallback(fd, static_cast<EventType>(events & (EventType::Read | EventType::Write)), ev->handle);
}

void neapu::EventBase::SignalCallback(evutil_socket_t fd, short events, void* user_data)
{
    auto ev = reinterpret_cast<Event*>(user_data);
    if (ev->sigcb) {
        ev->sigcb(fd, ev->handle);
    }
    ev->eventBase->OnSignalCallback(fd, ev->handle);
}

void neapu::EventBase::TimerCallback(evutil_socket_t fd, short events, void* user_data)
{
    auto ev = reinterpret_cast<Event*>(user_data);
    if (ev->timercb) {
        ev->timercb(ev->handle);
    }
    ev->eventBase->OnTimerCallback(ev->handle);
}

neapu::EventBase::EventBase(EventBase&& _eb) noexcept
{
    m_eb = _eb.m_eb;
    m_events = std::move(_eb.m_events);
    m_callback = std::move(_eb.m_callback);
}

neapu::EventBase::~EventBase()
{
    if (m_eb) {
        event_base_free(m_eb);
        m_eb = nullptr;
    }
}

int neapu::EventBase::Init(int _iocpThreadCount)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return ERROR_WSASTARTUP;
    }
    auto cfg = event_config_new();
    int rc = evthread_use_windows_threads();
    if (rc) {
        return ERROR_SET_WIN_THREAD;
    }
    //启用IOCP
    event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
    event_config_set_num_cpus_hint(cfg, _iocpThreadCount);
    m_eb = event_base_new_with_config(cfg);
    event_config_free(cfg);
#else
    m_eb = event_base_new();
#endif
    if (!m_eb) {
        return ERROR_EVENT_BASE;
    }
    return 0;
}

EventHandle neapu::EventBase::AddEvent(evutil_socket_t _fd, EventType _events, bool _persist, std::function<void(evutil_socket_t _fd, EventType _type, EventHandle _handle)> _cb)
{
    if (!m_eb)return nullptr;
    short evs = _events | EV_ET;
    if (_persist) {
        evs |= EV_PERSIST;
    }

    auto ev = std::make_shared<Event>();
    ev->eventBase = this;
    ev->fdcb = _cb;
    return AddEventImpl(_fd, evs, nullptr, &EventBase::FileDescriptorCallback, ev);
}

int neapu::EventBase::AddEvent(EventHandle _handle)
{
    if (!m_eb)return -1;
    //已经被释放，要重新创建
    if (m_events.find(_handle) == m_events.end()) {
        return -1;
    }
    return event_add(static_cast<event*>(_handle), nullptr);
}

EventHandle neapu::EventBase::AddSignal(int _signal, bool _persist, std::function<void(int _signal, EventHandle _handle)> _cb)
{
    if (!m_eb)return nullptr;
    short evs = EV_SIGNAL | EV_ET;
    if (_persist) {
        evs |= EV_PERSIST;
    }
    auto ev = std::make_shared<Event>();
    ev->eventBase = this;
    ev->sigcb = _cb;
    return AddEventImpl(_signal, evs, nullptr, &EventBase::SignalCallback, ev);
}

EventHandle neapu::EventBase::AddTimer(int _timeout, bool _persist, std::function<void(EventHandle _handle)> _cb)
{
    if (!m_eb)return nullptr;
    short evs = 0;
    if (_persist) {
        evs |= EV_PERSIST;
    }

    timeval tv{ _timeout / 1000, _timeout % 1000 };
    auto ev = std::make_shared<Event>();
    ev->eventBase = this;
    ev->timercb = _cb;
    return AddEventImpl(-1, evs, &tv, &EventBase::TimerCallback, ev);
}

void neapu::EventBase::ReleaseEvent(EventHandle _handle)
{
    if (m_events.find(_handle) == m_events.end()) {
        return;
    }
    m_events.erase(_handle);
    auto ev = reinterpret_cast<event*>(_handle);
    event_del(ev);
    event_free(ev);
}

void neapu::EventBase::OnFileDescriptorCallback(std::function<void(evutil_socket_t _fd, EventType _type, EventHandle _handle)> _cb)
{
    m_callback.onFileDescriptorCallback = _cb;
}

void neapu::EventBase::OnSignalCallback(std::function<void(int _signal, EventHandle _handle)> _cb)
{
    m_callback.onSignalCallback = _cb;
}

void neapu::EventBase::OnTimerCallback(std::function<void(EventHandle _handle)> _cb)
{
    m_callback.onTimerCallback = _cb;
}

int neapu::EventBase::EventLoop()
{
    if (!m_eb) {
        return ERROR_EVENT_UNINIT;
    }
    int rc = event_base_dispatch(m_eb);
    event_base_free(m_eb);
    m_eb = nullptr;
    OnEventLoopStoped();
    return rc;
}

int neapu::EventBase::EventLoopBreak()
{
    if (!m_eb) {
        return -1;
    }
    return event_base_loopbreak(m_eb);
}

void neapu::EventBase::OnFileDescriptorCallback(evutil_socket_t _fd, EventType _type, EventHandle _handle)
{
    if (m_callback.onFileDescriptorCallback) {
        m_callback.onFileDescriptorCallback(_fd, _type, _handle);
    }
}

void neapu::EventBase::OnSignalCallback(int _signal, EventHandle _handle)
{
    if (m_callback.onSignalCallback) {
        m_callback.onSignalCallback(_signal, _handle);
    }
}

void neapu::EventBase::OnTimerCallback(EventHandle _handle)
{
    if (m_callback.onTimerCallback) {
        m_callback.onTimerCallback(_handle);
    }
}

EventHandle neapu::EventBase::AddEventImpl(
    evutil_socket_t _fd, 
    short _events, 
    timeval* _timeout, 
    void(*_cb)(evutil_socket_t, short, void*), 
    std::shared_ptr<Event> _ev
)
{
    auto ev = event_new(nullptr, -1, 0, nullptr, nullptr);
    if (!ev) {
        return nullptr;
    }
    _ev->handle = ev;
    if (event_assign(ev, m_eb, _fd, _events, _cb, _ev.get()) < 0) {
        event_free(ev);
        return nullptr;
    }
    m_events[ev] = _ev;
    if (event_add(ev, _timeout) < 0) {
        m_events.erase(ev);
        event_free(ev);
        return nullptr;
    }
    return ev;
}
