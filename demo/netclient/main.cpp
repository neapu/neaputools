
#include "network/NESocketBase.h"
#include "network/NESocketManager.h"
#include "base/NEString.h"
#include "logger/logger.h"
#include <iostream>
#include <base/NEUtil.h>
#include <memory>
#include <neapu-config.h>
#include <signal.h>
#include <base/NESettings.h>
#include <network/NETcpClient2.h>
using namespace neapu;
using namespace std;

int main(int argc, char** argv)
{
    Arguments args(argc, argv);
    String strAddr = args.GetValue("addr", "127.0.0.1:8000");

    int ret = 0;

    std::shared_ptr<TcpClient2> cli = std::make_shared<TcpClient2>();
    auto addr = IPAddress::MakeAddress(strAddr);
    if (addr.IsEmptyAddr()) {
        LOG_DEADLY << "address is invalid.";
        return -1;
    }

    Logger(LM_INFO) << "Connect to:" << addr;
    ret = cli->Connect(addr);

    if (ret) {
        Logger(LM_ERROR) << "Connect failed:" << ret << cli->GetLastError();
        return ret;
    }
    Logger(LM_INFO) << "Connected:" << cli->GetAddress();

    SocketManager smg;
    ret = smg.Init();
    if (ret < 0) {
        LOG_ERROR << "SocketManager Init error:" << ret;
    }

    smg.AddSocket(cli);

    smg.OnReadEvent([&](SocketBasePtr _sock) {
        auto sock = std::static_pointer_cast<TcpClient2>(_sock);
        LOG_INFO << "Receive Data:" << sock->ReadAll();
    });

    smg.AddTimer(2000, true, [&](int id) {
        String testData = "test data";
        int ret = cli->Send(testData);
        LOG_INFO << "Send Data:" << ret;
        if (ret == -1) {
            smg.Stop();
        }
    });

    smg.LoopStart();
    LOG_INFO << "Stoped";

    // cli.OnRecvData([&](shared_ptr<NetChannel> _channel) {
    //     Logger(LM_INFO) << "Receive Data:" << _channel->ReadAll();
    // });

    // cli.OnClosed([]() {
    //     Logger(LM_INFO) << "Closed";
    // });

    // cli.AddSignal(SIGINT, false, [&](int _signal, EventHandle _handle) {
    //     Logger(LM_INFO) << "SIGINT trigger";
    //     cli.Stop();
    // });

    // cli.AddTimer(1000, true, [&](EventHandle) {
    //     String testData = "test data";
    //     cli.Send(testData);
    // });

    // cli.Run();
    return 0;
}