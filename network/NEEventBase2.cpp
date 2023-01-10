#include "NEEventBase2.h"
#include "NEThreadPoll.h"
#include "logger/logger.h"
#include "network/NEEventBase2.h"
#include "network/network_pub.h"
#include <csignal>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <thread>
#include <base/NEDateTime.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <signal.h>
#else
#include <bits/types/sigset_t.h>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
constexpr auto MAX_EVENT_COUNT = 2048;
#endif

using namespace neapu;

#ifdef _WIN32
std::map<int, std::set<EventBase2*>> g_signalInstanceList;
std::mutex g_instanceMutex;
void signalHandler(int signal)
{
    if (g_signalInstanceList.find(signal) != g_signalInstanceList.end()) {
        for (auto instance : g_signalInstanceList[signal]) {
            instance->SignalTrigger(signal);
        }
    }
}
#endif

EventBase2::EventBase2() noexcept
{
}

EventBase2::~EventBase2()
{
}

int EventBase2::Init(int _threadCount)
{
    if (m_running) {
        return -1;
    }

    m_threadCount = _threadCount;

#ifdef __linux__
    m_epollFd = epoll_create1(0);
    if (m_epollFd <= 0) {
        LOG_ERROR << "epoll_create1 ERROR:" << errno;
        return -1;
    }
#elif _WIN32

#endif
    return 0;
}

void EventBase2::Release()
{
#ifdef __linux__
    if (m_epollFd) {
        close(m_epollFd);
    }
    m_signalFds.clear();
#endif
    m_socketCallbackMap.clear();
    m_signalCallbackMap.clear();
    m_timerList.clear();
}

int EventBase2::LoopStart()
{
    m_threadPoolPtr = std::make_unique<ThreadPool2>(m_threadCount);
    m_threadPoolPtr->init();
#ifdef __linux__
    if (m_epollFd <= 0) {
        LOG_ERROR << "epoll no init";
        return -1;
    }

    if (m_threadPoolPtr == nullptr) {
        LOG_ERROR << "Thread Pool no init";
        return -1;
    }

    m_running = true;
    int ret = 0;
    struct epoll_event events[MAX_EVENT_COUNT] = {};
    while (m_running) {
        ret = epoll_wait(m_epollFd, events, MAX_EVENT_COUNT, 1);
        if (ret < 0) {
            LOG_ERROR << "epoll_wait ERROR";
            m_running = false;
            close(m_epollFd);
            m_epollFd = 0;
            m_threadPoolPtr->shutdown();
            return -1;
        }

        for (int i = 0; i < ret; i++) {
            SOCKET_FD fd = events[i].data.fd;
            uint32_t ev = events[i].events;
            m_threadPoolPtr->submit(
                std::bind(&EventBase2::FdTrigger, this, std::placeholders::_1, std::placeholders::_2),
                fd, ev);
        }
        TimerProc();
    }
    close(m_epollFd);
    m_epollFd = 0;
#elif defined(_WIN32)
    if (m_threadPoolPtr == nullptr) {
        LOG_ERROR << "Thread Pool no init";
        return -1;
    }

    m_running = true;
    int ret = 0;
    int fdCount = 0;
    int index = 0;
    while (m_running) {
        pollfd* pfds = nullptr;
        {
            std::unique_lock<std::recursive_mutex> locker(m_socketListMutex);
            fdCount = m_socketList.size();
            if (fdCount <= 0)continue;
            pfds = new pollfd[fdCount];
            index = 0;
            for (auto& [sock, se] : m_socketList) {
                if (se.trigger == true) continue;
                pfds[index] = se.pfd;
                pfds[index].revents = 0;
                ++index;
            }
        }

        if(index <= 0){
            delete[] pfds;
            continue;
        }

#ifdef _WIN32
        ret = WSAPoll(pfds, index, 1);
#endif
        if (ret < 0) {
            LOG_ERROR << "poll error:" << ret;
            m_threadPoolPtr->shutdown();
            return -1;
        }

        for (int i = 0; i < index; i++) {
            if (pfds[i].revents != 0) {
                uint32_t ev = pfds[i].revents;
                SOCKET_FD fd = pfds[i].fd;
#ifdef _WIN32
                std::unique_lock<std::recursive_mutex> locker(m_socketListMutex);
                if (m_socketList.find(fd) != m_socketList.end()) {
                    auto& se = m_socketList[fd];
                    se.trigger = true;
                }
#endif
                m_threadPoolPtr->submit(
                    std::bind(&EventBase2::FdTrigger, this, std::placeholders::_1, std::placeholders::_2),
                    fd, ev);
            }
        }
        delete[] pfds;
        TimerProc();
    }
#endif
    m_threadPoolPtr->shutdown();
    return 0;
}

