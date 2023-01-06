#ifndef __NEUDPSOCKET_H__
#define __NEUDPSOCKET_H__

#include "network/network_pub.h"
#include "base/NEByteArray.h"
#include "network/NESocketBase.h"
#include "network/NEIPAddress.h"
#include <memory>
#include <utility>
namespace neapu {
class NEAPU_NETWORK_EXPORT UdpSocket : public SocketBase {
public:
    static std::shared_ptr<UdpSocket> MakeUdpSocket();
    int Init(const IPAddress& _bindAddr);
    int SendTo(const ByteArray& _data, const IPAddress& _addr);
    std::pair<ByteArray, IPAddress> RecvFrom(size_t _len, int _timeout = 3000);
    IPAddress GetBindAddr() { return m_bindAddr; }

protected:
    IPAddress m_bindAddr;
};
using UdpSocketPtr = std::shared_ptr<UdpSocket>;
} // namespace neapu
#endif // __NEUDPSOCKET_H__