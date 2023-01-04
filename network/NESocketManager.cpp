#include "NESocketManager.h"
#include "network/NEEventBase2.h"
#include "network/network_pub.h"
#include <mutex>

using namespace neapu;

int SocketManager::AddSocket(SocketBasePtr _sock)
{
    int ret = SetSocketNonBlock(_sock->GetSocket());
    if (ret < 0) {
        return ret;
    }
    ret = EventBase2::AddSocket(_sock->GetSocket(), EventBase2::Read, true);
    if (ret < 0) {
        return ret;
    }
    std::unique_lock<std::recursive_mutex> locker(m_mapMutex);
    m_socketMap[_sock->GetSocket()] = _sock;
    return 0;
}

SocketManager& SocketManager::OnReadEvent(SocketCallback _cb)
{
    m_callback = _cb;
    return *this;
}

void SocketManager::OnReadEvent(SocketBasePtr _sock)
{
    if (m_callback) {
        m_callback(_sock);
    }
}

void SocketManager::OnSocketTriggerCallback(SOCKET_FD _fd, EventBase2::EventType _events)
{
    if (_events & EventBase2::Read) {
        std::unique_lock<std::recursive_mutex> locker(m_mapMutex);
        if (m_socketMap.find(_fd) == m_socketMap.end()) {
            return;
        }
        auto sock = m_socketMap[_fd];
        locker.unlock();
        OnReadEvent(sock);
    }
}