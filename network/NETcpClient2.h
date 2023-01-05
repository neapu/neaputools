#ifndef __NETCPCLIENT2_H__
#define __NETCPCLIENT2_H__

#include "network/network_pub.h"
#include "NEByteArray.h"
#include "NENetChannel.h"
#include "network/NEIPAddress.h"
#include "network/NETcpSocket.h"
#include <cstddef>
namespace neapu {
class NEAPU_NETWORK_EXPORT TcpClient2 : public NetChannel {
public:
    TcpClient2()
        : NetChannel(0, IPAddress()) {}
    int Connect(const IPAddress& _addr);
    int Send(const ByteArray& data);
    int Send(const char* _data, size_t _len);
    bool IsConnected();
    void Close();
    ByteArray Receive(size_t _len = (size_t)(-1), int _timeout = 3000);
    void SetNonBlock(bool nonBlock) { m_nonBlock = nonBlock; }

private:
    bool m_nonBlock = false;
    bool m_connected = false;
};
} // namespace neapu
#endif // __NETCPCLIENT2_H__