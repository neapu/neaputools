#include "NERedisConnector.h"
using namespace neapu;

RedisResponse neapu::RedisConnector::SyncRunCommand(String _cmd)
{
    RedisResponse rst;
    std::unique_lock<std::mutex> connectorLocker(m_connectorMutex);
    if (!IsConnected()) {
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
    std::unique_lock<std::mutex> syncLocker(m_syncMutex);
    int rc = Send(_cmd);
    if (rc < 0) {
        rst._type = RedisResponse::Type::Error;
        rst._errorCode = -2;
        rst._errorString = "send error";
        return rst;
    }
    m_rsped = false;
    while (!m_rsped) {
        m_syncCond.wait(syncLocker);
    }
    if (m_responseData == "+OK") {
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
        if (datas.size() < 2) {
            rst._type = RedisResponse::Type::Error;
            rst._errorCode = -6;
            rst._errorString = "data format error";
            return rst;
        }
        size_t len = datas[0].Middle(1, String::end).ToInt();
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

void neapu::RedisConnector::OnRecvData(std::shared_ptr<NetChannel> _channel)
{
    m_responseData = _channel->ReadAll();
    m_rsped = true;
    m_syncCond.notify_one();
}

void neapu::RedisConnector::OnError(const NetworkError& _err)
{
    m_responseData = "ConnectorError";
    m_rsped = true;
    m_syncCond.notify_one();
}

void neapu::RedisConnector::OnClosed()
{
    m_responseData.Clear();
    m_rsped = true;
    m_syncCond.notify_one();
}
