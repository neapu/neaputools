#include "NETcpServer.h"
#include <event2/event.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <functional>
#include "NEString.h"
#include <event2/thread.h>

#define BUF_SIZE 1024

using namespace neapu;

int neapu::TcpServer::Init(int _threadNum, const IPAddress& _addr, bool _enableWriteCallback = false)
{
    m_address = _addr;
    if (_enableWriteCallback) {
        m_socketEvent = EV_READ | EV_WRITE;
    }
    else {
        m_socketEvent = EV_READ;
    }
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup error");
        return 0;
    }
#endif // WIN32

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
    int rc = evutil_make_socket_nonblocking(m_listenFd);
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

    if (listen(m_listenFd, 10) < 0) {
        int err = evutil_socket_geterror(m_listenFd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_LISTEN;
    }

    rc = NetBase::InitEvent(_threadNum);
    if (rc < 0) {
        Stop();
        return rc;
    }


    rc = NetBase::AddSocket(m_listenFd, m_socketEvent);
    if (rc < 0) {
        Stop();
        return rc;
    }
    rc = NetBase::AddSignal(SIGINT);
    if (rc < 0) {
        Stop();
        return rc;
    }
    return 0;
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

int TcpServer::OnClientReadReady(int _fd)
{
    auto client = m_channels[_fd];
    char buf[BUF_SIZE];
    int readSize;
    for (;;) {
        readSize = recv(_fd, buf, BUF_SIZE, 0);
        if (readSize == EOF) { //接收完成
            int err = evutil_socket_geterror(_fd);
            if (err != 0 && err != 10035) { //对面意外掉线
                SetLastError(err, evutil_socket_error_to_string(err));
                client->Close();
                OnChannelClosed(client);
                ReleaseClient(_fd);
                return -1;
            }
            break;
        }
        else if (readSize == 0) { //对面主动断开
            int err = evutil_socket_geterror(_fd);
            if (err != 0) {
                SetLastError(err, evutil_socket_error_to_string(err));
            }
            client->Close();
            OnChannelClosed(client);
            ReleaseClient(_fd);
            return -1;
        }
        else if (readSize < 0) { //发生错误
            int err = evutil_socket_geterror(_fd);
            SetLastError(err, evutil_socket_error_to_string(err));
            client->SetLastError(err, evutil_socket_error_to_string(err));
            client->Close();
            OnChannelError(client);
            ReleaseClient(_fd);
            return -2;
        }
        client->AppendData(buf, readSize);
    }
    OnRecvData(client);
    if (client->IsClosed()) { //如果链接被关闭了就清理
        ReleaseClient(_fd);
    }
    return 0;
}

void neapu::TcpServer::OnReadReady(int _fd)
{
    if (_fd == m_listenFd) {
        OnListenerAccept(_fd);
    }
    else {
        OnClientReadReady(_fd);
    }
}

void neapu::TcpServer::OnWriteReady(int _fd)
{

}

void neapu::TcpServer::OnSignalReady(int _signal)
{
    if (_signal == SIGINT) {
        Stop();
    }
}

void neapu::TcpServer::OnListenerAccept(int fd)
{
    sockaddr_in sin;
    int len = sizeof(sin);
    int accp_fd = accept(fd, (sockaddr*)&sin, &len);
    if (accp_fd <= 0) {
        return;
    }

    AddClient(accp_fd, sin);

    if (evutil_make_socket_nonblocking(accp_fd) < 0) {
        OnChannelClosed(m_channels[accp_fd]);
        ReleaseClient(accp_fd);
        return;
    }

    NetBase::AddSocket(accp_fd, m_socketEvent);
}

std::shared_ptr<NetChannel> neapu::TcpServer::MakeChannel(int fd, sockaddr_in& sin)
{
    return std::shared_ptr<NetChannel>(new NetChannel(fd, ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port)));
}

void neapu::TcpServer::AddClient(int fd, sockaddr_in& sin)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_channels.find(fd) != m_channels.end()) {
        ReleaseClient(fd);
    }
    m_channels[fd] = MakeChannel(fd, sin);
}

void neapu::TcpServer::ReleaseClient(int fd)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_channels.find(fd) != m_channels.end()) {
        m_channels[fd]->Close();
        m_channels.erase(fd);
    }
}