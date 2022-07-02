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
#include "NEEventBase.h"
#include "NENetworkError.h"
#include "network_pub.h"

using SocketEventCallback = std::function<void(evutil_socket_t, EventHandle)>;

namespace neapu {
class NEAPU_NETWORK_EXPORT NetBase : public EventBase {
public:
    int Run();
    void Stop();
    NetworkError GetError()
    {
        return m_err;
    }
    int Init(int _threadNum);

    EventHandle AddSocket(evutil_socket_t _fd,
                          EventType _events,
                          bool _persist           = false,
                          SocketEventCallback _cb = {});

    void OnReadReady(SocketEventCallback _cb);
    void OnWriteReady(SocketEventCallback _cb);

    int EventLoop() = delete;

protected:
    virtual void OnReadReady(evutil_socket_t _socket, EventHandle _handle);
    virtual void OnWriteReady(evutil_socket_t _socket, EventHandle _handle);

    virtual void OnFileDescriptorCallback(evutil_socket_t _fd, EventType _type, EventHandle _handle) override;

    void SetLastError(int _err, String _errstr);
#ifndef _WIN32
    virtual void WorkThread();
#endif
protected:
    int m_threadNum = 0;
    NetworkError m_err;
    struct {
        SocketEventCallback readReady;
        SocketEventCallback writeReady;
    } m_callback;

    using Event = struct {
        evutil_socket_t fd;
        EventHandle handle;
        SocketEventCallback _cb;
    };
    std::map<EventHandle, std::shared_ptr<Event>> m_eventList;
#ifndef _WIN32
    ThreadPoll<std::function<void()>> m_threadPoll;
    SafeQueue<EventHandle> m_readQueue;
    SafeQueue<EventHandle> m_writeQueue;
    int m_running = 0;
#endif
};
} // namespace neapu