#include "NETcpBase.h"
#include <event2/event.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <functional>
#include "NEString.h"
#include <event2/thread.h>
#include "NENetWorkThread.h"

#define BUF_SIZE 1024

using namespace neapu;

void neapu::cbSigInt(evutil_socket_t sig, short events, void* user_data)
{
    TcpBase* nb = static_cast<TcpBase*>(user_data);
    nb->OnSignalInt();
}

void neapu::cbAccept(evutil_socket_t fd, short events, void* user_data)
{
    TcpBase* base = static_cast<TcpBase*>(user_data);
    if (events & EV_READ) {
        base->OnListenerAccept(fd);
    }
}

void neapu::cbClientRecv(evutil_socket_t fd, short events, void* user_data)
{
    TcpBase* base = static_cast<TcpBase*>(user_data);
    if (base->OnFdReadReady(fd)<0 && base->m_eb) {
        event_base_loopbreak(base->m_eb);
    }
}

int TcpBase::start(int port, int nThreads)
{
    m_port = port;
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup error");
        return 0;
    }
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif // WIN32
    m_eb = event_base_new();
    if (!m_eb) {
        return ERROR_EVENT_BASE;
    }

    //处理SIGINT
    auto evSigInt = evsignal_new(m_eb, SIGINT, cbSigInt, this);
    if (!evSigInt || event_add(evSigInt, nullptr) < 0) {
        return ERROR_EVENT_ADD;
    }

    //初始化线程池
    m_threadNum = nThreads;
    for (int i = 0; i < m_threadNum; i++) {
        m_threadPoll.push_back(std::shared_ptr<NetWorkThread>(new NetWorkThread(this)));
    }

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd <= 0) {
        int err = evutil_socket_geterror(listenFd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_SOCKET_OPEN;
    }

    int rc = evutil_make_socket_nonblocking(listenFd);
    if (0 != rc) {
        return ERROR_SOCKET_NONBLOCK;
    }
    rc = evutil_make_listen_socket_reuseable(listenFd);
    if (0 != rc) {
        return ERROR_SET_REUSEADDR;
    }

    sockaddr_in sin = { 0 };
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    if (bind(listenFd, (sockaddr*)&sin, sizeof(sin)) < 0) {
        int err = evutil_socket_geterror(listenFd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_BIND;
    }

    if (listen(listenFd, 10) < 0) {
        int err = evutil_socket_geterror(listenFd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_LISTEN;
    }

    auto evListen = event_new(m_eb, listenFd, EV_READ | EV_PERSIST, neapu::cbAccept, this);
    if (!evListen || event_add(evListen, nullptr) < 0) {
        return ERROR_EVENT_ADD;
    }

    

    OnListened();

    //事件循环
    event_base_dispatch(m_eb);
    event_free(evListen);
    event_base_free(m_eb);
    m_eb = nullptr;

    return 0;
}

int neapu::TcpBase::GetLastError()
{
    return m_err;
}

String neapu::TcpBase::GetLastErrorString()
{
    return m_errstr;
}

int TcpBase::OnFdReadReady(int _fd)
{
    auto client = m_channels[_fd];
    char buf[BUF_SIZE];
    int readSize;
    for (;;) {
        readSize = recv(_fd, buf, BUF_SIZE, 0);
        if (readSize == EOF) { //接收完成
            int err = evutil_socket_geterror(_fd);
            if (err != 0 && err != 10035) { //对面意外掉线
                SetLastError(err, evutil_socket_error_to_string(err));
                client->Close();
                OnChannelClosed(client);
                ReleaseClient(_fd);
                return -1;
            }
            break;
        }
        else if (readSize == 0) { //对面主动断开
            int err = evutil_socket_geterror(_fd);
            if (err != 0) {
                SetLastError(err, evutil_socket_error_to_string(err));
            }
            client->Close();
            OnChannelClosed(client);
            ReleaseClient(_fd);
            return -1;
        }
        else if (readSize < 0) { //发生错误
            int err = evutil_socket_geterror(_fd);
            SetLastError(err, evutil_socket_error_to_string(err));
            client->SetLastError(err, evutil_socket_error_to_string(err));
            client->Close();
            OnChannelError(client);
            ReleaseClient(_fd);
            return -2;
        }
        client->AppendData(buf, readSize);
    }
    OnRecvData(client);
    if (client->IsClosed()) { //如果链接被关闭了就清理
        ReleaseClient(_fd);
    }
    return 0;
}

void neapu::TcpBase::OnSignalInt()
{
    if (m_eb) {
        event_base_loopbreak(m_eb);
    }
    for (int i = 0; i < m_threadNum; i++) {
        m_threadPoll[i]->Exit();
    }
}

void neapu::TcpBase::OnListenerAccept(int fd)
{
    sockaddr_in sin;
    int len = sizeof(sin);
    int accp_fd = accept(fd, (sockaddr*)&sin, &len);
    if (accp_fd <= 0) {
        return;
    }

    AddClient(accp_fd, sin);

    if (evutil_make_socket_nonblocking(accp_fd) < 0) {
        OnChannelClosed(m_channels[accp_fd]);
        ReleaseClient(accp_fd);
        return;
    }

    static int currentThread = 0;
    if (currentThread >= m_threadNum) {
        currentThread = 0;
    }
    if (m_threadPoll[currentThread]->dispatch(accp_fd) == 0) {
        OnAccepted(m_channels[accp_fd]);
    }
    
}

std::shared_ptr<NetChannel> neapu::TcpBase::MakeChannel(int fd, sockaddr_in& sin)
{
    return std::shared_ptr<NetChannel>(new NetChannel(fd, ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port)));
}

void neapu::TcpBase::AddClient(int fd, sockaddr_in& sin)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_channels.find(fd) != m_channels.end()) {
        ReleaseClient(fd);
    }
    m_channels[fd] = MakeChannel(fd, sin);
}

void neapu::TcpBase::ReleaseClient(int fd)
{
    std::unique_lock<std::recursive_mutex> lock(m_channelMutex);
    if (m_channels.find(fd) != m_channels.end()) {
        m_channels[fd]->Close();
        m_channels.erase(fd);
    }
}

void neapu::TcpBase::SetLastError(int _err, String _errstr)
{
    m_err = _err;
    m_errstr = _errstr;
}
