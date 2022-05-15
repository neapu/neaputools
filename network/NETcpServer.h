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
    TcpServer() {}
    int Init(int _threadNum, const IPAddress& _addr, bool _enableWriteCallback = false);
    void Stop();

protected:
    virtual void OnRecvData(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnAccepted(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client);
    virtual void OnChannelError(std::shared_ptr<neapu::NetChannel> _client);

    
    virtual void OnWriteReady(int _fd) override;
    virtual void OnSignalReady(int _signal) override;

    virtual int OnClientReadReady(int _fd);
    void OnListenerAccept(int fd);
    static std::shared_ptr<NetChannel> MakeChannel(int fd, sockaddr_in& sin);
    void AddClient(int fd, sockaddr_in& sin);
    void ReleaseClient(int fd);
protected:
    using TcpServerCallback = std::function<void(std::shared_ptr<neapu::NetChannel> _client)>;
    struct {
        TcpServerCallback onRecvData;
        TcpServerCallback onAccepted;
        TcpServerCallback onChannelClose;
        TcpServerCallback onChannelError;
    } m_callback;

private:
    virtual void OnReadReady(int _fd) override;

private:
    int m_listenFd = 0;
    short m_socketEvent = 0;
    IPAddress m_address;
    std::map<int,std::shared_ptr<NetChannel>> m_channels;
    std::recursive_mutex m_channelMutex;
};
}
