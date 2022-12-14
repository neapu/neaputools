#include "NEHttpServer.h"
#include "NEFile.h"
#include "NEHttpHandle.h"

using namespace neapu;

std::map<String, String> neapu::HttpServer::m_contentTypeMap{
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"gif", "image/png"},
    {"txt", "text/plain"},
    {"html", "text/html"},
    {"htm", "text/html"},
    {"xml", "text/xml"},
    {"css", "text/css"},
    {"js", "application/x-javascript"},
    {"json", "application/json"},
    {"ico", "image/x-icon"}};

int neapu::HttpServer::Init(int _threadNum, const IPAddress& _addr)
{
    int rc = 0;
    rc = TcpServer2::Init(_addr, _threadNum);
    return rc;
}

HttpServer& neapu::HttpServer::Get(const String& _path,
                                   HttpServerCallback _cb)
{
    m_router.Insert(_path, _cb, HttpMethod::GET);
    return *this;
}

HttpServer& neapu::HttpServer::Post(const String& _path,
                                    HttpServerCallback _cb)
{
    m_router.Insert(_path, _cb, HttpMethod::POST);
    return *this;
}

HttpServer& neapu::HttpServer::All(const String& _path,
                                   HttpServerCallback _cb)
{
    m_router.Insert(_path, _cb, HttpMethod::ALL);
    return *this;
}

void neapu::HttpServer::AddRouter(const String& _path, HttpMethod _method,
                                  HttpServerCallback _cb)
{
    m_router.Insert(_path, _cb, _method);
}

void neapu::HttpServer::HttpLog(bool _log)
{
    m_log = _log;
}

void neapu::HttpServer::StaticPath(const String& _reqPath,
                                   const String& _filePath,
                                   const size_t _cacheSize,
                                   const time_t _cacheTimeout,
                                   bool _enableRelativePath)
{
    m_staticPath.m_reqPath = _reqPath;
    m_staticPath.m_filePath = _filePath;
    if (m_staticPath.m_filePath.Back() != '\\' && m_staticPath.m_filePath.Back() != '/') {
#ifdef _WIN32
        m_staticPath.m_filePath.Append('\\');
#else
        m_staticPath.m_filePath.Append('/');
#endif // _WIN32
    }
    m_staticPath.m_enableRelativePath = _enableRelativePath;
    m_staticFileCacheSize = _cacheSize;
    m_staticCacheTimeout = _cacheTimeout;
}

int neapu::HttpServer::HistoryMode(const String& _filePath)
{
    File file(_filePath);
    if (!file.Open(File::OpenMode::ReadOnly)) {
        return -1;
    }
    m_historyMode.enable = true;

    String extName = file.Extension();
    String contentType;
    if (m_contentTypeMap.find(extName) != m_contentTypeMap.end()) {
        contentType = m_contentTypeMap[extName];
    } else {
        contentType = "application/octet-stream";
    }
    m_historyMode.cache.m_contentType = contentType;
    m_historyMode.cache.update(file.Read(), std::move(contentType));

    return 0;
}

void neapu::HttpServer::SetHttpRequestReceiveSize(size_t _size)
{
    m_httpRequestSize = _size;
}

void neapu::HttpServer::OnRequestStaticFile(
    const String& _reqPath, std::shared_ptr<HttpHandle> _handle)
{
    //??????????????????????????????????????????
    if (!m_staticPath.m_enableRelativePath && (_reqPath.Contain("../") || _reqPath.Contain("..\\") || _reqPath.Contain("./") || _reqPath.Contain(".\\"))) {
        _handle->SetState(403, "Forbidden");
        _handle->SendResponse(ByteArray());
    }
    if (m_staticFileCache.find(_reqPath) != m_staticFileCache.end()) {
        auto& cache = m_staticFileCache[_reqPath];
        if (m_staticCacheTimeout && time(nullptr) - cache.m_lastupd < m_staticCacheTimeout) {
            _handle->AddSendHeader("Content-Type", cache.m_contentType);
            _handle->SendResponse(cache.m_data);
            return;
        }
    }
    //????????????
    String strFilePath =
        m_staticPath.m_filePath + _reqPath.Middle(m_staticPath.m_reqPath.Length(), String::end);
    File file(strFilePath);
    if (file.Open(File::OpenMode::ReadOnly)) {
        String extName = file.Extension();
        ByteArray data = file.Read();
        String contentType;
        if (m_contentTypeMap.find(extName) != m_contentTypeMap.end()) {
            contentType = m_contentTypeMap[extName];
        } else {
            contentType = "application/octet-stream";
        }
        if (data.Length() < m_staticFileCacheSize) {
            m_staticFileCache[_reqPath].update(data, contentType);
        }
        _handle->AddSendHeader("Content-Type", contentType);
        _handle->SendResponse(data);
    } else if (file.Extension().IsEmpty() && m_historyMode.enable) {
        if (m_staticCacheTimeout && time(nullptr) - m_historyMode.cache.m_lastupd < m_staticCacheTimeout) {
            _handle->AddSendHeader("Content-Type", m_historyMode.cache.m_contentType);
            _handle->SendResponse(m_historyMode.cache.m_data);
        } else {
            HistoryMode(String(m_historyMode.path));
        }
    } else {
        _handle->SetState(404, "Not Found");
        _handle->SendResponse(ByteArray("404 Not Found"));
    }
}

void neapu::HttpServer::OnReceive(TcpSocketPtr _socket)
{
    std::shared_ptr<HttpHandle> httpHandle(new HttpHandle(_socket));
    int rst = httpHandle->AnalysisRequest(m_httpRequestSize);
    if (rst != 0) {
        _socket->Close();
        return;
    }
    String path = httpHandle->Path();
    LOG_DEBUG << "Receive request path:" << path;
    HttpMethod method = httpHandle->Method();
    auto callback = m_router.Find(path, method);
    if (callback) {
        callback(httpHandle);
    } else if (method == HttpMethod::GET) {
        if (path.Left(m_staticPath.m_reqPath.Length()) == m_staticPath.m_reqPath) {
            OnRequestStaticFile(path, httpHandle);
        }
    } else {
        httpHandle->SetState(404, "Not Found");
        httpHandle->SendResponse(ByteArray("404 Not Found"));
    }
    auto Connection = httpHandle->GetRecvHeader("Connection");
    if (Connection != "keep-alive") {
        httpHandle->CloseConnetion();
    }
}

void neapu::HttpServer::OnAccepted(TcpSocketPtr _socket)
{
    if (m_log) {
        Logger(LM_INFO) << "[HttpServer][OnAccepted]" << _socket->GetAddress();
    }
}

void neapu::HttpServer::OnClose(TcpSocketPtr _socket)
{
    if (m_log) {
        Logger(LM_INFO) << "[HttpServer][OnChannelClosed]" << _socket->GetAddress();
    }
}

void neapu::HttpServer::OnError(TcpSocketPtr _socket)
{
    if (m_log) {
        Logger(LM_INFO) << "[HttpServer][OnChannelError]" << _socket->GetAddress();
    }
}

void neapu::HttpServer::StaticFileCache::update(ByteArray&& _data,
                                                String&& _contentType)
{
    m_data = std::move(_data);
    m_contentType = std::move(_contentType);
    m_lastupd = time(nullptr);
}

void neapu::HttpServer::StaticFileCache::update(const ByteArray& _data,
                                                const String& _contentType)
{
    m_data = _data;
    m_contentType = _contentType;
    m_lastupd = time(nullptr);
}
