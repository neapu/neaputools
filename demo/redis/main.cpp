#include <iostream>
#include <NERedisConnector.h>
#include <NEIPAddress.h>
#include <NELogger.h>
using namespace neapu;

int main()
{
    RedisConnector connector;
    int rc = connector.Connect(IPAddress::MakeAddress(IPAddress::Type::IPv4, "192.168.2.211", 6379));
    if (rc < 0) {
        Logger(LM_ERROR) << "Connect Error:" << rc;
    }
    rc = connector.Auth("pytafjamt");
    if (rc != 0) {
        Logger(LM_ERROR) << "Auth Failed" << connector.GetReidsError();
        return 0;
    }
    
    rc = connector.SyncSet("test2", "888");
    if (rc != 0) {
        Logger(LM_ERROR) << "Set Failed" << connector.GetReidsError();
        return 0;
    }

    auto rst = connector.SyncGet("test2");
    if (rst.IsString()) {
        Logger(LM_INFO) << "Get Data:" << rst._string;
    }
    else if (rst.IsError()) {
        Logger(LM_INFO) << "Get Failed:" << rst._errorString;
    }
    else {
        Logger(LM_ERROR) << "Type Error" << (int)rst._type;
        return 0;
    }
    return 0;
}