#pragma once
#include "network_pub.h"
#include "NETcpServer.h"
#include "NEHttpRequest.h"
#include "NEHttpResponse.h"
#include "NERouter.h"

namespace neapu {
    class HttpServer :public TcpServer {
    public:
        using HttpServerCallback = std::function<HttpResponse(const HttpRequest&)>;
        int Init(int _threadNum, const IPAddress& _addr);

        /*********************************
        * 设置HTTP请求路径回调
        * 路径支持在最后加通配符*
        * 例如：/home/*
        * 不支持在路径中间加通配符
        *********************************/
        HttpServer& Get(const String _path, HttpServerCallback);
        HttpServer& Post(const String _path, HttpServerCallback);
        HttpServer& All(const String _path, HttpServerCallback);

        /****************************************
        * 开启或关闭Http日志打印
        * 主要是Accepted、Closed等发生时的日志打印
        * 不会影响用户日志
        ****************************************/
        void HttpLog(bool _log = true);

        
        //设置静态文件路径
        void StaticPath(const String& _reqPath, const String& _filePath, bool _enableRelativePath = false);
        /********************************
        * 单页面应用的history模式
        * 当请求不存在的路径时返回指定文件
        * _reqPath: 指定文件的路径，相当于用这个路径替换了不存在的路径的请求。例如：/index.html
        ********************************/
        void HistoryMode(const String& _reqPath);
    private:
        virtual void OnRecvData(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnAccepted(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnChannelError(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnChannelWrite(std::shared_ptr<neapu::NetChannel> _client) override {};

    private:
        Router m_router;
        bool m_log = false;
        struct {
            String m_reqPath;
            String m_filePath;
            bool m_enableRelativePath;
        } m_staticPath;
        String m_HistoryPath;
        
    };
}