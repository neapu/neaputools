#include "Server.h"
#include "NTLogger.h"
using namespace neapu;

void Server::OnRecvRequest(std::shared_ptr<neapu::NetClient> _client)
{
    String recv = _client->ReadAll();
    if(recv.length()>0){
        _client->Write(recv);
    }
}

void Server::OnListened()
{
    Logger(LM_INFO) << "Server start on " << m_port;
}

void Server::OnAccept(std::shared_ptr<NetClient> _client)
{
    Logger(LM_INFO)<<"Client Accept, "<<(*_client);
}

void Server::OnClientClosed(std::shared_ptr<NetClient> _client)
{
    Logger(LM_INFO)<<"Client Close, "<<(*_client);
}

void Server::OnClientError(std::shared_ptr<NetClient> _client)
{
    Logger(LM_ERROR)<<"Client Error, "<<(*_client);
}