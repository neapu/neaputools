#pragma once
#include <mutex>
#include <memory>
#include "NEString.h"
#include "NEIPAddress.h"
#include "NENetworkError.h"
#include "network_pub.h"
namespace neapu{
class Logger;
class NEAPU_NETWORK_EXPORT NetChannel{
    friend class TcpServer;
    friend class TcpClient;
    friend NEAPU_NETWORK_EXPORT Logger& operator<<(Logger& _logger, const NetChannel& _netclient);
public:
    ByteArray Read(size_t _len);
    ByteArray ReadAll();
    int Write(ByteArray _data);
    void Close();
    bool IsClosed();
    void SetUserData(std::shared_ptr<void*> _userData);
    std::shared_ptr<void*> GetUserData() const;
    IPAddress GetAddress() const { return m_address; }
    NetworkError GetError() const { return m_err; }
    void SetError(const NetworkError& _err) { m_err = _err; }
private:
    NetChannel(int _fd, const IPAddress& _addr)
        : m_fd(_fd)
        , m_address(_addr)
    {}
    void AppendData(ByteArray _data);
    void AppendData(const char* _data, size_t _len);
    void SetLastError(int _err, String _str);
private:
    int m_fd;
    ByteArray m_readBuffer;
    std::mutex m_bufferLock;
    std::mutex m_writeLock;
    std::shared_ptr<void*> m_userData;
    IPAddress m_address;
    NetworkError m_err;
};
NEAPU_NETWORK_EXPORT Logger& operator<<(Logger& _logger, const NetChannel& _netclient);
}