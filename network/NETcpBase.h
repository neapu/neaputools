#pragma once
#include "network_pub.h"
#include "NENetChannel.h"
#include <map>
#include <memory>
#include <vector>
#include <thread>
#include <functional>

#define ERROR_SOCKET_OPEN       -1
#define ERROR_BIND              -2
#define ERROR_EVENT_BASE        -3
#define ERROR_LISTEN            -4
#define ERROR_EVENT_ADD         -5
#define ERROR_SOCKET_NONBLOCK   -6
#define ERROR_SET_REUSEADDR     -7
#define ERROR_CONNECT           -8
#define ERROR_DISPATCH          -9

#ifndef evutil_socket_t
#ifdef WIN32
#define evutil_socket_t intptr_t
#else
#define evutil_socket_t int
#endif // WIN32
#endif // !evutil_socket_t

struct sockaddr_in;
struct event_base;
namespace neapu {
class NEAPU_NETWORK_EXPORT TcpBase{
    friend void cbAccept(evutil_socket_t fd, short events, void* user_data);
    friend void cbSigInt(evutil_socket_t sig, short events, void* user_data);
    friend void cbClientRecv(evutil_socket_t fd, short events, void* user_data);
    friend class NetWorkThread;
public: 
    TcpBase() :  m_running(false), m_eb(nullptr), m_threadNum(0), m_err(0), m_userData(0) {}
    //作为服务端
    int start(int port, int nThreads);
    int GetLastError();
    String GetLastErrorString();
protected:
    virtual void OnRecvData(std::shared_ptr<neapu::NetChannel> _client) {}
    virtual void OnListened() {}
    virtual void OnAccepted(std::shared_ptr<neapu::NetChannel> _client) {}
    virtual void OnChannelClosed(std::shared_ptr<neapu::NetChannel> _client) {}
    virtual void OnChannelError(std::shared_ptr<neapu::NetChannel> _client) {}

    virtual int OnFdReadReady(int _fd);
    virtual void OnSignalInt();
    void OnListenerAccept(int fd);
    static std::shared_ptr<NetChannel> MakeChannel(int fd, sockaddr_in& sin);
    void AddClient(int fd, sockaddr_in& sin);
    void ReleaseClient(int fd);

    void SetLastError(int _err, String _errstr);
protected:
    int m_port = 0;
    uint64_t m_userData;//传给回调函数用的
    event_base* m_eb;
private:
    bool m_running;
    std::map<int,std::shared_ptr<NetChannel>> m_channels;
    int m_threadNum;
    std::vector<std::shared_ptr<NetWorkThread>> m_threadPoll;
    int m_err;
    String m_errstr;
    std::recursive_mutex m_channelMutex;
};
}
