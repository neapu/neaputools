#include "NEHttpServer.h"
using namespace neapu;

int neapu::HttpServer::Init(int _threadNum, const IPAddress& _addr)
{
    int rc = 0;
    rc = TcpServer::Init(_threadNum, _addr);
    return rc;
}

HttpServer& neapu::HttpServer::Get(const String _path, HttpServerCallback _cb)
{
    m_router.Insert(_path, _cb, HttpMethod::GET);
    return *this;
}

HttpServer& neapu::HttpServer::Post(const String _path, HttpServerCallback _cb)
{
    m_router.Insert(_path, _cb, HttpMethod::POST);
    return *this;
}

HttpServer& neapu::HttpServer::All(const String _path, HttpServerCallback _cb)
{
    m_router.Insert(_path, _cb, HttpMethod::ALL);
    return *this;
}

void neapu::HttpServer::HttpLog(bool _log)
{
    m_log = _log;
}

void neapu::HttpServer::StaticPath(const String& _reqPath, const String& _filePath, bool _enableRelativePath)
{
    m_staticPath.m_reqPath = _reqPath;
    m_staticPath.m_filePath = _filePath;
    m_staticPath.m_enableRelativePath = _enableRelativePath;
}

void neapu::HttpServer::HistoryMode(const String& _reqPath)
{
    m_HistoryPath = _reqPath;
}

void neapu::HttpServer::OnRecvData(std::shared_ptr<neapu::NetChannel> _client)
{
    ByteArray data = _client->ReadAll();
}

void neapu::HttpServer::OnAccepted(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_log) {
        Logger(LM_INFO) << "[HttpServer][OnAccepted]" << _client->GetAddress();
    }
}

void neapu::HttpServer::OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_log) {
        Logger(LM_INFO) << "[HttpServer][OnChannelClosed]" << _client->GetAddress();
    }
}

void neapu::HttpServer::OnChannelError(std::shared_ptr<neapu::NetChannel> _client)
{
    if (m_log) {
        Logger(LM_INFO) << "[HttpServer][OnChannelError]" << _client->GetAddress();
    }
}
