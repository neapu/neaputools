#ifndef __NESOCKETMANAGER_H__
#define __NESOCKETMANAGER_H__

#include "network/NEEventBase2.h"
#include "network/NESocketBase.h"
#include <functional>
#include <map>
#include <mutex>
namespace neapu {
class SocketManager : public EventBase2 {
public:
    using SocketCallback = std::function<void(SocketBasePtr)>;

    int AddSocket(SocketBasePtr _sock);

    SocketManager& OnReadEvent(SocketCallback _cb);

protected:
    virtual void OnReadEvent(SocketBasePtr _sock);

private:
    virtual void OnSocketTriggerCallback(SOCKET_FD _fd, EventBase2::EventType _events) override;

private:
    SocketCallback m_callback;

    std::map<SOCKET_FD, SocketBasePtr> m_socketMap;
    std::recursive_mutex m_mapMutex;
};
} // namespace neapu
#endif // __NESOCKETMANAGER_H__