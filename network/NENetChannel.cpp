#include "NENetChannel.h"
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "NELogger.h"
using namespace neapu;
#define WRITE_SIZE 1024

ByteArray NetChannel::Read(size_t _len)
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    ByteArray rst;
    rst=m_readBuffer.Left(_len);
    if(m_readBuffer.length()<_len) {
        m_readBuffer = m_readBuffer.Middle(_len, m_readBuffer.length());
    }
    return rst;
}

ByteArray NetChannel::ReadAll()
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    ByteArray rst = m_readBuffer;
    m_readBuffer.clear();
    return rst;
}

void NetChannel::Write(ByteArray _data)
{
    std::unique_lock<std::mutex> locker(m_writeLock);
    const char* ptr = _data.data();
    size_t writeSize = 0;
    size_t offset = 0;
    while(offset<_data.length()) {
        writeSize = _data.length()-offset>WRITE_SIZE?WRITE_SIZE:_data.length()-offset;
        ::send(m_fd, ptr+offset, writeSize, 0);
        offset+=writeSize;
    }
}

void NetChannel::Close()
{
    if(m_fd){
#ifdef WIN32
        closesocket(m_fd);
#else
        close(m_fd);
#endif // WIN32
        m_fd=0;
    }
}

bool NetChannel::IsClosed()
{
    return m_fd==0;
}

void NetChannel::SetUserData(std::shared_ptr<void*> _userData)
{
    m_userData = _userData;
}

std::shared_ptr<void*> NetChannel::GetUserData() const
{
    return m_userData;
}

void NetChannel::AppendData(ByteArray _data)
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    m_readBuffer.append(_data);
}

void NetChannel::AppendData(const char* _data, size_t _len)
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    m_readBuffer.append(_data, _len);
}

void NetChannel::SetLastError(int _err, String _str)
{
    m_err = _err;
    m_errString = _str;
}

NEAPU_NETWORK_EXPORT Logger& neapu::operator<<(Logger& _logger, const NetChannel& _NetChannel)
{
    return _logger<<"[Class NetChannel][fd:"<<_NetChannel.m_fd<<"]";
}