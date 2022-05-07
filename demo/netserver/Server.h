#pragma once
#include "NETcpBase.h"

class Server: public neapu::TcpBase{
public:
protected:
    virtual void OnRecvData(std::shared_ptr<neapu::NetChannel> _client) override;
    virtual void OnListened() override;
    virtual void OnAccepted(std::shared_ptr<neapu::NetChannel> _client) override;
    virtual void OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client) override;
    virtual void OnChannelError(std::shared_ptr<neapu::NetChannel> _client) override;
};