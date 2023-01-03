#ifdef USE_LIBEVENT
#include "NETcpServer.h"
#include <event2/event.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <functional>
#include "NEString.h"
#include <event2/thread.h>

#define BUF_SIZE 1024
#ifdef _WIN32
#define RECV_EOF_EN 10035
#else
#define RECV_EOF_EN EAGAIN
#endif

using namespace neapu;

int neapu::TcpServer::Init(int _threadNum, const IPAddress& _addr)
{
    m_address = _addr;
    int rc = NetBase::Init(_threadNum);
    if (rc < 0) {
        Stop();
        return rc;
    }

    if (m_address.IsIPv4()) {
        m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (m_address.IsIPv6()) {
        m_listenFd = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if (m_listenFd <= 0) {
        int err = evutil_socket_geterror(m_listenFd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_SOCKET_OPEN;
    }

    //设置非阻塞
    rc = evutil_make_socket_nonblocking(m_listenFd);
    if (0 != rc) {
        return ERROR_SOCKET_NONBLOCK;
    }
    //设置端口重入
    rc = evutil_make_listen_socket_reuseable(m_listenFd);
    if (0 != rc) {
        return ERROR_SET_REUSEADDR;
    }

    if (m_address.IsIPv4()) {
        sockaddr_in sin = { 0 };
        m_address.ToSockaddr(&sin);

        if (bind(m_listenFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_listenFd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_BIND;
        }
    }
    else if (m_address.IsIPv6()) {
        sockaddr_in6 sin = { 0 };
        m_address.ToSockaddr(&sin);

        if (bind(m_listenFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_listenFd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_BIND;
        }
    }

    

    m_listenHandle = this->AddSocket(m_listenFd, EventType::Read, true);
    if (m_listenHandle == EmptyHandle) {
        Stop();
        return rc;
    }
    return 0;
}

int neapu::TcpServer::Listen()
{
    if (listen(m_listenFd, 10) < 0) {
        int err = evutil_socket_geterror(m_listenFd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_LISTEN;
    }
    return 0;
}

void neapu::TcpServer::OnEventLoopStoped()
{
    NetBase::OnEventLoopStoped();
    if (m_listenFd) {
        evutil_closesocket(m_listenFd);
        m_listenFd = 0;
    }
}

TcpServer& neapu::TcpServer::OnRecvData(TcpServerCallback _cb)
{
    m_callback.onRecvData = _cb;
    return *this;
}

TcpServer& neapu::TcpServer::OnAccepted(TcpServerCallback _cb)
{
    m_callback.onAccepted = _cb;
    return *this;
}

TcpServer& neapu::TcpServer::OnChannelClosed(TcpServerCallback _cb)
{
    m_callback.onChannelClose = _cb;
    return *this;
}

TcpServer& neapu::TcpServer::OnChannelError(TcpServerCallback _cb)
{
    m_callback.onChannelError = _cb;
    return *this;
}

TcpServer& neapu::TcpServer::OnChannelWrite(TcpServerCallback _cb)
{
    m_callback.onChannelWrite = _cb;
    return *this;
}

void neapu::TcpServer::OnRecvData(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_callback.onRecvData) {
        m_callback.onRecvData(_client);
    }
}

void neapu::TcpServer::OnAccepted(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_callback.onAccepted) {
        m_callback.onAccepted(_client);
    }
}

void neapu::TcpServer::OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_callback.onChannelClose) {
        m_callback.onChannelClose(_client);
    }
}

void neapu::TcpServer::OnChannelError(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_callback.onChannelError) {
        m_callback.onChannelError(_client);
    }
}

void neapu::TcpServer::OnChannelWrite(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_callback.onChannelWrite) {
        m_callback.onChannelWrite(_client);
    }
}

int TcpServer::OnClientReadReady(int _fd, EventHandle _handle)
{
    auto client = m_channels[_fd];
    
    char buf[1];
    int rc = recv(_fd, buf, 1, MSG_PEEK);
    int err = evutil_socket_geterror(_fd);
    if (rc == 0) {
        OnChannelClosed(client);
        client->Close();
    }
    else if (rc < 0 && err != 0 && err != RECV_EOF_EN) {
        SetLastError(err, evutil_socket_error_to_string(err));
        client->SetError(m_err);
        OnChannelError(client);
        client->Close();
    }
    else {
        OnRecvData(client);
    }
    
    if (client->IsClosed()) { //如果链接被关闭了就清理
        this->ReleaseEvent(_handle);
        ReleaseClient(_fd);
    }
    return 0;
}

void neapu::TcpServer::OnReadReady(evutil_socket_t _socket, EventHandle _handle)
{
    if (static_cast<int>(_socket) == m_listenFd) {
        OnListenerAccept(static_cast<int>(_socket));
    }
    else {
        OnClientReadReady(static_cast<int>(_socket), _handle);
    }
}

void neapu::TcpServer::OnWriteReady(evutil_socket_t _socket, EventHandle _handle)
{
    if (m_channels.find(static_cast<int>(_socket)) != m_channels.end()) {
        OnChannelWrite(m_channels[static_cast<int>(_socket)]);
    }
    
}

void neapu::TcpServer::OnListenerAccept(int fd)
{
    int accp_fd;
    IPAddress addr;
    if (m_address.IsIPv4()) {
        sockaddr_in sin;
        socklen_t len = sizeof(sin);
        accp_fd = accept(fd, (sockaddr*)&sin, &len);
        if (accp_fd <= 0) {
            return;
        }
        addr = IPAddress::MakeAddress(sin);
    }
    else {
        sockaddr_in6 sin;
        socklen_t len = sizeof(sin);
        accp_fd = accept(fd, (sockaddr*)&sin, &len);
        if (accp_fd <= 0) {
            return;
        }
        addr = IPAddress::MakeAddress(sin);
    }

    AddClient(accp_fd, addr);

    if (evutil_make_socket_nonblocking(accp_fd) < 0) {
        OnChannelClosed(m_channels[accp_fd]);
        ReleaseClient(accp_fd);
        return;
    }

    (void)this->AddSocket(accp_fd, EventType::Read, true);
    OnAccepted(m_channels[accp_fd]);
}

void neapu::TcpServer::AddClient(int fd, const IPAddress& _addr)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_channels.find(fd) != m_channels.end()) {
        ReleaseClient(fd);
    }
    m_channels[fd] = std::shared_ptr<NetChannel>(new NetChannel(fd, _addr));
}

void neapu::TcpServer::ReleaseClient(int fd)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_channels.find(fd) != m_channels.end()) {
        m_channels[fd]->Close();
        m_channels.erase(fd);
    }
}

void neapu::TcpServer::Stop()
{
    if (m_listenHandle) {
        this->ReleaseEvent(m_listenHandle);
        m_listenHandle = EmptyHandle;
    }
    NetBase::Stop();
}
#endif