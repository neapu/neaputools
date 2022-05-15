#include <NETcpClient.h>
#include <iostream>
#include <NELogger.h>
using namespace neapu;
using namespace std;

#define IPV6

void WorkThread(TcpClient* cli)
{
	cli->Run();
}

int main()
{
	int rc = 0;
	TcpClient cli;
#ifndef IPV6
	rc = cli.Connect(IPAddress::MakeAddress(IPAddress::Type::IPv4, "127.0.0.1", 9884));
#else
	rc = cli.Connect(IPAddress::MakeAddress(IPAddress::Type::IPv6, "::1", 9884));
#endif
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
		cli.Send(strin.c_str());
	}

	if (t1.joinable()) {
		t1.join();
	}
	return 0;
}