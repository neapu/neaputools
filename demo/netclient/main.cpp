#include <NETcpClient.h>
#include <iostream>
#include <NELogger.h>
#include <NEUtil.h>
#include <neapu-config.h>
using namespace neapu;
using namespace std;

void InputThread(TcpClient* _cli)
{
	char buf[1024];
	while (true) {
		string str;
		getline(cin, str);
		str += "\r\n";
		_cli->Send(str.c_str(), str.length());
	}
}

int main()
{
	Settings set;
	IPAddress::Type type = IPAddress::Type::IPv4;
	String address = "127.0.0.1";
	int port = 9884;
	if (set.Init(String(NETOOLS_SOURCE_DIR)+"/demo/configs/client.conf") == 0) {
		if (set.GetValue("client", "type", "IPv4") == "IPv6") {
			type = IPAddress::Type::IPv6;
		}
		
		address = set.GetValue("client", "address", "127.0.0.1");
		port = (int)set.GetValue("client", "port", "9884").ToInt();
	}
	int rc = 0;


	TcpClient cli;
	auto addr = IPAddress::MakeAddress(type, address, port);
	Logger(LM_INFO) << "Connect to:" << addr;
	rc = cli.Connect(addr);

	if (rc) {
		Logger(LM_ERROR) << "Connect failed:" << rc << cli.GetError();
		return rc;
	}
	Logger(LM_INFO) << "Connected:" << cli.GetAddress();

	cli.OnRecvData([&](shared_ptr<NetChannel> _channel) {
		Logger(LM_INFO) << "Receive Data:" << _channel->ReadAll();
	}).OnClosed([]() {
		Logger(LM_INFO) << "Closed";
	});

	thread(InputThread, &cli).detach();

	cli.Run();
	return 0;
}