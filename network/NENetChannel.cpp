#include "NENetChannel.h"
#include "network/network_pub.h"
#ifdef USE_LIBEVENT
#include <event2/event.h>
#endif
#include "NELogger.h"

using namespace neapu;
#define WRITE_SIZE 1024
#define BUF_SIZE 1024

ByteArray NetChannel::Read(size_t _len)
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    ByteArray rst;
    if (!m_fd) return rst;
    unsigned char buf[BUF_SIZE];
    int readSize = 0;
    size_t readCount = 0;
    while (readCount < _len) {
        readSize = BUF_SIZE < _len - readCount ? BUF_SIZE : _len - readCount;
#ifdef WIN32
        readSize = recv(m_fd, reinterpret_cast<char*>(buf), readSize, 0);
#else
        readSize = recv(m_fd, buf, readSize, 0);
#endif // WIN32
        if (readSize <= 0) break;
        rst.Append(buf, readSize);
        readCount += readSize;
    }
    return rst;
}

ByteArray NetChannel::ReadAll()
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    ByteArray rst;
    if (!m_fd) return rst;
    unsigned char buf[BUF_SIZE];
    int readSize = 0;
    while (true) {
#ifdef WIN32
        readSize = recv(m_fd, reinterpret_cast<char*>(buf), BUF_SIZE, 0);
#else
        readSize = recv(m_fd, buf, BUF_SIZE, 0);
#endif // WIN32
        if (readSize <= 0) break;
        rst.Append(buf, readSize);
    }
    return rst;
}

int NetChannel::Write(const ByteArray& _data)
{
    std::unique_lock<std::mutex> locker(m_writeLock);
    if (!m_fd) return -1;
    const unsigned char* ptr = _data.Data();
    size_t writeSize = 0;
    size_t offset = 0;
    int count = 0;
    while (offset < _data.Length()) {
        writeSize = _data.Length() - offset > WRITE_SIZE ? WRITE_SIZE : _data.Length() - offset;
#ifdef WIN32
        count = ::send(m_fd, reinterpret_cast<const char*>(ptr + offset), writeSize, 0);
#else
        count = ::send(m_fd, ptr + offset, writeSize, 0);
#endif // WIN32
        if (count <= 0) {
            int err = GetSocketError(m_fd);
            m_err.code = err;
            m_err.str = GetErrorString(err);
            return count;
        }
        offset += writeSize;
    }
    return count;
}

void NetChannel::Close()
{
    if (m_fd) {
#ifdef WIN32

#else
        close(m_fd);
#endif
        m_fd = 0;
    }
}

bool NetChannel::IsClosed()
{
    return m_fd == 0;
}

void NetChannel::SetUserData(std::shared_ptr<void*> _userData)
{
    m_userData = _userData;
}

std::shared_ptr<void*> NetChannel::GetUserData() const
{
    return m_userData;
}

NEAPU_NETWORK_EXPORT Logger& operator<<(Logger& _logger, const NetChannel& _netclient)
{
    return _logger << "[Class NetChannel][fd:" << _netclient.m_fd << "][address:" << _netclient.GetAddress() << "]";
}

NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetChannel& _netclient)
{
    return _logger << "[Class NetChannel][fd:" << _netclient.m_fd << "][address:" << _netclient.GetAddress() << "]";
}

NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetChannel&& _netclient)
{
    return _logger << "[Class NetChannel][fd:" << _netclient.m_fd << "][address:" << _netclient.GetAddress() << "]";
}