#include "NETcpClient.h"
#include <event2/event.h>
#include <functional>

#define BUF_SIZE 1024

int neapu::TcpClient::Connect(String _IPAddr, int _port,
    const RecvDataCallbackCli& _recvCb, 
    const ConnectedCallback& _connCb, 
    uint64_t _userData
)
{
    m_userData = _userData;
    m_recvCb = _recvCb;
    m_connectCb = _connCb;
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup error");
        return 0;
    }
#endif
    m_eb = event_base_new();
    if (!m_eb) {
        return ERROR_EVENT_BASE;
    }

    m_workThread = std::thread(std::bind(&TcpClient::WorkThread, this));

    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd <= 0) {
        int err = evutil_socket_geterror(m_fd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_SOCKET_OPEN;
    }

    sockaddr_in sin = { 0 };
    sin.sin_family = AF_INET;
    evutil_inet_pton(AF_INET, _IPAddr.data(), &sin.sin_addr);
    sin.sin_port = htons(_port);

    if (::connect(m_fd, (sockaddr*)&sin, sizeof(sin)) < 0) {
        int err = evutil_socket_geterror(m_fd);
        SetLastError(err, evutil_socket_error_to_string(err));
        return ERROR_CONNECT;
    }

    int rc = evutil_make_socket_nonblocking(m_fd);
    if (0 != rc) {
        return ERROR_SOCKET_NONBLOCK;
    }

    auto ev = event_new(m_eb, m_fd, EV_READ | EV_PERSIST, neapu::cbClientRead, this);
    if (!ev) {
        if (ev)event_free(ev);
        return -1;
    }
    event_add(ev, nullptr);

    std::unique_lock<std::mutex> lck(m_workThreadMutex);
    m_workThreadCond.wait(lck);
    
    m_channel = MakeChannel(m_fd, sin);
    OnConnected();
    if (m_connectCb) {
        m_connectCb(m_userData);
    }

    return 0;
}

void neapu::TcpClient::Close()
{
    event_base_loopbreak(m_eb);
    if (m_workThread.joinable()) {
        m_workThread.join();
    }
    m_channel->Close();
    OnChannelClosed(m_channel);
    m_fd = 0;
    m_channel.reset();
    event_base_free(m_eb);
    m_eb = nullptr;
}

void neapu::TcpClient::Send(const ByteArray& data)
{
    if (m_channel) {
        m_channel->Write(data);
    }
}

int neapu::TcpClient::OnFdReadReady(int _fd)
{
    ByteArray data;
    char buf[BUF_SIZE];
    int readSize;
    for (;;) {
        readSize = recv(_fd, buf, BUF_SIZE, 0);
        if (readSize == EOF) { //接收完成
            int err = evutil_socket_geterror(_fd);
            if (err != 0 && err != 10035) { //对面意外掉线
                Close();
            }
            break;
        }
        else if (readSize == 0) { //对面主动断开
            Close();
            
            return 0;
        }
        else if (readSize < 0) { //发生错误
            OnChannelError(m_channel);
            Close();
            return 0;
        }
        data.append(buf, readSize);
    }
    OnRecvData(data);
    if (m_recvCb) {
        m_recvCb(data, m_userData);
    }

    return 0;
}

void neapu::TcpClient::WorkThread()
{
    m_workThreadCond.notify_all();
    event_base_dispatch(m_eb);
}

void neapu::cbClientRead(evutil_socket_t fd, short events, void* user_data)
{
    static_cast<neapu::TcpClient*>(user_data)->OnFdReadReady(fd);
}
