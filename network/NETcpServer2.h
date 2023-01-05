#ifndef __NETCPSERVER2_H__
#define __NETCPSERVER2_H__

#include "network/network_pub.h"
#include "NENetworkError.h"
#include "network/NEEventBase2.h"
#include "network/NEIPAddress.h"
#include "network/NETcpSocket.h"
#include <map>
#include <mutex>
namespace neapu {
class NEAPU_NETWORK_EXPORT TcpServer2 : public EventBase2 {
public:
    using TcpServerCallback = std::function<void(TcpSocketPtr _socket)>;
    TcpServer2() {}
    int Init(const IPAddress& _addr, int _threadNum = 1);
    int Listen();
    void Stop();

    TcpServer2& OnReceive(TcpServerCallback _cb);
    TcpServer2& OnAccepted(TcpServerCallback _cb);
    TcpServer2& OnClosed(TcpServerCallback _cb);
    TcpServer2& OnError(TcpServerCallback _cb);

    IPAddress GetAddress() const { return m_address; }
    NetworkError GetError() const { return m_err; }

protected:
    virtual void OnReceive(TcpSocketPtr _socket);
    virtual void OnAccepted(TcpSocketPtr _socket);
    virtual void OnClose(TcpSocketPtr _socket);
    virtual void OnError(TcpSocketPtr _socket);

    void SetLastError();

private:
    virtual void OnSocketTriggerCallback(SOCKET_FD _fd, EventBase2::EventType _events);
    void OnListenerAccept();
    void OnClientReadReady(SOCKET_FD _fd);
    TcpSocketPtr AddClient(SOCKET_FD _fd, const IPAddress& _addr);
    void ReleaseClient(SOCKET_FD _fd);
    TcpSocketPtr GetClient(SOCKET_FD _fd);

protected:
    IPAddress m_address;
    SOCKET_FD m_listenFd = 0;
    NetworkError m_err;

    struct {
        TcpServerCallback onReceive;
        TcpServerCallback onAccepted;
        TcpServerCallback onClose;
        TcpServerCallback onError;
    } m_callback;
    std::map<SOCKET_FD, TcpSocketPtr> m_socketMap;
    std::recursive_mutex m_channelMutex;
    std::recursive_mutex m_errorMutex;
};
} // namespace neapu
#endif // __NETCPSERVER2_H__