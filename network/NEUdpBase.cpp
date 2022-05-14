#include "NEUdpBase.h"
#include <event2/event.h>
#include <signal.h>
using namespace neapu;

constexpr auto BUFFER_SIZE = 65536; //UDP协议报文长度

static void neapu::cbSocketEvent(evutil_socket_t fd, short events, void* user_data)
{
    UdpBase* base = static_cast<UdpBase*>(user_data);
    if (events & EV_READ) {
#ifdef _WIN32
        base->OnReadReady(fd);
#else
        base->m_readQueue->enqueue(fd);
#endif
    }
    if (events & EV_WRITE) {
        base->OnWritable();
        if (base->m_callback.writableCallback) {
            base->m_callback.writableCallback();
        }
    }
}

void neapu::cbSigInt(evutil_socket_t sig, short events, void* user_data) 
{
    UdpBase* nb = static_cast<UdpBase*>(user_data);
    nb->OnSignalInt();
}

int neapu::UdpBase::Init(int _port, int _threads, IPAddress::Type _serverType, bool _enableWriteCallback)
{
    m_port = _port;
    m_ipType = _serverType;
    m_enableWriteCallback = _enableWriteCallback;
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup error");
        return 0;
    }
#endif // WIN32

    m_eb = event_base_new();
    if (!m_eb) {
        return ERROR_EVENT_BASE;
    }

    //处理SIGINT
    auto evSigInt = evsignal_new(m_eb, SIGINT, neapu::cbSigInt, this);
    if (!evSigInt || event_add(evSigInt, nullptr) < 0) {
        return ERROR_EVENT_ADD;
    }

#ifndef _WIN32
    //初始化线程池
    m_running = 0;
    m_threadPoll.Init(_threads, std::bind(&UdpBase::WorkThread, this));
#endif // !_WIN32

    //创建套接字
    if (_serverType == IPAddress::Type::IPv4) {
        m_udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else if (_serverType == IPAddress::Type::IPv6) {
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

    if (_serverType == IPAddress::Type::IPv4) {
        sockaddr_in sin = { 0 };
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(_port);

        if (bind(m_udpFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_udpFd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_BIND;
        }
    }
    else if (_serverType == IPAddress::Type::IPv6) {
        sockaddr_in6 sin = { 0 };
        sin.sin6_family = AF_INET6;
        sin.sin6_addr = in6addr_any;
        sin.sin6_port = htons(_port);

        if (bind(m_udpFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_udpFd);
            SetLastError(err, evutil_socket_error_to_string(err));
            return ERROR_BIND;
        }
    }

    short events = EV_READ | EV_PERSIST | EV_ET;
    if (_enableWriteCallback) {
        events |= EV_WRITE;
    }
    auto evListen = event_new(m_eb, m_udpFd, events, neapu::cbSocketEvent, this);
    if (!evListen || event_add(evListen, nullptr) < 0) {
        return ERROR_EVENT_ADD;
    }
    
    
    return 0;
}

int neapu::UdpBase::Send(const ByteArray& _data, const IPAddress& _addr)
{
    if (_addr.IsIPv4()) {
        sockaddr_in sin;
        _addr.ToSockaddr(&sin);
        return ::sendto(m_udpFd, _data.data(), _data.length(), 0, (sockaddr*)&sin, sizeof(sin));
    }
    else if (_addr.IsIPv6()) {
        sockaddr_in6 sin;
        _addr.ToSockaddr(&sin);
        return ::sendto(m_udpFd, _data.data(), _data.length(), 0, (sockaddr*)&sin, sizeof(sin));
    }
    return 0;
}

int neapu::UdpBase::Run()
{
    event_base_dispatch(m_eb);
    event_base_free(m_eb);
    evutil_closesocket(m_udpFd);
    return 0;
}

neapu::UdpBase& neapu::UdpBase::OnRecvData(std::function<void(const ByteArray&, const IPAddress&)> _cb)
{
    m_callback.recvDataCallback = _cb;
    return *this;
}

UdpBase& neapu::UdpBase::OnWritable(std::function<void()> _cb)
{
    m_callback.writableCallback = _cb;
    return *this;
}

void neapu::UdpBase::SetLastError(int _err, String _errstr)
{
    m_err = _err;
    m_errstr = _errstr;
}

#ifndef _WIN32
void neapu::UdpBase::WorkThread()
{
    while (m_running) {
        int _fd;
        if (m_readQueue.dequeue(_fd))
        {
            OnReadReady(_fd);
        }
    }
}
#endif

void neapu::UdpBase::OnReadReady(int _fd)
{
    ByteArray data;
    int readSize = 0;
    IPAddress addr;
    std::unique_ptr<char> buf(new char[BUFFER_SIZE]);
    if (m_ipType == IPAddress::Type::IPv4) {
        sockaddr_in sin;
        int sinLen = sizeof(sin);
        readSize = ::recvfrom(_fd, buf.get(), BUFFER_SIZE, 0, (sockaddr*)&sin, &sinLen);
        addr = IPAddress::MakeAddress(sin);
    }
    else if (m_ipType == IPAddress::Type::IPv6) {
        sockaddr_in6 sin;
        int sinLen = sizeof(sin);
        readSize = ::recvfrom(_fd, buf.get(), BUFFER_SIZE, 0, (sockaddr*)&sin, &sinLen);
        addr = IPAddress::MakeAddress(sin);
    }
    data.append(buf.get(), static_cast<size_t>(readSize));
    OnRecvData(data, addr);
    if (m_callback.recvDataCallback) {
        m_callback.recvDataCallback(data, addr);
    }
}

void neapu::UdpBase::OnSignalInt()
{
    event_base_loopbreak(m_eb);
#ifndef _WIN32
    m_running = false;
    m_threadPoll.Join();
#endif // !A

}
