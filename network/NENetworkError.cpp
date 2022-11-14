#include "NENetworkError.h"
using namespace neapu;

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