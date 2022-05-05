#include "Server.h"
#include "NELogger.h"
using namespace neapu;

void Server::OnRecvData(std::shared_ptr<neapu::NetChannel> _client)
{
    String recv = _client->ReadAll();
    Logger(LM_INFO) << "Receive request:" << recv;
    if(recv.length()>0){
        _client->Write(recv);
    }
}

void Server::OnListened()
{
    Logger(LM_INFO) << "Server start on " << m_port;
}

void Server::OnAccepted(std::shared_ptr<NetChannel> _client)
{
    Logger(LM_INFO)<<"Client Accept, "<<(*_client);
}

void Server::OnChannelClosed(std::shared_ptr<NetChannel> _client)
{
    Logger(LM_INFO)<<"Client Close, "<<(*_client);
}

void Server::OnChannelError(std::shared_ptr<NetChannel> _client)
{
    Logger(LM_ERROR)<<"Client Error, "<<(*_client);
}