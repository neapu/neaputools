#include "base/NEString.h"
#include "base/NELogger.h"
#include "base/NEUtil.h"
#include "logger/logger.h"
#include "network/NETcpServer2.h"
#include "base/NEByteArray.h"
#include <neapu-config.h>
#include <signal.h>
#include <base/NESettings.h>

using namespace neapu;
int main(int argc, char** argv)
{
    Arguments args(argc, argv);
    String address = args.GetValue("bind", "0.0.0.0:8000");

    TcpServer2 tcpServer;
    int rc = tcpServer.Init(IPAddress::MakeAddress(address));

    if (rc < 0) {
        Logger(LM_ERROR) << "Server Init Error:" << rc << " " << tcpServer.GetError();
        return 0;
    }
    Logger(LM_INFO) << "Server Start:" << tcpServer.GetAddress();
    tcpServer.OnAccepted([&](std::shared_ptr<NetChannel> _netChannel) {
        Logger(LM_INFO) << "Client Accept:" << *_netChannel;
    });

    tcpServer.OnReceive([&](std::shared_ptr<NetChannel> _netChannel) {
        ByteArray data = _netChannel->ReadAll();
        Logger(LM_INFO) << "Receive From:" << *_netChannel;
        Logger(LM_INFO) << "Recvice Data:" << data;
        _netChannel->Write(data);
    });

    tcpServer.OnClosed([&](std::shared_ptr<NetChannel> _netChannel) {
        Logger(LM_INFO) << "Client Close:" << *_netChannel;
    });

    tcpServer.OnError([&](std::shared_ptr<NetChannel> _netChannel) {
        Logger(LM_ERROR) << "Channel Error [" << *_netChannel << "]:" << _netChannel->GetLastError();
    });

    tcpServer.AddSignal(SIGINT, [&](int _signal) {
        Logger(LM_INFO) << "SIGINT trigger";
        tcpServer.Stop();
    });

    rc = tcpServer.Listen();
    if (rc < 0) {
        Logger(LM_ERROR) << "Listen Error:" << tcpServer.GetError();
        return rc;
    }

    Logger(LM_INFO) << "Server Stop:" << rc;
    return 0;
}