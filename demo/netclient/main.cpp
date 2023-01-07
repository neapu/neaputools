
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

int main()
{
    IPAddress::Type type = IPAddress::Type::IPv4;
    String address = "192.168.2.52";
    int port = 9884;
    int ret = 0;

    std::shared_ptr<TcpClient2> cli = std::make_shared<TcpClient2>();
    auto addr = IPAddress::MakeAddress(type, address, port);
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

	smg.OnReadEvent([&](SocketBasePtr _sock){
		auto sock = std::static_pointer_cast<TcpClient2>(_sock);
		LOG_INFO << "Receive Data:" << sock->ReadAll();
	});

	smg.AddTimer(1000, true, [&](int id){
		String testData = "test data";
		cli->Send(testData);
	});

	smg.LoopStart();

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