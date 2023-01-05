#include "NEUdpSocket.h"
#include "NEByteArray.h"
#include "logger/logger.h"
#include "network/NEEventBase2.h"
#include "network/NEIPAddress.h"
#include "network/NENetworkError.h"
#include <cstddef>
#ifdef WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#endif

using namespace neapu;

int UdpSocket::Init(const IPAddress& bindAddr)
{
    SOCKET_FD fd = -1;
    int ret = 0;
    if (bindAddr.IsIPv4()) {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    } else {
        fd = socket(AF_INET6, SOCK_DGRAM, 0);
        m_ipv6 = true;
    }
    if (fd <= 0) {
        return ERROR_SOCKET_OPEN;
    }

    m_bindAddr = bindAddr;
    if (!bindAddr.IsEmptyAddr() || bindAddr.Port() != 0) {
        sockaddr_storage sin;
        bindAddr.ToSockaddr(&sin);
        ret = bind(fd, (sockaddr*)&sin, bindAddr.SockaddrLen());
        if (ret < 0) {
#ifdef _WIN32
            closesocket(fd);
#else
            close(fd);
#endif
            return ERROR_BIND;
        }
    }

    SetSocket(fd);
    return 0;
}

int UdpSocket::SendTo(const ByteArray& _data, const IPAddress& _addr)
{
    sockaddr_storage sin;
    _addr.ToSockaddr(&sin);
#ifdef WIN32
    int ret = ::sendto(m_fd, (char*)_data.Data(), _data.Length(), 0, (sockaddr*)&sin, _addr.SockaddrLen());
#else
    int ret = ::sendto(m_fd, _data.Data(), _data.Length(), 0, (sockaddr*)&sin, _addr.SockaddrLen());
#endif
    return ret;
}

std::pair<ByteArray, IPAddress> UdpSocket::RecvFrom(size_t _len, int _timeout)
{
    ByteArray rst;
    IPAddress addr;
    if (m_fd <= 0) {
        return {rst, addr};
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

    sockaddr_storage sin;
    socklen_t sockLen = 0;
    if (m_ipv6) {
        sockLen = sizeof(sockaddr_in6);
    } else {
        sockLen = sizeof(sockaddr_in);
    }
    char* buf = new char[_len];
    size_t ret = ::recvfrom(m_fd, buf, _len, 0, (struct sockaddr*)&sin, &sockLen);
    rst.Append((unsigned char*)buf, ret);
    addr = IPAddress::MakeAddress(sin);
    return {rst, addr};
}
