#ifndef __NEUDPSOCKET_H__
#define __NEUDPSOCKET_H__

#include "NEByteArray.h"
#include "NESocketBase.h"
#include "network/NEIPAddress.h"
#include <utility>
namespace neapu {
class UdpSocket : public SocketBase {
public:
    int Init(const IPAddress& _bindAddr);
    int SendTo(const ByteArray& _data, const IPAddress& _addr);
    std::pair<ByteArray, IPAddress> RecvFrom(size_t _len, int _timeout = 3000);
    IPAddress GetBindAddr() { return m_bindAddr; }

protected:
    IPAddress m_bindAddr;
};
} // namespace neapu
#endif // __NEUDPSOCKET_H__