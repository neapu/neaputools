#include "NETcpServer2.h"
#include "logger/logger.h"
#include "network/NEEventBase2.h"
#include "network/NENetworkError.h"
#include "network/NETcpSocket.h"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <mutex>
#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
#define RECV_EOF_EN 10035
#else
#define RECV_EOF_EN EAGAIN
#endif

using namespace neapu;

int TcpServer2::Init(const IPAddress& _addr, int _threadNum)
{
    m_address = _addr;
    int rc = EventBase2::Init(_threadNum);
    if (rc < 0) {
        return rc;
    }

    if (m_address.IsIPv4()) {
        m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    } else if (m_address.IsIPv6()) {
        m_listenFd = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if (m_listenFd <= 0) {
        SetLastError();
        return ERROR_SOCKET_OPEN;
    }

    // 设置非阻塞
    rc = SetSocketNonBlock(m_listenFd);
    if (0 != rc) {
        return ERROR_SOCKET_NONBLOCK;
    }

    // 设置重入
    rc = SetSocketReuseable(m_listenFd);
    if (0 != rc) {
        return ERROR_SET_REUSEADDR;
    }

    if (m_address.IsIPv4()) {
        sockaddr_in sin = {0};
        m_address.ToSockaddr(&sin);

        if (bind(m_listenFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            SetLastError();
            return ERROR_BIND;
        }
    } else if (m_address.IsIPv6()) {
        sockaddr_in6 sin = {0};
        m_address.ToSockaddr(&sin);

        if (bind(m_listenFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            SetLastError();
            return ERROR_BIND;
        }
    }

    rc = EventBase2::AddSocket(m_listenFd, EventBase2::Read);
    if (rc < 0) {
        EventBase2::Release();
        return rc;
    }
    return 0;
}

int TcpServer2::Listen()
{
    if (m_listenFd <= 0) {
        return ERROR_LISTEN;
    }
    int ret = ::listen(m_listenFd, 20);
    if (ret < 0) {
        SetLastError();
        return ERROR_LISTEN;
    }
    return EventBase2::LoopStart();
}

void TcpServer2::Stop()
{
    EventBase2::Stop();
}

TcpServer2& TcpServer2::OnReceive(TcpServerCallback _cb)
{
    m_callback.onReceive = _cb;
    return *this;
}

TcpServer2& TcpServer2::OnAccepted(TcpServerCallback _cb)
{
    m_callback.onAccepted = _cb;
    return *this;
}

TcpServer2& TcpServer2::OnClosed(TcpServerCallback _cb)
{
    m_callback.onClose = _cb;
    return *this;
}

TcpServer2& TcpServer2::OnError(TcpServerCallback _cb)
{
    m_callback.onError = _cb;
    return *this;
}

void TcpServer2::OnReceive(TcpSocketPtr _socket)
{
    if (m_callback.onReceive) {
        m_callback.onReceive(_socket);
    }
}

void TcpServer2::OnAccepted(TcpSocketPtr _socket)
{
    if (m_callback.onAccepted) {
        m_callback.onAccepted(_socket);
    }
}

void TcpServer2::OnClose(TcpSocketPtr _socket)
{
    if (m_callback.onClose) {
        m_callback.onClose(_socket);
    }
}

void TcpServer2::OnError(TcpSocketPtr _socket)
{
    if (m_callback.onError) {
        m_callback.onError(_socket);
    }
}

void TcpServer2::SetLastError()
{
    m_err.code = GetSocketError(m_listenFd);
    m_err.str = GetErrorString(m_err.code);
}

void TcpServer2::OnSocketTriggerCallback(SOCKET_FD _fd, EventBase2::EventType _events)
{
    if (_events & EventBase2::Read) {
        if (_fd == m_listenFd) {
            OnListenerAccept();
        } else {
            OnClientReadReady(_fd);
        }
    }
}

void TcpServer2::OnListenerAccept()
{
    int accp_fd;
    IPAddress addr;
    if (m_address.IsIPv4()) {
        sockaddr_in sin;
        socklen_t len = sizeof(sin);
        accp_fd = accept(m_listenFd, (sockaddr*)&sin, &len);
        if (accp_fd <= 0) {
            return;
        }
        addr = IPAddress::MakeAddress(sin);
    } else {
        sockaddr_in6 sin;
        socklen_t len = sizeof(sin);
        accp_fd = accept(m_listenFd, (sockaddr*)&sin, &len);
        if (accp_fd <= 0) {
            return;
        }
        addr = IPAddress::MakeAddress(sin);
    }

    int ret = SetSocketNonBlock(accp_fd);
    if (ret < 0) {
        LOG_ERROR << "SetSocketNonBlock ERROR:" << ret << " socket:" << accp_fd;
        close(accp_fd);
        return;
    }

    auto sock = AddClient(accp_fd, addr);
    OnAccepted(sock);
}

void TcpServer2::OnClientReadReady(SOCKET_FD _fd)
{
    auto client = GetClient(_fd);

    char buf[1];
    int rc = recv(_fd, buf, 1, MSG_PEEK);
    if (rc == 0) { //客户端主动关闭
        OnClose(client);
        ReleaseClient(_fd);
    } else if (rc < 0) {
        std::unique_lock<std::recursive_mutex> errorLocker(m_errorMutex);
        SetLastError();
        if (m_err.code != 0 && m_err.code != RECV_EOF_EN) {
            client->SetError(m_err);
            errorLocker.unlock();
            OnError(client);
            ReleaseClient(_fd);
        }
    } else {
        OnReceive(client);
    }

    if (client->IsClosed()) {
        ReleaseClient(_fd);
    }
}

TcpSocketPtr TcpServer2::AddClient(SOCKET_FD _fd, const IPAddress& _addr)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_socketMap.find(_fd) != m_socketMap.end()) {
        ReleaseClient(_fd);
    }
    m_socketMap[_fd] = std::shared_ptr<TcpSocket>(new TcpSocket(_fd, _addr));
    AddSocket(_fd, EventBase2::Read);
    return m_socketMap[_fd];
}

void TcpServer2::ReleaseClient(SOCKET_FD _fd)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_socketMap.find(_fd) != m_socketMap.end()) {
        EventBase2::RemoveSocket(_fd);
        m_socketMap[_fd]->Close();
        m_socketMap.erase(_fd);
    }
}

TcpSocketPtr TcpServer2::GetClient(SOCKET_FD _fd)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_socketMap.find(_fd) != m_socketMap.end()) {
        return m_socketMap[_fd];
    }
    return nullptr;
}