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
	int rc = udp.Init(1, IPAddress::MakeAddress(IPAddress::Type::IPv4, String(), 8567));
#else
	int rc = udp.Init(1, IPAddress::MakeAddress(IPAddress::Type::IPv6, String(), 8567));
#endif // !IPV6
	Logger(LM_INFO) << "Init Over:" << rc;
	if (rc)return rc;
	Logger(LM_INFO) << "Bind:" << udp.Address();
	udp.OnRecvData([&](const ByteArray& _data, const IPAddress& _addr) {
			Logger(LM_INFO) << "Receive From:" << _addr << " Data:" << _data;
			udp.Send(_data, _addr);
		});
	rc = udp.Run();
	Logger(LM_INFO) << "Server Stop:" << rc;

	return 0;
}