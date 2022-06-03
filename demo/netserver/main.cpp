#include "NEString.h"
#include "NELogger.h"
#include "NEUtil.h"
#include "NETcpServer.h"
#include "NEByteArray.h"
#include <neapu-config.h>

using namespace neapu;
int main(int argc, char** argv)
{
    Settings set;
    IPAddress::Type type = IPAddress::Type::IPv4;
    String address = "0.0.0.0";
    int port = 9884;
    if (set.Init(String(NETOOLS_SOURCE_DIR) + "/demo/netserver/server.conf") == 0) {
        if (set.GetValue("server", "type", "IPv4") == "IPv6") {
            type = IPAddress::Type::IPv6;
        }

        address = set.GetValue("server", "address", "0.0.0.0");
        port = (int)set.GetValue("server", "port", "9884").ToInt();
    }
    TcpServer tcpServer;

    int rc = tcpServer.Init(1, IPAddress::MakeAddress(type, address, port));

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