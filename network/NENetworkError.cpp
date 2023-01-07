#include "NENetworkError.h"
#include "network/NENetworkError.h"
#include "network/network_pub.h"
using namespace neapu;

void NetworkError::SetLastError()
{
    this->code = GetSocketError(0);
    this->str = GetErrorString(code);
}

Logger& operator<<(Logger& _logger, const NetworkError& _err)
{
    return _logger << "[" << _err.code << "][" << _err.str << "]";
}

NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetworkError& _err)
{
    return _logger << "[" << _err.code << "][" << _err.str << "]";
}

NEAPU_NETWORK_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::NetworkError&& _err)
{
    return _logger << "[" << _err.code << "][" << _err.str << "]";
}