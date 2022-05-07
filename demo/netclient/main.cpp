#include <NETcpClient.h>
#include <iostream>
#include <NELogger.h>
using namespace neapu;
using namespace std;

void OnRecv(const ByteArray& data, uint64_t)
{
	Logger(LM_INFO) << "Recv data:" << data;
}

void OnConnected(uint64_t)
{
	Logger(LM_INFO) << "Connected";
}

int main()
{
	int rc;
	TcpClient cli;
	rc = cli.Connect("127.0.0.1", 7669, OnRecv, OnConnected);

	if (rc) {
		Logger(LM_ERROR) << "Connect failed:" << rc << " error:" << cli.GetLastErrorString() << " code:" << cli.GetLastError();
		return rc;
	}

	while (true) {
		string strin;
		cin >> strin;
		cli.Send(strin.c_str());
	}
}