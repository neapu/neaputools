#include "iostream"
#include <NEUdpBase.h>
#include <NELogger.h>
using namespace std;
using namespace neapu;

#define IPV6

int main()
{
	UdpBase udp;
#ifndef IPV6
	int rc = udp.Init(8567, 1, IPAddress::Type::IPv4);
#else
	int rc = udp.Init(8567, 1, IPAddress::Type::IPv6);
#endif // !IPV6
	Logger(LM_INFO) << "Init Over:" << rc;
	if (rc)return rc;
	Logger(LM_INFO) << "Bind Port:" << udp.BindPort();
	udp.OnRecvData([](const ByteArray& _data, const IPAddress& _addr) {
			Logger(LM_INFO) << "Receive From:" << _addr.ToString() << " Data:" << _data;
		});
	rc = udp.Run();
	Logger(LM_INFO) << "Server Stop:" << rc;
	return rc;
}