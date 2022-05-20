#pragma once
#include "network_pub.h"
#include "NENetChannel.h"
#include <map>
#include <memory>
#include <vector>
#include <thread>
#include <functional>
#include "NENetworkError.h"
#include "NENetBase.h"
#include "NEIPAddress.h"

namespace neapu {
class NEAPU_NETWORK_EXPORT TcpServer : public NetBase{
public: 
    using TcpServerCallback = std::function<void(std::shared_ptr<neapu::NetChannel> _client)>;
    TcpServer() {}
    int Init(int _threadNum, const IPAddress& _addr);
    
    TcpServer& OnRecvData(TcpServerCallback _cb);
    TcpServer& OnAccepted(TcpServerCallback _cb);
    TcpServer& OnChannelClosed(TcpServerCallback _cb);
    TcpServer& OnChannelError(TcpServerCallback _cb);
    TcpServer& OnChannelWrite(TcpServerCallback _cb);

    IPAddress GetAddress() const { return m_address; }
protected:
    virtual void OnRecvData(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnAccepted(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnChannelError(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnChannelWrite(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnSignalReady(int _signal) override;

    virtual int OnClientReadReady(int _fd);
    void OnListenerAccept(int fd);
    void AddClient(int fd, const IPAddress& _addr);
    void ReleaseClient(int fd);
    virtual void Stoped() override;
protected:
    
    struct {
        TcpServerCallback onRecvData;
        TcpServerCallback onAccepted;
        TcpServerCallback onChannelClose;
        TcpServerCallback onChannelError;
        TcpServerCallback onChannelWrite;
    } m_callback;

private:
    virtual void OnReadReady(int _fd) override;
    virtual void OnWriteReady(int _fd) override;

private:
    int m_listenFd = 0;
    IPAddress m_address;
    std::map<int,std::shared_ptr<NetChannel>> m_channels;
    std::recursive_mutex m_channelMutex;
};
}
