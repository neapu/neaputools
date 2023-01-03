#include "NETcpClient2.h"
#include "NEByteArray.h"
#include "network/NENetworkError.h"
#include "network/NETcpSocket.h"
#include <sys/socket.h>

using namespace neapu;

int TcpClient2::Connect(const IPAddress& _addr)
{
    m_address = _addr;
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup error");
        return -1;
    }
#endif
    if (_addr.IsIPv4()) {
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
    } else {
        m_fd = socket(AF_INET6, SOCK_STREAM, 0);
    }

    if (m_fd <= 0) {
        int err = GetSocketError(m_fd);
        SetLastError(err, GetErrorString(err));
        m_fd = 0;
        return ERROR_SOCKET_OPEN;
    }

    sockaddr_storage sin;
    _addr.ToSockaddr(&sin);
    int ret = connect(m_fd, (sockaddr*)&sin, _addr.SockaddrLen());
    if (ret < 0) {
        int err = GetSocketError(m_fd);
        SetLastError(err, GetErrorString(err));
        Close();
        return ERROR_CONNECT;
    }
    m_connected = true;
    return 0;
}

int TcpClient2::Send(const ByteArray& data)
{
    return TcpSocket::Write(data);
}

int TcpClient2::Send(const char* _data, size_t _len)
{
    ByteArray data((const unsigned char*)_data, _len);
    return TcpSocket::Write(data);
}

bool TcpClient2::IsConnected()
{
    return m_connected;
}

void TcpClient2::Close()
{
    m_connected = false;
    return TcpSocket::Close();
}

ByteArray TcpClient2::Receive(size_t _len, int _timeout)
{
    if (m_fd <= 0) {
        return ByteArray();
    }
    if (m_nonBlock == false) {
#ifndef _WIN32
        struct timeval timeout;
        timeout.tv_sec = _timeout / 1000;
        timeout.tv_usec = _timeout % 1000;
#else
        int timeout = _timeout;
#endif
        setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    }

    return TcpSocket::Read(_len);
}