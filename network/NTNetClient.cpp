#include "NTNetClient.h"
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "NTLogger.h"
using namespace neapu;
#define WRITE_SIZE 1024

ByteArray NetClient::Read(size_t _len)
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    ByteArray rst;
    rst=m_readBuffer.Left(_len);
    if(m_readBuffer.length()<_len) {
        m_readBuffer = m_readBuffer.Middle(_len, m_readBuffer.length());
    }
    return rst;
}

ByteArray NetClient::ReadAll()
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    ByteArray rst = m_readBuffer;
    m_readBuffer.clear();
    return rst;
}

void NetClient::Write(ByteArray _data)
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

void NetClient::Close()
{
    if(m_fd){
        close(m_fd);
        m_fd=0;
    }
}

bool NetClient::IsClosed()
{
    return m_fd==0;
}

void NetClient::SetUserData(std::shared_ptr<void*> _userData)
{
    m_userData = _userData;
}

std::shared_ptr<void*> NetClient::GetUserData()
{
    return m_userData;
}

void NetClient::AppendData(ByteArray _data)
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    m_readBuffer.append(_data);
}

unsigned int NetClient::GetIPv4()
{
    return m_ipv4;
}

String NetClient::GetIPv4String()
{
    return String(); //todo 
}

void NetClient::AppendData(const char* _data, size_t _len)
{
    std::unique_lock<std::mutex> locker(m_bufferLock);
    m_readBuffer.append(_data, _len);
}

void NetClient::SetLastError(int _err, String _str)
{
    m_err = _err;
    m_errString = _str;
}

Logger& neapu::operator<<(Logger& _logger, const NetClient& _netclient)
{
    return _logger<<"[Class NetClient][fd:"<<_netclient.m_fd<<"]";
}