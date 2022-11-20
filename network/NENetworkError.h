#pragma once
#include <base/NEString.h>
#include <logger/logger.h>
#include "network/network_pub.h"

#define ERROR_SOCKET_OPEN -1
#define ERROR_BIND -2
#define ERROR_EVENT_BASE -3
#define ERROR_LISTEN -4
#define ERROR_EVENT_ADD -5
#define ERROR_SOCKET_NONBLOCK -6
#define ERROR_SET_REUSEADDR -7
#define ERROR_CONNECT -8
#define ERROR_DISPATCH -9
#define ERROR_SET_WIN_THREAD -10
#define ERROR_EB_NULLPTR -11
#define ERROR_EVENT_NEW -12
#define ERROR_EVENT_UNINIT -13
#define ERROR_WSASTARTUP -14
#define ERROR_ADD_SOCKET -15

namespace neapu {
using NetworkError = struct tagNetworkError {
    int code = 0;
    String str;
};

} // namespace neapu
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, const neapu::NetworkError& _err);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetworkError& _err);
NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetworkError&& _err);