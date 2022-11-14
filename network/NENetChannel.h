#pragma once
#include <mutex>
#include <memory>
#include "NEString.h"
#include "NEIPAddress.h"
#include "NENetworkError.h"
#include "network_pub.h"
#include <NEByteArray.h>

namespace neapu {
class NetChannel;
}
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, const neapu::NetChannel& _netclient);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetChannel& _netclient);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetChannel&& _netclient);
namespace neapu {
class Logger;
class NEAPU_NETWORK_EXPORT NetChannel {
    friend NEAPU_NETWORK_EXPORT neapu::Logger& ::operator<<(neapu::Logger& _logger, const neapu::NetChannel& _netclient);
    friend NEAPU_NETWORK_EXPORT neapu::Logger& ::operator<<(neapu::Logger& _logger, neapu::NetChannel& _netclient);
    friend NEAPU_NETWORK_EXPORT neapu::Logger& ::operator<<(neapu::Logger& _logger, neapu::NetChannel&& _netclient);

public:
    ByteArray Read(size_t _len);
    ByteArray ReadAll();
    int Write(const ByteArray& _data);
    void Close();
    bool IsClosed();
    void SetUserData(std::shared_ptr<void*> _userData);
    std::shared_ptr<void*> GetUserData() const;
    IPAddress GetAddress() const
    {
        return m_address;
    }
    NetworkError GetError() const
    {
        return m_err;
    }
    void SetError(const NetworkError& _err)
    {
        m_err = _err;
    }
    NetChannel(int _fd, const IPAddress& _addr)
        : m_fd(_fd)
        , m_address(_addr)
    {}
    void SetLastError(int _err, String _str);

private:
    int m_fd;
    std::mutex m_bufferLock;
    std::mutex m_writeLock;
    std::shared_ptr<void*> m_userData;
    IPAddress m_address;
    NetworkError m_err;
};

} // namespace neapu
