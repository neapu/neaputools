#include "NEHttpHandle.h"
#include "NEByteStream.h"
#include "NENetChannel.h"
#include "NEDateTime.h"
#include "neapu-config.h"
#include <time.h>
using namespace neapu;

String HttpHandle::m_defaultContentType = "text/plain";

HttpHandle::HttpHandle(const HttpHandle& _handle)
{
    m_channel = _handle.m_channel;
    m_method = _handle.m_method;
    m_path = _handle.m_path;
    m_recvHeader = _handle.m_recvHeader;
    m_recvBody = _handle.m_recvBody;
    m_sendHeader = _handle.m_sendHeader;
    m_stateCode = _handle.m_stateCode;
    m_stateString = _handle.m_stateString;
}

int neapu::HttpHandle::AnalysisRequest(size_t _length)
{
    ByteStream data = (ByteStream)m_channel->Read(_length);
    String stateLine = data.ReadLineCRLF();
    auto stateLineDatas = stateLine.Split(' ');
    if (stateLineDatas.size() != 3) {
        return -1;
    }
    String method = stateLineDatas[0];
    if (method == "GET") {
        m_method = HttpMethod::GET;
    }
    else if (method == "POST") {
        m_method = HttpMethod::POST;
    }
    else {
        m_method = HttpMethod::ALL;
    }
    m_path = stateLineDatas[1];

    while (true) {
        String headLine = data.ReadLineCRLF();
        if (headLine.IsEmpty())break;
        size_t index = headLine.IndexOf(':');
        if (index == String::npos) {
            return -2;
        }
        String key = String::RemoveHeadAndTailSpace(headLine.Middle(0, index - 1));
        String value = String::RemoveHeadAndTailSpace(headLine.Middle(index + 1, String::npos));

        m_recvHeader[key] = value;
    }

    //接收body
    if (m_recvHeader.find("Content-Length") != m_recvHeader.end()) {
        size_t bodyLength = m_recvHeader["Content-Length"].ToUInt();
        m_recvBody = data.Read(bodyLength);
    }
    return 0;
}

int neapu::HttpHandle::AnalysisResponse(size_t _length)
{
    //未实现
    return 0;
}

String neapu::HttpHandle::GetRecvHeader(String _key)
{
    if (m_recvHeader.find(_key) != m_recvHeader.end()) {
        return m_recvHeader[_key];
    }
    return String();
}

void neapu::HttpHandle::SetSendHeader(String _key, String _value)
{
    m_sendHeader[_key] = _value;
}

void neapu::HttpHandle::SetSendHeader(String _key, int _value)
{
    SetSendHeader(_key, String::ToString(_value));
}

void neapu::HttpHandle::SetState(int _code, String _str)
{
    m_stateCode = _code;
    m_stateString = _str;
}

int neapu::HttpHandle::SendResponse(const ByteArray& _body)
{
    //状态行
    if (m_stateCode == 0) {
        m_stateCode = 200;
        m_stateString = "OK";
    }
    //添加一些基本的响应头
    if (m_sendHeader.find("Date") == m_sendHeader.end()) {
        SetSendHeader("Date", DateTime::CurrentDatetime().ToHttpDateTime());
    }
    if (m_sendHeader.find("Content-Length") == m_sendHeader.end() && !_body.IsEmpty()) {
        SetSendHeader("Content-Length", static_cast<int>(_body.Length()));
    }
    if (m_sendHeader.find("Content-Type") == m_sendHeader.end()) {
        SetSendHeader("Content-Type", m_defaultContentType);
    }
    if (m_sendHeader.find("Server") == m_sendHeader.end()) {
        SetSendHeader("Server", String("Neapu Http Server/") + NETOOLS_PROJECT_VERSION);
    }
    String stateLine = String("HTTP/1.1 %1 %2\r\n").Argument(m_stateCode).Argument(m_stateString);
    String headers;
    for (auto& item : m_sendHeader) {
        headers.Append(String("%1: %2\r\n").Argument(item.first).Argument(item.second));
    }
    //空行
    headers.Append("\r\n");
    return SendRow(ByteArray().Append(ByteArray(stateLine)).Append(ByteArray(headers)).Append(_body));
}

int neapu::HttpHandle::SendResponse(const char* _str)
{
    if (_str) {
        return SendResponse(ByteArray(_str, strlen(_str)));
    }
    return 0;
}

int neapu::HttpHandle::SendRow(const ByteArray& _content)
{
    return m_channel->Write(_content);
}

void neapu::HttpHandle::SetContentType(ContentType _contentType)
{
    String contentType;
    switch (_contentType)
    {
    case neapu::HttpHandle::ContentType::Text:
        contentType = "text/plain";
        break;
    case neapu::HttpHandle::ContentType::Html:
        contentType = "text/html";
        break;
    case neapu::HttpHandle::ContentType::Js:
        contentType = "application/x-javascript";
        break;
    case neapu::HttpHandle::ContentType::Json:
        contentType = "application/json";
        break;
    case neapu::HttpHandle::ContentType::Jpeg:
        contentType = "image/jpeg";
        break;
    case neapu::HttpHandle::ContentType::Gif:
        contentType = "image/gif";
        break;
    case neapu::HttpHandle::ContentType::Png:
        contentType = "image/png";
        break;
    default:
        contentType = "application/octet-stream";
        break;
    }
    SetSendHeader("Content-Type", contentType);
}

void neapu::HttpHandle::CloseConnetion()
{
    m_channel->Close();
}

void neapu::HttpHandle::SetDefaultContentType(String _ct)
{
    m_defaultContentType = _ct;
}
