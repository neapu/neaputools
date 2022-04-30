#include "NTNetBase.h"
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <functional>
#include "NTString.h"

#define BUF_SIZE 1024

using namespace neapu;
#define MAX_EVENT 50

#ifdef _WIN32
static int write(int _fd, const char* _buf, size_t _n)
{
    return send(_fd, _buf, _n, 0);
}
#endif

static int g_nFdClose;//管道fd用于退出程序
static void exit_signal(int signo)
{
    write(g_nFdClose, "c", 1);
}

#ifdef _WIN32
static int _dgram_socketpair(int fds[2])
{
    int nSend, nRecv;

    struct sockaddr_in sin = { 0 };
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int len = sizeof(sin);
    
    nRecv = socket(AF_INET, SOCK_DGRAM, 0);
    if (nRecv < 0)return nRecv;

    nSend = socket(AF_INET, SOCK_DGRAM, 0);
    if (nSend < 0)return nSend;

    int rc = bind(nRecv, (struct sockaddr*)&sin, sizeof(sin));
    if (rc < 0)return rc;

    rc = getsockname(nRecv, (struct sockaddr*)&sin, &len);
    if (rc < 0)return rc;
    
    rc = connect(nSend, (struct sockaddr*)&sin, len);
    if (rc < 0)return rc;

    fds[0] = nRecv;
    fds[1] = nSend;
    return 0;
}
#endif

static int make_socket_non_blocking(int fd)
{
#ifdef _WIN32
    unsigned long nonBlock = 1;
    return ioctlsocket(fd, FIONBIO, &nonBlock);
#else
    int flags, s;
    // 获取当前flag
    flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) {
        perror("Get fd status");
        return -1;
    }

    flags |= O_NONBLOCK;

    // 设置flag
    s = fcntl(fd, F_SETFL, flags);
    if (-1 == s) {
        perror("Set fd status");
        return -1;
    }
    return 0;
#endif
}

static int SetReuseAddr(int fd)
{
#ifdef WIN32
#else
    int flag=1;
    int len = sizeof(int);
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, len);
#endif
}

