#ifndef __NETCPSOCKET_H__
#define __NETCPSOCKET_H__

#include "network/NENetChannel.h"
#include <memory>
namespace neapu{
    using TcpSocket = NetChannel;
    using TcpSocketPtr = std::shared_ptr<TcpSocket>;
}
#endif // __NETCPSOCKET_H__