#include "base/NEByteArray.h"
#include "network/NEIPAddress.h"
#include "network/NENetworkError.h"
#include "network/NESocketBase.h"
#include "network/NESocketManager.h"
#include "base/NEString.h"
#include "iostream"
#include "logger/logger.h"
#include "network/network_pub.h"
#include <network/NEUdpSocket.h>
#include <logger/logger.h>
#include <base/NEUtil.h>
#include <memory>
#include <neapu-config.h>
#include <signal.h>
#include <base/NESettings.h>
#include <string.h>
using namespace std;
using namespace neapu;

#define IPV6

int main()
{
    IPAddress::Type type = IPAddress::Type::IPv4;
    String address = "0.0.0.0";
    // IPAddress::Type type = IPAddress::Type::IPv6;
    // String address = "::";
    int port = 9884;

    UdpSocketPtr udp = std::make_shared<UdpSocket>();
    int rc = udp->Init(IPAddress::MakeAddress(type, address, port));
    SOCKET_FD fd = udp->GetSocket();

    Logger(LM_INFO) << "Init Over:" << rc;
    if (rc < 0) {
        LOG_INFO << "Init Error:" << rc << " err:" << udp->GetLastError();
        return rc;
    }
    Logger(LM_INFO) << "Bind:" << udp->GetBindAddr();

    String testData = "test data";
    auto target = IPAddress::MakeAddress(IPAddress::Type::IPv4, "192.168.2.52", 7886);
    // auto target = IPAddress::MakeAddress(IPAddress::Type::IPv6, "240e:3b4:38ef:17a1::f88", 5769);
    // rc = udp->SendTo(testData, target);
    // LOG_INFO << "send:" << rc;
    // auto [data, addr] = udp->RecvFrom(1024);
    // LOG_INFO << "Receive Data:" << data;
    // LOG_INFO << "len:" << data.Length() << " Address:" << addr;

    SocketManager smg;
    rc = smg.Init();
    if (rc < 0) {
        LOG_ERROR << "SocketManager Init Error:" << rc;
        return -1;
    }

    rc = smg.AddSocket(udp);
    if (rc < 0) {
        LOG_ERROR << "add socket failed:" << rc;
        return -1;
    }

    smg.OnReadEvent([](SocketBasePtr _sock) {
        auto udpSock = std::static_pointer_cast<UdpSocket>(_sock);
        auto [data, addr] = udpSock->RecvFrom(1024);
        LOG_INFO << "Receive Data:" << data;
        LOG_INFO << "len:" << data.Length() << " Address:" << addr;
    });

    smg.AddSignal(SIGINT, [&](int _signal) {
        LOG_INFO << "On Signal:" << _signal;
        if (_signal == SIGINT) {
            smg.Stop();
        }
    });

    smg.AddTimer(5000, true, [&](int _id) {
        int ret = udp->SendTo(testData, target);
        if (ret < 0) {
            LOG_ERROR << "Send Error:" << udp->GetLastError();
        } else {
            LOG_INFO << "send data:" << ret;
        }
    });

    smg.LoopStart();
    LOG_INFO << "Shutdown";

    return 0;
}