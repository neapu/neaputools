#pragma once
#include "http/NEHttpPublic.h"
#include "network/NETcpServer.h"
#include "http/NERouter.h"

namespace neapu {
    using HttpHandlePtr = std::shared_ptr<neapu::HttpHandle>;
    class NEAPU_HTTP_EXPORT HttpServer :public TcpServer {
    private:
        class StaticFileCache {
        public:
            void update(ByteArray&& _data, String&& _contentType);
            void update(const ByteArray& _data, const String& _contentType);
            ByteArray m_data;
            String m_contentType;
            time_t m_lastupd = 0;
        };

        
    public:
        using HttpServerCallback = std::function<void(std::shared_ptr<HttpHandle>)>;
        int Init(int _threadNum, const IPAddress& _addr);

        /*********************************
        * 设置HTTP请求路径回调
        * 路径支持在最后加通配符*
        * 例如：/home/ *
        * 不支持在路径中间加通配符
        *********************************/
        HttpServer& Get(const String& _path, HttpServerCallback);
        HttpServer& Post(const String& _path, HttpServerCallback);
        HttpServer& All(const String& _path, HttpServerCallback);
        void AddRouter(const String& _path, HttpMethod _method, HttpServerCallback _cb);

        /****************************************
        * 开启或关闭Http日志打印
        * 主要是Accepted、Closed等发生时的日志打印
        * 不会影响用户日志
        ****************************************/
        void HttpLog(bool _log = true);

        
        /******************************************************************
        * 启用静态文件服务器
        * _reqPath：静态文件的请求路径，例如/static/
        * _filePath: 静态文件所在的目录
        * _cacheSize: 静态文件缓存大小，0表示关闭缓存，每次都去读文件
        * _cacheTimeout: 静态文件缓存的超时时间，0表示永不超时
        * _enableRelativePath: 是否允许相对路径请求。建议禁止，防止夸目录攻击
        * 注意:   静态文件服务会把文件读到内存中再一次性发出去,
        *       如果文件太大会带来一些不可预知的问题(预计未来优化)
        *****************************************************************/
        void StaticPath(
            const String& _reqPath,
            const String& _filePath,
            const size_t _cacheSize = 0,
            const time_t _cacheTimeout = 0,
            bool _enableRelativePath = false
        );
        
        /********************************
        * 单页面应用的history模式
        * 当请求不存在的路径时返回指定文件
        * _filePath: 要返回的文件的路径
        ********************************/
        int HistoryMode(const String& _filePath);

        //接收并解析http请求(包括head和body)的大小,默认10240字节
        void SetHttpRequestReceiveSize(size_t _size);

        static void SetContentType(const String& _fileExtName, const String& _contentType)
        {
            m_contentTypeMap[_fileExtName] = _contentType;
        }
    protected:
        void OnRequestStaticFile(const String& _reqPath, std::shared_ptr<HttpHandle> _handle);

    private:
        virtual void OnRecvData(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnAccepted(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnChannelError(std::shared_ptr<neapu::NetChannel> _client) override;
        virtual void OnChannelWrite(std::shared_ptr<neapu::NetChannel> _client) override {};

    
        
    private:
        struct {
            String m_reqPath;
            String m_filePath;
            bool m_enableRelativePath;
        } m_staticPath;
        struct {
            bool enable = false;
            String path;
            
            StaticFileCache cache;
        } m_historyMode;

        bool m_log = false;
        Router m_router;
        static std::map<String, String> m_contentTypeMap;
        std::map<String, StaticFileCache> m_staticFileCache;
        size_t m_staticFileCacheSize = 0;
        size_t m_staticCacheTimeout = 0;
        size_t m_httpRequestSize = 10240;
    };
}