#pragma once
#include <memory>
#include <map>
#include "NEHttpPublic.h"
#include <NEString.h>
#include <NEByteArray.h>

namespace neapu {
    class NetChannel;
    class HttpHandle {
    public:
        HttpHandle(std::shared_ptr<NetChannel> _channel) : m_channel(_channel) {}

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
        void SetSendHeader(String _key, String _value);
        void SetSendHeader(String _key, int _value);
        void SetState(int _code, String _str);
        
        /**********************************
        * 向Http客户端发送数据
        * 使用这个方法会带上HTTP头
        **********************************/
        int SendResponse(const ByteArray& _body);

        /**********************************
        * 向远端发送数据
        * 这个方法会发送原始数据，不会带上HTTP头
        **********************************/
        int SendRow(const ByteArray& _content);

        String Path() const { return m_path; }
        void SetPath(String _path) { m_path = _path; }
        HttpMethod Method() const { return m_method; }
        void SetMethod(HttpMethod _method) { m_method = _method; }
    public:
        /**********************************
        * 设定默认Content-Type
        * SendResponse会在没有显示设置Content-Type时往header添加一个默认的Content-Type
        * 这个函数设定的就是这个默认值
        * 如果没有设置则是 text/plain
        **********************************/
        static void SetDefaultContentType(String _ct);
    private:
        std::shared_ptr<NetChannel> m_channel;
        HttpMethod m_method = HttpMethod::ALL;
        String m_path;
        std::map<String, String> m_recvHeader;
        ByteArray m_recvBody;
        std::map<String, String> m_sendHeader;
        int m_stateCode = 0;
        String m_stateString;
        static String m_defaultContentType;
    };
}