#pragma once
#ifdef _WIN32
    #ifdef NEAPU_NETWORK_EXPORTS
        #define NEAPU_NETWORK_EXPORT __declspec(dllexport)
    #else
        #define NEAPU_NETWORK_EXPORT __declspec(dllimport)
    #endif
#else
    #define NEAPU_NETWORK_EXPORT 
    // typedef unsigned long long size_t;
#endif
//#define check1(x, p) if(!(x)) {perror(p);exit(-1);}

#ifdef _WIN32
#include <basetsd.h>
using SOCKET_FD = UINT_PTR;
#else
using SOCKET_FD = int;
#endif
namespace neapu{
int GetSocketError(SOCKET_FD sock);
const char* GetErrorString(int err);
int SetSocketNonBlock(SOCKET_FD fd);
int SetSocketReuseable(SOCKET_FD sock);
}