void EventBase2::Stop()
{
    m_running = false;
}

int EventBase2::AddSocket(SOCKET_FD _fd, EventType _events, bool _persist, SocketCallback _callback)
{
#ifdef __linux__
    struct epoll_event ev;
    ev.data.fd = _fd;
    ev.events = EPOLLET;
    if (_events & Read) {
        ev.events |= EPOLLIN;
    }
    if (_events & Write) {
        ev.events |= EPOLLOUT;
    }
    if (_persist == false) {
        ev.events |= EPOLLONESHOT;
    }
    if (_callback) {
        m_socketCallbackMap[_fd] = _callback;
    }

    int ret = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, _fd, &ev);
    if (ret < 0) {
        LOG_ERROR << "epoll_ctl ERROR:" << errno;
        if (_callback) {
            m_socketCallbackMap.erase(_fd);
        }
        return -1;
    }
#elif defined(_WIN32)
    SocketEvent se;
    se.pfd.fd = _fd;
    se.pfd.events = 0;
    se.pfd.revents = 0;
    if (_events & Read) {
        se.pfd.events |= POLLIN;
    }
    if (_events & Write) {
        se.pfd.events |= POLLOUT;
    }
    se.persist = _persist;
    se.trigger = false;
    std::unique_lock<std::recursive_mutex> locker(m_socketListMutex);
    m_socketList[_fd] = se;

#endif
    return 0;
}

int EventBase2::AddSignal(int _signal, SignalCallback _callback)
{
#ifdef __linux__
    sigset_t mask;
    sigemptyset(&mask);
    int ret = sigaddset(&mask, _signal);
    if (ret < 0) {
        LOG_ERROR << "sigaddset ERROR:" << errno;
        return -1;
    }
    ret = sigprocmask(SIG_BLOCK, &mask, NULL);
    if (ret < 0) {
        LOG_ERROR << "sigprocmask ERROR:" << errno;
        return -1;
    }
    int sfd = signalfd(-1, &mask, 0);
    m_signalFds.insert(sfd);
    if (_callback) {
        m_signalCallbackMap[_signal] = _callback;
    }
    ret = AddSocket(sfd, Read);
    if (ret < 0 && _callback) {
        m_signalCallbackMap.erase(_signal);
    }
    return ret;
#elif defined(_WIN32)
    if (_callback) {
        m_signalCallbackMap[_signal] = _callback;
    }
    std::unique_lock<std::mutex> locker(g_instanceMutex);
    if (g_signalInstanceList.find(_signal) == g_signalInstanceList.end()) {
        g_signalInstanceList[_signal].insert(this);
        signal(_signal, signalHandler);
    } else {
        g_signalInstanceList[_signal].insert(this);
    }

#endif
    return 0;
}

int EventBase2::AddTimer(uint64_t _period, bool _persist, TimerCallback _callback)
{
    TimerInfo timer;
    timer.id = m_timerBaseID++;
    timer.period = _period;
    timer.persist = _persist;
    timer.nextTrigger = DateTime::CurrentTimestampMs() + _period;
    timer.callback = _callback;
    timer.isTrigger = false;
    m_timerList[timer.id] = timer;
    return timer.id;
}

