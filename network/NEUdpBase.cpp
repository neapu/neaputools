#include "NEUdpBase.h"
#include <signal.h>
#include <event2/event.h>
#include <NEByteArray.h>

using namespace neapu;

constexpr auto BUFFER_SIZE = 65536; //UDP协议报文长度

neapu::UdpBase::~UdpBase() noexcept
{
    Stop();
}

neapu::UdpBase::UdpBase(UdpBase&& _ub) noexcept
{
}

int neapu::UdpBase::Init(int _threads, const IPAddress& _addr, bool _enableWriteCallback)
{
    if (m_udpFd) {
        Stop();
    }
    m_address = _addr;
    m_enableWriteCallback = _enableWriteCallback;
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup error");
        return 0;
    }
#endif // WIN32


    //创建套接字
    if (m_address.IsIPv4()) {
        m_udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else if (m_address.IsIPv6()) {
        m_udpFd = socket(AF_INET6, SOCK_DGRAM, 0);
    }
    if (m_udpFd <= 0) {
        int err = evutil_socket_geterror(m_udpFd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_SOCKET_OPEN;
    }

    int rc = evutil_make_socket_nonblocking(m_udpFd);
    if (0 != rc) {
        return ERROR_SOCKET_NONBLOCK;
    }

    if (m_address.IsIPv4()) {
        sockaddr_in sin = { 0 };
        m_address.ToSockaddr(&sin);

        if (bind(m_udpFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_udpFd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_BIND;
        }
    }
    else if (m_address.IsIPv6()) {
        sockaddr_in6 sin = { 0 };
        m_address.ToSockaddr(&sin);

        if (bind(m_udpFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_udpFd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_BIND;
        }
    }

    rc = NetBase::InitEvent(_threads);
    if (rc < 0) {
        Stop();
        return rc;
    }

    short events = EV_READ;
    if (_enableWriteCallback) {
        events |= EV_WRITE;
    }
    rc = AddSocket(m_udpFd, events);
    if (rc < 0) {
        Stop();
        return rc;
    }

    rc = AddSignal(SIGINT);
    if (rc < 0) {
        Stop();
        return rc;
    }
    
    return rc;
}

int neapu::UdpBase::Send(const ByteArray& _data, const IPAddress& _addr)
{
    if (!m_udpFd) {
#ifdef WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            perror("WSAStartup error");
            return 0;
        }
#endif // WIN32
        //创建套接字
        if (_addr.IsIPv4()) {
            m_udpFd = socket(AF_INET, SOCK_DGRAM, 0);
        }
        else if (_addr.IsIPv6()) {
            m_udpFd = socket(AF_INET6, SOCK_DGRAM, 0);
        }

        int rc = evutil_make_socket_nonblocking(m_udpFd);
        if (0 != rc) {
            return ERROR_SOCKET_NONBLOCK;
        }
    }
    int rc = 0;
    if (_addr.IsIPv4()) {
        sockaddr_in sin = {0};
        _addr.ToSockaddr(&sin);
        rc = ::sendto(m_udpFd, _data.Data(), _data.Length(), 0, (sockaddr*)&sin, sizeof(sin));
    }
    else if (_addr.IsIPv6()) {
        sockaddr_in6 sin = {0};
        _addr.ToSockaddr(&sin);
        rc = ::sendto(m_udpFd, _data.Data(), _data.Length(), 0, (sockaddr*)&sin, sizeof(sin));
    }
    if (rc<0) {
        int err = evutil_socket_geterror(m_udpFd);
        SetLastError(err, evutil_socket_error_to_string(err));
    }
    return rc;
}

void neapu::UdpBase::Stoped()
{
    if (m_udpFd) {
        evutil_closesocket(m_udpFd);
        m_udpFd = 0;
    }
}

neapu::UdpBase& neapu::UdpBase::OnRecvData(std::function<void(const ByteArray&, const IPAddress&)> _cb)
{
    m_callback.recvDataCallback = _cb;
    return *this;
}

UdpBase& neapu::UdpBase::OnWriteReady(std::function<void()> _cb)
{
    m_callback.writableCallback = _cb;
    return *this;
}

void neapu::UdpBase::OnRecvData(const ByteArray& _data, const IPAddress& _addr)
{
    if (m_callback.recvDataCallback) {
        m_callback.recvDataCallback(_data, _addr);
    }
}

void neapu::UdpBase::OnWriteReady(int /*_fd*/)
{
    if (m_callback.writableCallback) {
        m_callback.writableCallback();
    }
}

void neapu::UdpBase::OnReadReady(int _fd)
{
    ByteArray data;
    int readSize = 0;
    IPAddress addr;
    std::unique_ptr<char> buf(new char[BUFFER_SIZE]);
    if (m_address.IsIPv4()) {
        sockaddr_in sin;
        socklen_t sinLen = sizeof(sin);
        readSize = ::recvfrom(_fd, buf.get(), BUFFER_SIZE, 0, (sockaddr*)&sin, &sinLen);
        addr = IPAddress::MakeAddress(sin);
    }
    else if (m_address.IsIPv6()) {
        sockaddr_in6 sin;
        socklen_t sinLen = sizeof(sin);
        readSize = ::recvfrom(_fd, buf.get(), BUFFER_SIZE, 0, (sockaddr*)&sin, &sinLen);
        addr = IPAddress::MakeAddress(sin);
    }
    data.Append(buf.get(), static_cast<size_t>(readSize));
    OnRecvData(data, addr);
}

void neapu::UdpBase::OnSignalReady(int _signal)
{
    if (_signal == SIGINT) {
        this->Stop();
    }
}
