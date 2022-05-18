#include <NETcpClient.h>
#include <iostream>
#include <NELogger.h>
#include <NEUtil.h>
using namespace neapu;
using namespace std;

#define IPV6

void WorkThread(TcpClient* cli)
{
	cli->Run();
}

int main()
{
	Settings set;
	IPAddress::Type type = IPAddress::Type::IPv4;
	String address = "127.0.0.1";
	int port = 9884;
	if (set.Init("client.conf") == 0) {
		if (set.GetValue("client", "type", "IPv4") == "IPv6") {
			type = IPAddress::Type::IPv6;
		}
		
		address = set.GetValue("client", "address", "127.0.0.1");
		port = (int)set.GetValue("client", "port", "9884").ToInt();
	}
	int rc = 0;
	TcpClient cli;
	rc = cli.Connect(IPAddress::MakeAddress(type, address, port));

	if (rc) {
		Logger(LM_ERROR) << "Connect failed:" << rc << cli.GetError();
		return rc;
	}
	Logger(LM_INFO) << "Connected:" << cli.GetAddress();

	cli.OnRecvData([&](const neapu::ByteArray& data) {
		Logger(LM_INFO) << "Receive Data:" << data;
	}).OnClosed([]() {
		Logger(LM_INFO) << "Closed";
	});

	std::thread t1 = thread(WorkThread, &cli);

	while (true) {
		string strin;
		cin >> strin;
		cli.Send(strin.c_str(), strin.size());
	}

	if (t1.joinable()) {
		t1.join();
	}
	return 0;
}