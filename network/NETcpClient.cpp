#include "NETcpClient.h"
#include <event2/event.h>
#include <functional>
#include <csignal>
using namespace neapu;

#define BUF_SIZE 1024
#define WRITE_SIZE 1024
#ifdef _WIN32
#define RECV_EOF_EN 10035
#else
#define RECV_EOF_EN EAGAIN
#endif

int neapu::TcpClient::Connect(const IPAddress& _addr)
{
    m_address = _addr;
    int rc = NetBase::Init(1);
    if (rc < 0) {
        return rc;
    }
    
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
    

    rc = evutil_make_socket_nonblocking(m_fd);
    if (0 != rc) {
        return ERROR_SOCKET_NONBLOCK;
    }

    
    if (AddSocket(m_fd, EventType::Read, true) == EmptyHandle) {
        evutil_closesocket(m_fd);
        m_fd = 0;
        return ERROR_ADD_SOCKET;
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

int neapu::TcpClient::Send(const char* _data, size_t _len)
{
    return Send(ByteArray(_data, _len));
}

TcpClient& neapu::TcpClient::OnWrite(std::function<void()> _cb)
{
    m_callback.onWrite = _cb;
    return *this;
}

TcpClient& neapu::TcpClient::OnRecvData(std::function<void(std::shared_ptr<NetChannel> _channel)> _cb)
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

void neapu::TcpClient::OnRecvData(std::shared_ptr<NetChannel> _channel)
{
    if (m_callback.onRecvData) {
        m_callback.onRecvData(_channel);
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

void neapu::TcpClient::OnEventLoopStoped()
{
    NetBase::OnEventLoopStoped();
    if (m_fd) {
        evutil_closesocket(m_fd);
        m_fd = 0;
    }
}

void neapu::TcpClient::OnReadReady(evutil_socket_t _socket, EventHandle _handle)
{
    int _fd = static_cast<int>(_socket);
    if (_fd != m_fd)return;
    int rc = recv(_fd, nullptr, 0, MSG_PEEK);
    int err = evutil_socket_geterror(_fd);
    if (err == RECV_EOF_EN) {//连接正常关闭
        OnClosed();
        ReleaseEvent(_handle);
        Stop();
        return;
    }
    else if (err != 0) {
        SetLastError(err, evutil_socket_error_to_string(err));
        m_channel->SetError(m_err);
        OnError(m_err);
        ReleaseEvent(_handle);
        Stop();
        return;
    }
    else {
        OnRecvData(m_channel);
    }

    if (m_channel->IsClosed()) { //如果链接被关闭了就清理
        ReleaseEvent(_handle);
        Stop();
    }
}

int neapu::TcpClientSync::Connect(const IPAddress& _addr)
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
    }
    else if (_addr.IsIPv6()) {
        m_fd = socket(AF_INET6, SOCK_STREAM, 0);
    }
    if (m_fd <= 0) {
        int err = evutil_socket_geterror(m_fd);
        SetLastError(err, evutil_socket_error_to_string(err));
        m_fd = 0;
        return ERROR_SOCKET_OPEN;
    }

    if (_addr.IsIPv4()) {
        sockaddr_in sin = { 0 };
        m_address.ToSockaddr(&sin);
        if (::connect(m_fd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_fd);
            SetLastError(err, evutil_socket_error_to_string(err));
            m_fd = 0;
            return ERROR_CONNECT;
        }
    }
    else if (_addr.IsIPv6()) {
        sockaddr_in6 sin = { 0 };
        m_address.ToSockaddr(&sin);
        if (::connect(m_fd, (sockaddr*)&sin, sizeof(sin)) < 0) {
            int err = evutil_socket_geterror(m_fd);
            SetLastError(err, evutil_socket_error_to_string(err));
            m_fd = 0;
            return ERROR_CONNECT;
        }
    }

    return 0;
}

int neapu::TcpClientSync::Send(const ByteArray& _data)
{
    std::unique_lock<std::mutex> locker(m_writeLock);
    if (!m_fd)return -1;
    const char* ptr = _data.Data();
    size_t writeSize = 0;
    size_t offset = 0;
    int count = 0;
    while (offset < _data.Length()) {
        writeSize = _data.Length() - offset > WRITE_SIZE ? WRITE_SIZE : _data.Length() - offset;
        count = ::send(m_fd, ptr + offset, writeSize, 0);
        if (count <= 0) {
            int err = evutil_socket_geterror(m_fd);
            m_err.code = err;
            m_err.str = evutil_socket_error_to_string(err);
            return count;
        }
        offset += writeSize;
    }
    return count;
}

int neapu::TcpClientSync::Send(const char* _data, size_t _len)
{
    return Send(ByteArray(_data, _len));
}

ByteArray neapu::TcpClientSync::Recv(size_t _len, int _timeout)
{
    ByteArray rst;
    if (!m_fd)return rst;

#ifndef _WIN32
    struct timeval timeout;
    timeout.tv_sec = _timeout / 1000;
    timeout.tv_usec = _timeout % 1000;
#else
    int timeout = _timeout;
#endif
    setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    char buf[BUF_SIZE];
    int readSize = 0;
    size_t readCount = 0;
    while (readCount < _len) {
        readSize = ::recv(m_fd, buf, BUF_SIZE, 0);
        if (readSize <= 0) {
            break;
        }
        readCount += readSize;
        rst.Append(buf, readSize);
        if (readSize < BUF_SIZE) {
            break;
        }
    }
    return rst;
}

void neapu::TcpClientSync::Close()
{
    if (m_fd) {
        evutil_closesocket(m_fd);
        m_fd = 0;
    }
}

void neapu::TcpClientSync::SetLastError(int _err, String _errstr)
{
    m_err.code = _err;
    m_err.str = _errstr;
}