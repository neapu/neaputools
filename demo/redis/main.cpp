#include <iostream>
#include <NERedisConnector.h>
using namespace neapu;

int main()
{
    RedisConnector connector;
    int rc = connector.Connect(IPAddress::MakeAddress(IPAddress::Type::IPv4, "192.168.2.211", 6379));
    if (rc < 0) {
        Logger(LM_ERROR) << "Connect Error:" << rc << connector.GetError();
    }
    auto rst = connector.SyncRunCommand("AUTH pytafjamt");
    if (rst._type == RedisResponse::Type::Success) {
        Logger(LM_INFO) << "Auth Success";
    }
    else {
        Logger(LM_ERROR) << "Auth Failed";
        return 0;
    }

    rst = connector.SyncRunCommand("SET test1 123");
    if (rst._type == RedisResponse::Type::Success) {
        Logger(LM_INFO) << "Set Success";
    }
    else {
        Logger(LM_ERROR) << "Set Failed";
        return 0;
    }

    rst = connector.SyncRunCommand("GET test1");
    if (rst._type == RedisResponse::Type::String) {
        Logger(LM_INFO) << "Get Data:" << rst._string;
    }
    else {
        Logger(LM_ERROR) << "Type Error" << (int)rst._type;
        return 0;
    }
    return 0;
}