#include "network/network_pub.h"
#include "base/NELogger.h"
#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include <fcntl.h>
#endif

int neapu::GetSocketError(SOCKET_FD sock)
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

const char* neapu::GetErrorString(int err)
{
#ifdef _WIN32
    HLOCAL LocalAddress=NULL;  
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,  
         NULL,(DWORD)err,0,(PTSTR)&LocalAddress,0,NULL);  
    return (LPSTR)LocalAddress; 
#else
    return strerror(err);
#endif
}

int neapu::SetSocketNonBlock(SOCKET_FD fd)
{
#ifdef _WIN32
    {
        unsigned long nonblocking = 1;
        if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
            LOG_INFO << "ioctlsocket ERROR";
            return -1;
        }
    }
#else
    {
        int flags;
        if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
            LOG_INFO << "fcntl(fd, F_GETFL) ERROR";
            return -1;
        }
        if (!(flags & O_NONBLOCK)) {
            if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                LOG_INFO << "fcntl(fd, F_GETFL) ERROR";
                return -1;
            }
        }
    }
#endif
    return 0;
}

int neapu::SetSocketReuseable(SOCKET_FD sock)
{
#if defined(SO_REUSEADDR) && !defined(_WIN32)
    int one = 1;
    /* REUSEADDR on Unix means, "don't hang on to this address after the
     * listener is closed."  On Windows, though, it means "don't keep other
     * processes from binding to this address while we're using it. */
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&one,
                      (socklen_t)sizeof(one));
#else
    return 0;
#endif
}