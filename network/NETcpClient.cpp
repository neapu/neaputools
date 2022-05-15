#include "NETcpClient.h"
#include <event2/event.h>
#include <functional>

#define BUF_SIZE 1024

int neapu::TcpClient::Connect(const IPAddress& _addr, bool _enableWriteCallback = false)
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

    if (_enableWriteCallback) {
        NetBase::AddSocket(m_fd, EV_READ | EV_WRITE);
    }
    else {
        NetBase::AddSocket(m_fd, EV_READ);
    }
    

    return 0;
}

void neapu::TcpClient::Send(const ByteArray& data)
{
    if (m_channel) {
        m_channel->Write(data);
    }
}

void neapu::TcpClient::Stop()
{
    NetBase::Stop();
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
                Stop();
            }
            break;
        }
        else if (readSize == 0) { //对面主动断开
            Stop();

            return;
        }
        else if (readSize < 0) { //发生错误
            //OnChannelError(m_channel);
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