int NetBase::start(int port, int nThreads)
{
    m_port = port;

    //接收处理ctrl+c信号
    signal(SIGINT, exit_signal);
    //管道(配对socket)用于在事件循环中接收ctrl+c事件
    int extfd[2];
#ifdef _WIN32
    if(_dgram_socketpair(extfd) < 0) {
        perror("pipe error");
        exit(-1);
    }
#else
    if(pipe(extfd)<0) {
        perror("pipe error");
        _exit(-1);
    }
#endif
    g_nFdClose = extfd[1];
    
    //初始化线程池
    m_running = true;
    m_threadPoll.Init(nThreads, std::bind(&NetBase::WorkThread, this));
#ifdef _WIN32
    
#else
    //listen socket epoll事件对象ev和accept socket epoll事件对象数组event
    struct epoll_event ev, event[MAX_EVENT];
#endif
    //初始化socket
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd == -1) {
        // fprintf(stderr, "socket open error\n");
        return ERROR_SOCKET_OPEN;
    }

    //启用端口重入
    if(SetReuseAddr(m_fd) == -1) {
        return ERROR_SET_REUSEADDR;
    }

    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(port);

    if (bind(m_fd, (struct sockaddr *)&sockAddr, sizeof(sockaddr)) == -1) {
        // fprintf(stderr, "socket bind error\n");
        return ERROR_BIND;
    }

    if (make_socket_non_blocking(m_fd) == -1) {
        // fprintf(stderr, "socket set non blocking error\n");
        return ERROR_SET_NON_BLOCKING;
    }

    //创建epoll实例
    m_epfd = epoll_create1(0);
    if (m_epfd == 1)
    {
        // fprintf(stderr, "epoll create error\n");
        return ERROR_CREATE_EPOLL;
    }

    ev.data.fd = m_fd;
    ev.events = EPOLLIN | EPOLLET; //边缘触发
    //设置epoll事件
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_fd, &ev) == -1) {
        // fprintf(stderr, "epoll ctl error\n");
        return ERROR_EPOLL_CTL;
    }

    //绑定管道
    ev.data.fd = extfd[0];
    ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, extfd[0], &ev) == -1) {
        // fprintf(stderr, "epoll ctl error\n");
        return ERROR_EPOLL_CTL;
    }


    if (listen(m_fd, 200) == -1) {
        // fprintf(stderr, "socket listen error\n");
        return ERROR_LISTEN;
    }

    OnListened();

    for(;;) {
        int nWaitCount = epoll_wait(m_epfd, event, MAX_EVENT, -1);

        for(int i=0;i<nWaitCount;i++) {
            auto events = event[i].events;
            auto socketfd = event[i].data.fd;
            if(socketfd == extfd[0] && (events & EPOLLIN)) {
                // fprintf(stderr, "recv exit signal\n");
                close(m_fd);
                m_running = false;
                //等待线程退出
                m_threadPoll.Join();
                close(m_epfd);
                return 0;
            }
            if (events & EPOLLERR || events & EPOLLHUP || (!(events & EPOLLIN))) {
                epoll_ctl(m_epfd, EPOLL_CTL_DEL, socketfd, NULL);
                m_clients[socketfd]->Close();
                m_clients.erase(socketfd);
                continue;
            }
            else if (socketfd == m_fd) {
                for (;;) {
                    struct sockaddr_in in_addr = {0};
                    socklen_t in_addr_len = sizeof(in_addr);
                    int accp_fd = 0;
                    accp_fd = accept(m_fd, (sockaddr*)&in_addr, &in_addr_len);
                    
                    if (accp_fd == -1) {
                        break;
                    }

                    if (make_socket_non_blocking(accp_fd) == -1) {
                        // fprintf(stderr, "socket set non blocking error\n");
                        return ERROR_SET_NON_BLOCKING;
                    }

                    //todo: 添加客户端连接回调
                    //自定义client类
                    m_clients[accp_fd] = std::shared_ptr<NetClient>(new NetClient(accp_fd, ntohl(in_addr.sin_addr.s_addr), ntohs(in_addr.sin_port)));
                    OnAccept(m_clients[accp_fd]);

                    struct epoll_event ev;
                    ev.events = EPOLLIN|EPOLLET;
                    ev.data.fd = accp_fd;
                    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, accp_fd, &ev) == -1) {
                        // fprintf(stderr, "epoll ctl error\n");
                        return ERROR_EPOLL_CTL;
                    }
                }
            }
            else {
                m_fdQueue.enqueue(socketfd);
            }
        }
    }
    return 0;
}

void NetBase::OnFdReadReady(int _fd)
{
    auto client = m_clients[_fd];
    char buf[BUF_SIZE];
    int readSize;
    for(;;) {
        readSize = recv(_fd, buf, BUF_SIZE, 0);
        if(readSize==EOF) { //接收完成
            break;
        }
        else if(readSize == 0) { //用户主动断开
            epoll_ctl(m_epfd, EPOLL_CTL_DEL, _fd, NULL);
            client->Close();
            OnClientClosed(client);
            m_clients.erase(_fd);
            return;
        }
        else if(readSize<0) { //发生错误
            client->SetLastError(errno, strerror(errno));
            epoll_ctl(m_epfd, EPOLL_CTL_DEL, _fd, NULL);
            client->Close();
            OnClientClosed(client);
            m_clients.erase(_fd);
            return;
        }
        client->AppendData(buf, readSize);
    }
    OnRecvRequest(client);
    if(client->IsClosed()) { //如果链接被关闭了就清理
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, _fd, NULL);
        m_clients.erase(_fd);
    }
}

void NetBase::WorkThread()
{
    while(m_running) {
        int fd;
        if(m_fdQueue.dequeue(fd)) {
            OnFdReadReady(fd);
        }
    }
}