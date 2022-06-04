#include "iostream"
#include <NEUdpBase.h>
#include <NELogger.h>
#include <NEUtil.h>
#include <neapu-config.h>
using namespace std;
using namespace neapu;

#define IPV6

int main()
{
	Settings set;
	IPAddress::Type type = IPAddress::Type::IPv4;
	String address = "0.0.0.0";
	int port = 9884;
	if (set.Init(String(NETOOLS_SOURCE_DIR) + "/demo/configs/udp.conf") == 0) {
		if (set.GetValue("udp", "type", "IPv4") == "IPv6") {
			type = IPAddress::Type::IPv6;
		}

		address = set.GetValue("udp", "address", "0.0.0.0");
		port = (int)set.GetValue("udp", "port", "9884").ToInt();
	}
	UdpBase udp;
	int rc = udp.Init(1, IPAddress::MakeAddress(type, address, port));

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