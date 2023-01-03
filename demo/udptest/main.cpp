#include "NEByteArray.h"
#include "NEIPAddress.h"
#include "NEString.h"
#include "iostream"
#include "logger/logger.h"
#include <NEUdpSocket.h>
#include <NELogger.h>
#include <NEUtil.h>
#include <neapu-config.h>
#include <signal.h>
#include <base/NESettings.h>
using namespace std;
using namespace neapu;

#define IPV6

int main()
{
    // IPAddress::Type type = IPAddress::Type::IPv4;
    // String address = "0.0.0.0";
    IPAddress::Type type = IPAddress::Type::IPv6;
    String address = "::";
    int port = 9884;

    UdpSocket udp;
    int rc = udp.Init(IPAddress::MakeAddress(type, address, port));

    Logger(LM_INFO) << "Init Over:" << rc;
    if (rc) return rc;
    Logger(LM_INFO) << "Bind:" << udp.GetBindAddr();

    String testData = "test data";
    // auto target = IPAddress::MakeAddress(IPAddress::Type::IPv4, "127.0.0.1", 5769);
    auto target = IPAddress::MakeAddress(IPAddress::Type::IPv6, "240e:3b4:38ef:17a1::f88", 5769);
    rc = udp.SendTo(testData, target);
    LOG_INFO << "send:" << rc;
    auto [data, addr] = udp.RecvFrom(1024);
    LOG_INFO << "Receive Data:" << data;
    LOG_INFO << "len:" << data.Length() << " Address:" << addr;

    return 0;
}