void EventBase2::OnSocketTriggerCallback(SOCKET_FD _fd, EventBase2::EventType _events)
{
    if (m_socketCallbackMap.contains(_fd) && m_socketCallbackMap[_fd] != nullptr) {
        m_socketCallbackMap[_fd](_fd, _events);
    }
}

void EventBase2::OnSignalCallback(int _signal)
{
    if (m_signalCallbackMap.contains(_signal) && m_signalCallbackMap[_signal] != nullptr) {
        m_signalCallbackMap[_signal](_signal);
    }
}

void EventBase2::OnTimerCallback(int _timerID)
{
    if (m_timerList.contains(_timerID) && m_timerList[_timerID].callback != nullptr) {
        m_timerList[_timerID].callback(_timerID);
    }
}

int EventBase2::RemoveSocket(SOCKET_FD _fd)
{
    if (m_socketCallbackMap.contains(_fd)) {
        m_socketCallbackMap.erase(_fd);
    }
#ifdef __linux__
    int ret = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, _fd, nullptr);
    if (ret < 0) {
        LOG_ERROR << "epoll_ctl ERROR:" << errno;
        return -1;
    }
#elif _WIN32
    std::unique_lock<std::recursive_mutex> locker(m_socketListMutex);
    if (m_socketList.find(_fd) != m_socketList.end()) {
        m_socketList.erase(_fd);
    }
#endif

    return 0;
}

int EventBase2::RemoveTimer(int _timerID)
{
    if (m_timerList.contains(_timerID)) {
        m_timerList.erase(_timerID);
    }
    return 0;
}

// 此函数运行在工作线程里
void EventBase2::FdTrigger(SOCKET_FD _fd, uint32_t events)
{
#ifdef _WIN32
    short type = None;
    if (events & POLLIN) {
        type |= Read;
    }
    if (events & POLLOUT) {
        type |= Write;
    }
    if (events & POLLERR) {
        type |= Error;
    }
    this->OnSocketTriggerCallback(_fd, (EventType)type);
    std::unique_lock<std::recursive_mutex> locker(m_socketListMutex);
    if (m_socketList.find(_fd) != m_socketList.end()) {
        auto& se = m_socketList[_fd];
        if (se.persist == false) {
            m_socketList.erase(_fd);
        } else {
            se.trigger = false;
        }
    }

#else
    if (m_signalFds.contains(_fd)) {
        m_signalFds.erase(_fd);
        struct signalfd_siginfo fdsiI;
        read(_fd, &fdsiI, sizeof(struct signalfd_siginfo));
        this->OnSignalCallback(fdsiI.ssi_signo);
    } else {
        short type = None;
        if (events & EPOLLIN) {
            type |= Read;
        }
        if (events & EPOLLOUT) {
            type |= Write;
        }
        this->OnSocketTriggerCallback(_fd, (EventType)type);
    }
#endif
}
// #endif

void EventBase2::TimerProc()
{
    for (auto& [timerid, timer] : m_timerList) {
        int id = timer.id;
        uint64_t curTime = DateTime::CurrentTimestampMs();
        if (timer.nextTrigger <= curTime && timer.isTrigger == false) {
            m_timerList[id].isTrigger = true;
            m_threadPoolPtr->submit([id, this]() {
                this->OnTimerCallback(id);
                auto& timer = m_timerList[id];
                if (timer.persist) {
                    timer.nextTrigger += timer.period;
                    timer.isTrigger = false;
                } else {
                    m_timerList.erase(id);
                }
            });
        }
    }
}

#ifdef _WIN32
void EventBase2::SignalTrigger(int _signal)
{
    if (m_threadPoolPtr == nullptr) return;
    m_threadPoolPtr->submit([this, _signal]() {
        this->OnSignalCallback(_signal);
    });
}
#endif