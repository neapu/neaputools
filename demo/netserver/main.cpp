#include "NEString.h"
#include "NELogger.h"
#include "NEUtil.h"
#include "NETcpServer.h"

#define IPV6

using namespace neapu;
int main(int argc, char** argv)
{
    Arguments arg(argc, argv);
    int port = arg.GetValue("port", "9884").ToInt();
    TcpServer tcpServer;
#ifndef IPV6
    int rc = tcpServer.Init(1, IPAddress::MakeAddress(IPAddress::Type::IPv4, String(), port));
#else
    int rc = tcpServer.Init(1, IPAddress::MakeAddress(IPAddress::Type::IPv6, String(), port));
#endif
    if (rc < 0) {
        Logger(LM_ERROR) << "Server Init Error:" << rc << " " << tcpServer.GetError();
        return 0;
    }
    Logger(LM_INFO) << "Server Start:" << tcpServer.GetAddress();
    tcpServer.OnAccepted([&](std::shared_ptr<NetChannel> _netChannel) {
        Logger(LM_INFO) << "Client Accept:" << *_netChannel;
    }).OnRecvData([&](std::shared_ptr<NetChannel> _netChannel) {
        ByteArray data = _netChannel->ReadAll();
        Logger(LM_INFO) << "Receive From:" << *_netChannel;
        Logger(LM_INFO) << "Recvice Data:" << data;
        //_netChannel->Write(data);
    }).OnChannelClosed([&](std::shared_ptr<NetChannel> _netChannel) {
        Logger(LM_INFO) << "Client Close:" << *_netChannel;
    }).OnChannelError([&](std::shared_ptr<NetChannel> _netChannel) {
        Logger(LM_ERROR) << "Channel Error [" << *_netChannel << "]:" << _netChannel->GetError();
    });
    rc = tcpServer.Run();
    Logger(LM_INFO) << "Server Stop:" << rc;
    return 0;
}