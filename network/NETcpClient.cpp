#include "NETcpClient.h"
#include <event2/event.h>
#include <functional>
using namespace neapu;

#define BUF_SIZE 1024

int neapu::TcpClient::Connect(const IPAddress& _addr, bool _enableWriteCallback)
{
    m_address = _addr;
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup error");
        return 0;
    }
#endif
    
    if (_addr.IsIPv4()) {
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (_addr.IsIPv6()) {
        m_fd = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if (m_fd <= 0) {
        int err = evutil_socket_geterror(m_fd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_SOCKET_OPEN;
    }

    if (_addr.IsIPv4()) {
        sockaddr_in sin = { 0 };
        m_address.ToSockaddr(&sin);
        if (::connect(m_fd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_fd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_CONNECT;
        }
    } else if (_addr.IsIPv6()) {
        sockaddr_in6 sin = { 0 };
        m_address.ToSockaddr(&sin);
        if (::connect(m_fd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_fd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_CONNECT;
        }
    }
    

    int rc = evutil_make_socket_nonblocking(m_fd);
    if (0 != rc) {
        return ERROR_SOCKET_NONBLOCK;
    }

    NetBase::InitEvent(1);

    if (_enableWriteCallback) {
        NetBase::AddSocket(m_fd, EV_READ | EV_WRITE);
    }
    else {
        NetBase::AddSocket(m_fd, EV_READ);
    }
    
    m_channel = std::shared_ptr<neapu::NetChannel>(new neapu::NetChannel(m_fd, m_address));

    return 0;
}

int neapu::TcpClient::Send(const ByteArray& data)
{
    if (m_channel) {
        return m_channel->Write(data);
    }
    return 0;
}

TcpClient& neapu::TcpClient::OnWrite(std::function<void()> _cb)
{
    m_callback.onWrite = _cb;
    return *this;
}

TcpClient& neapu::TcpClient::OnRecvData(std::function<void(const ByteArray& data)> _cb)
{
    m_callback.onRecvData = _cb;
    return *this;
}

TcpClient& neapu::TcpClient::OnError(std::function<void(const NetworkError& _err)> _cb)
{
    m_callback.onError = _cb;
    return *this;
}

TcpClient& neapu::TcpClient::OnClosed(std::function<void()> _cb)
{
    m_callback.onClosed = _cb;
    return *this;
}

void neapu::TcpClient::OnWrite()
{
    if (m_callback.onWrite) {
        m_callback.onWrite();
    }
}

void neapu::TcpClient::OnRecvData(const ByteArray& data)
{
    if (m_callback.onRecvData) {
        m_callback.onRecvData(data);
    }
}

void neapu::TcpClient::OnError(const NetworkError& _err)
{
    if (m_callback.onError) {
        m_callback.onError(_err);
    }
}

void neapu::TcpClient::OnClosed()
{
    if (m_callback.onClosed) {
        m_callback.onClosed();
    }
}

void neapu::TcpClient::Stoped()
{
    if (m_fd) {
        evutil_closesocket(m_fd);
        m_fd = 0;
    }
}

void neapu::TcpClient::OnReadReady(int _fd)
{
    ByteArray data;
    char buf[BUF_SIZE];
    int readSize;
    for (;;) {
        readSize = recv(_fd, buf, BUF_SIZE, 0);
        if (readSize == EOF) { //接收完成
            int err = evutil_socket_geterror(_fd);
            if (err != 0 && err != 10035) { //对面意外掉线
                SetLastError(err, evutil_socket_error_to_string(err));
                OnError(m_err);
                Stop();
            }
            break;
        }
        else if (readSize == 0) { //对面主动断开
            Stop();
            OnClosed();
            return;
        }
        else if (readSize < 0) { //发生错误
            int err = evutil_socket_geterror(_fd);
            SetLastError(err, evutil_socket_error_to_string(err));
            OnError(m_err);
            Stop();
            return;
        }
        data.append(buf, readSize);
    }
    OnRecvData(data);
    return;
}

void neapu::TcpClient::OnSignalReady(int _signal)
{
    Stop();
}