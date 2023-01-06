#ifndef __NEUDPMANAGER_H__
#define __NEUDPMANAGER_H__

#include "network/NESocketBase.h"
#include "network/NESocketManager.h"
#include "network/NEUdpSocket.h"
namespace neapu{
class UdpManager:public SocketManager{
public:
    using SocketCallback = std::function<void(UdpSocketPtr)>;

    UdpManager& OnReceive(SocketCallback _cb);

protected:
    virtual void OnReceive(UdpSocketPtr _sock);

private:
    virtual void OnReadEvent(SocketBasePtr _sock) override;

protected:
    SocketCallback m_callback;
};
}
#endif // __NEUDPMANAGER_H__