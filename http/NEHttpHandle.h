#pragma once
#include <memory>
#include <map>
#include "NEHttpPublic.h"
#include <NEString.h>
#include <NEByteArray.h>
#include <utility>
#include <vector>
#include <set>
namespace neapu {
    struct Cookie{
        String key;
        String value;
        int maxAge = -1;
        String domain;
        String path;
        bool secure = false;
        bool httpOnly = false;
    };
    class NetChannel;
    class NEAPU_HTTP_EXPORT HttpHandle {
    public:
        enum class ContentType:char
        {
            Text,
            Html,
            Js,
            Json,
            Jpeg,
            Gif,
            Png
        };
    public:
        HttpHandle(std::shared_ptr<NetChannel> _channel) : m_channel(_channel) {}
        HttpHandle(const HttpHandle&);

        /********************************
        * 作为服务端
        * 接收并解析HTTP请求头
        * _length: 最大接收长度
        *********************************/
        int AnalysisRequest(size_t _length);
        /********************************
        * 作为客户端
        * 接收并解析HTTP响应头
        * _length: 最大接收长度
        *********************************/
        int AnalysisResponse(size_t _length);

        String GetRecvHeader(String _key);
        void AddSendHeader(String _key, String _value);
        void AddSendHeader(String _key, int _value);
        void SetState(int _code, String _str);
        
        /**********************************
        * 向Http客户端发送数据
        * 使用这个方法会带上HTTP头
        **********************************/
        int SendResponse(const ByteArray& _body);
        int SendResponse(const char* _str);

        /**********************************
        * 向远端发送数据
        * 这个方法会发送原始数据，不会带上HTTP头
        **********************************/
        int SendRow(const ByteArray& _content);

        void SetContentType(ContentType _contentType);

        void CloseConnetion();

        String Path() const { return m_path; }
        void SetPath(const String& _path) { m_path = _path; }
        HttpMethod Method() const { return m_method; }
        void SetMethod(HttpMethod _method) { m_method = _method; }

        ByteArray GetRecvBody() { return m_recvBody; }

        void AddCookie(const Cookie& _cookie);
        String GetCookie(const String& _key);
    public:
        /**********************************
        * 设定默认Content-Type
        * SendResponse会在没有显示设置Content-Type时往header添加一个默认的Content-Type
        * 这个函数设定的就是这个默认值
        * 如果没有设置则是 text/plain
        **********************************/
        static void SetDefaultContentType(const String& _ct);
    private:
        std::shared_ptr<NetChannel> m_channel;
        HttpMethod m_method = HttpMethod::ALL;
        String m_path;
        std::map<String, String> m_recvHeader;
        std::map<String, String> m_recvCookies;
        ByteArray m_recvBody;
        // std::map<String, String> m_sendHeader;
        std::vector<std::pair<String, String>> m_sendHeader;
        std::set<String> m_sendHeaderKeys;
        int m_stateCode = 0;
        String m_stateString;
        static String m_defaultContentType;
    };
}