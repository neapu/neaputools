#include "NERedisConnector.h"
#include "NETcpClient.h"
using namespace neapu;

int neapu::RedisConnector::Connect(const IPAddress& _addr)
{
    if (m_tcpClientSync) {
        m_tcpClientSync->Close();
        m_tcpClientSync.reset();
    }

    m_tcpClientSync = std::unique_ptr<TcpClientSync>(new TcpClientSync);
    int rc = m_tcpClientSync->Connect(_addr);
    if (rc < 0) {
        m_tcpClientSync->Close();
        m_tcpClientSync.reset();
        return rc;
    }
    return 0;
}

RedisResponse neapu::RedisConnector::SyncRunCommand(String _cmd)
{
    RedisResponse rst;
    std::unique_lock<std::mutex> connectorLocker(m_connectorMutex);
    if (!m_tcpClientSync || !m_tcpClientSync->IsConnected()) {
        rst._type = RedisResponse::Type::Error;
        rst._errorCode = -1;
        rst._errorString = "no connect";
        return rst;
    }
    if (_cmd.IsEmpty())return rst;
    if (_cmd.Back() != '\n') {
        if (_cmd.Back() == '\r') {
            _cmd.Append('\n');
        }
        else {
            _cmd.Append("\r\n");
        }
    }
    int rc = m_tcpClientSync->Send(_cmd);
    if (rc < 0) {
        rst._type = RedisResponse::Type::Error;
        rst._errorCode = -2;
        rst._errorString = "send error";
        return rst;
    }
    m_responseData = m_tcpClientSync->Recv();
    if (m_responseData == "+OK\r\n") {
        rst._type = RedisResponse::Type::Success;
        return rst;
    }
    else if (m_responseData == "ConnectorError") {
        rst._type = RedisResponse::Type::Error;
        rst._errorCode = -3;
        rst._errorString = "connector error";
        return rst;
    }
    else if (m_responseData.IsEmpty()) {
        rst._type = RedisResponse::Type::Error;
        rst._errorCode = -4;
        rst._errorString = "connect close";
        return rst;
    }
    else if(m_responseData.Front() == '-') {
        rst._type = RedisResponse::Type::Error;
        rst._errorCode = -5;
        rst._errorString = m_responseData;
        return rst;
    }
    else if (m_responseData.Front() == '$') {//字符串
        auto datas = m_responseData.Split("\r\n");
        int64_t len = datas[0].Middle(1, String::end).ToInt();
        if (len <= 0 || datas.size() < 2) {
            rst._type = RedisResponse::Type::String;
            return rst;
        }
        String data = datas[1];
        if (data.Length() != len) {
            rst._type = RedisResponse::Type::Error;
            rst._errorCode = -6;
            rst._errorString = "data format error";
            return rst;
        }
        rst._type = RedisResponse::Type::String;
        rst._string = data;
    }
    else if (m_responseData.Front() == ':') {//整数
        rst._type = RedisResponse::Type::Integer;
        rst._integer = m_responseData.Middle(1, String::end).ToInt();
        return rst;
    }
    return rst;
}

int neapu::RedisConnector::Auth(String _password)
{
    auto rst = SyncRunCommand(String("AUTH %1").Argument(_password));
    if (rst.IsError()) {
        m_redisError = rst._errorString;
    }
    return rst._type == RedisResponse::Type::Success?0:-1;
}

int neapu::RedisConnector::SyncSet(String _key, String _value)
{
    auto rst = SyncRunCommand(String("SET %1 %2").Argument(_key).Argument(_value));
    if (rst.IsError()) {
        m_redisError = rst._errorString;
    }
    return rst._type == RedisResponse::Type::Success?0:-1;
}

RedisResponse neapu::RedisConnector::SyncGet(String _key)
{
    return SyncRunCommand(String("GET %1").Argument(_key));
}
