#pragma once
#include "NTNetBase.h"

class Server: public neapu::NetBase{
public:
protected:
    virtual void OnRecvRequest(std::shared_ptr<neapu::NetClient> _client) override;
    virtual void OnListened() override;
    virtual void OnAccept(std::shared_ptr<neapu::NetClient> _client) override;
    virtual void OnClientClosed(std::shared_ptr<neapu::NetClient> _client) override;
    virtual void OnClientError(std::shared_ptr<neapu::NetClient> _client) override;
};