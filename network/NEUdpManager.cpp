#include "NEUdpManager.h"
#include "network/NEUdpSocket.h"
#include <memory>

using namespace neapu;

UdpManager& UdpManager::OnReceive(SocketCallback _cb)
{
    m_callback = _cb;
    return *this;
}

void UdpManager::OnReceive(UdpSocketPtr _sock)
{
    if (m_callback) {
        m_callback(_sock);
    }
}

void UdpManager::OnReadEvent(SocketBasePtr _sock)
{
    OnReceive(std::static_pointer_cast<UdpSocket>(_sock));
}