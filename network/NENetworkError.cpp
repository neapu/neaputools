#include "NENetworkError.h"
using namespace neapu;

Logger& neapu::operator<<(Logger& _logger, const NetworkError& _err)
{
	return _logger << "[" << _err.code << "][" << _err.str << "]";
}