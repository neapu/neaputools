#include "NEEventBase2.h"
#include "NEThreadPoll.h"
#include "logger/logger.h"
#include <bits/types/sigset_t.h>
#include <csignal>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <base/NEDateTime.h>
#ifdef __linux__
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
#ifndef USE_LIBEVENT
using namespace neapu;

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
    int ret = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, _fd, nullptr);
    if (ret < 0) {
        LOG_ERROR << "epoll_ctl ERROR:" << errno;
        return -1;
    }
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
}

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

#endif