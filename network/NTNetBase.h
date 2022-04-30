#pragma once
#include "network_pub.h"
#include "NTNetClient.h"
#include <map>
#include <memory>
#include <vector>
#include <thread>
#include <functional>
#include "NTSafeQueue.h"
#include "NTThreadPoll.h"

#define ERROR_SOCKET_OPEN       -1
#define ERROR_BIND              -2
#define ERROR_SET_NON_BLOCKING  -3
#define ERROR_LISTEN            -4
#define ERROR_CREATE_EPOLL      -5
#define ERROR_EPOLL_CTL         -6
#define ERROR_SET_REUSEADDR     -7


namespace neapu {
class NetBase{
public: 
    NetBase() : m_fd(0), m_running(false) {}
    int start(int port, int nThreads);
protected:
    virtual void OnRecvRequest(std::shared_ptr<neapu::NetClient> _client) = 0;
    virtual void OnListened() {}
    virtual void OnAccept(std::shared_ptr<neapu::NetClient> _client) {}
    virtual void OnClientClosed(std::shared_ptr<neapu::NetClient> _client) {}
    virtual void OnClientError(std::shared_ptr<neapu::NetClient> _client) {}
private:
    void OnFdReadReady(int _fd);
    void WorkThread();
protected:
    int m_port = 0;
private:
    int m_fd;
    bool m_running;
#ifdef WIN32
#else
    int m_epfd;
#endif //WIN32
    SafeQueue<int> m_fdQueue;
    ThreadPoll<std::function<void()>> m_threadPoll;
    std::map<int,std::shared_ptr<NetClient>> m_clients;
};
}
