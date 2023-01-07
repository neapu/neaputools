#ifndef __NEEVENTBASE2_H__
#define __NEEVENTBASE2_H__

#include "network/network_pub.h"
#include <WinSock2.h>
#include <functional>
#include <base/NEThreadPoll.h>
#include <memory>
#include <map>
#include <mutex>
#include <set>
#include <thread>

void signalHandler(int signal);
namespace neapu {
class NEAPU_NETWORK_EXPORT EventBase2 {
private:
#ifdef _WIN32
    struct SocketEvent {
        pollfd pfd;
        bool persist;
        bool trigger;
    };
#endif
public:
    EventBase2() noexcept;
    EventBase2(const EventBase2&) = delete;
    EventBase2(EventBase2&& _eb) noexcept;
    virtual ~EventBase2();

    enum EventType : short {
        None = 0,
        Read = 0x02,
        Write = 0x04
    };

    using SocketCallback = std::function<void(SOCKET_FD _fd, EventBase2::EventType)>;
    using SignalCallback = std::function<void(int _signal)>;
    using TimerCallback = std::function<void(int _timerID)>;

    int Init(int _threadCount = 1);

    void Release();

    int LoopStart();

    void Stop();

    int AddSocket(SOCKET_FD _fd, EventType _events, bool _persist = false, SocketCallback _callback = {});

    int AddSignal(int _signal, SignalCallback _callback = {});

    int AddTimer(uint64_t _period, bool _persist = false, TimerCallback _callback = {});

    virtual void OnSocketTriggerCallback(SOCKET_FD _fd, EventBase2::EventType _events);

    virtual void OnSignalCallback(int _signal);

    virtual void OnTimerCallback(int _timerID);

    int RemoveSocket(SOCKET_FD _fd);

    int RemoveTimer(int _timerID);

private:
    void FdTrigger(SOCKET_FD _fd, uint32_t events);
    void TimerProc();
#ifdef _WIN32
    void SignalTrigger(int _signal);
    friend void ::signalHandler(int signal);
#endif

private:
    struct TimerInfo {
        int id;
        bool isTrigger;
        bool persist;
        uint64_t period;
        uint64_t nextTrigger;
        TimerCallback callback;
    };

private:
    std::unique_ptr<ThreadPool2> m_threadPoolPtr = nullptr;
    bool m_running = false;
    std::map<int, SocketCallback> m_socketCallbackMap;
    std::map<int, SignalCallback> m_signalCallbackMap;
    std::map<int, TimerInfo> m_timerList;
    int m_timerBaseID = 1;
    int m_threadCount = 1;

#ifdef __linux__
    int m_epollFd = 0;
    std::set<int> m_signalFds;
#elif defined(_WIN32)

    std::map<SOCKET_FD, SocketEvent> m_socketList;
    std::recursive_mutex m_socketListMutex;
#endif
};
} // namespace neapu
#endif // __NEEVENTBASE2_H__