#pragma once
#include <mutex>
#include <memory>
#include "base/NEString.h"
#include "network/NEIPAddress.h"
#include "network/NENetworkError.h"
#include "network/network_pub.h"
#include <base/NEByteArray.h>
#include <network/NESocketBase.h>

namespace neapu {
class NetChannel;
}
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, const neapu::NetChannel& _netclient);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetChannel& _netclient);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetChannel&& _netclient);
namespace neapu {
class Logger;
class NEAPU_NETWORK_EXPORT NetChannel : public SocketBase {
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

    NetChannel(SOCKET_FD _fd, const IPAddress& _addr)
        : SocketBase(_fd)
        , m_address(_addr)
    {}

    using SocketBase::SetLastError;
    void SetLastError(int _err, String _str)
    {
        m_err.code = _err;
        m_err.str = _str;
    }

protected:
    std::mutex m_bufferLock;
    std::mutex m_writeLock;
    std::shared_ptr<void*> m_userData;
    IPAddress m_address;
};

} // namespace neapu
