#include "network/network_pub.h"
#include "base/NELogger.h"
#ifdef WIN32
#else
#include <fcntl.h>
#endif

int neapu::GetSocketError(SOCKET_FD sock)
{
#ifdef __linux__
    return errno;
#endif
}

const char* neapu::GetErrorString(int err)
{
#ifdef __linux__
    return strerror(err);
#endif
}

int neapu::SetSocketNonBlock(SOCKET_FD fd)
{
#ifdef _WIN32
    {
        unsigned long nonblocking = 1;
        if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
            event_sock_warn(fd, "ioctlsocket(%d, FIONBIO, &%lu)", (int)fd, (unsigned long)nonblocking);
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