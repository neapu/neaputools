#ifndef __NESOCKET_H__
#define __NESOCKET_H__

#include "network/NEEventBase2.h"
#include "network/NENetworkError.h"
#include "network/network_pub.h"
#include <memory>

namespace neapu {
class SocketBase {
public:
    SocketBase(SOCKET_FD _fd = 0)
        : m_fd(_fd) {}
    void SetSocket(SOCKET_FD _fd) { m_fd = _fd; }
    SOCKET_FD GetSocket() { return m_fd; }
    void SetNonBlock(bool _nonblock) { m_nonBlock = _nonblock; }
    void SetLastError() { m_err.SetLastError(); }
    void SetLastError(NetworkError _err) { m_err = _err; }
    NetworkError GetLastError() { return m_err; }

protected:
    SOCKET_FD m_fd = 0;
    bool m_ipv6 = false;
    bool m_nonBlock = false;
    NetworkError m_err;
};
using SocketBasePtr = std::shared_ptr<SocketBase>;
} // namespace neapu
#endif // __NESOCKET_H__