#pragma once
#include "NTByteArray.h"
#include <mutex>
#include <memory>
#include "NTString.h"
namespace neapu{
class Logger;
class NetClient{
    friend class NetBase;
    friend Logger& operator<<(Logger& _logger, const NetClient& _netclient);
public:
    enum class InetType : char {
        IPV4,
        IPV6
    };
    ByteArray Read(size_t _len);
    ByteArray ReadAll();
    void Write(ByteArray _data);
    void Close();
    bool IsClosed();
    void SetUserData(std::shared_ptr<void*> _userData);
    std::shared_ptr<void*> GetUserData();
    unsigned int GetIPv4();
    String GetIPv4String();
    int GetPort();
    InetType GetInetType();
private:
    NetClient(int _fd, unsigned int _ipv4, int _port) : m_fd(_fd), m_ipv4(_ipv4), m_port(_port), m_inetType(InetType::IPV4) {}
    void AppendData(ByteArray _data);
    void AppendData(const char* _data, size_t _len);
    void SetLastError(int _err, String _str);
private:
    int m_fd;
    ByteArray m_readBuffer;
    std::mutex m_bufferLock;
    std::mutex m_writeLock;
    std::shared_ptr<void*> m_userData;
    unsigned int m_ipv4;
    int m_port;
    InetType m_inetType;
    int m_err;
    String m_errString;
};
Logger& operator<<(Logger& _logger, const NetClient& _netclient);
}