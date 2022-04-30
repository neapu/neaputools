#pragma once
#ifdef _WIN32
    #ifdef NEAPU_BASE_EXPORTS
        #define NEAPU_BASE_EXPORT __declspec(dllexport)
    #else
        #define NEAPU_BASE_EXPORT __declspec(dllimport)
    #endif
#else
    #define NEAPU_BASE_EXPORT 
    // typedef unsigned long long size_t;
#endif
