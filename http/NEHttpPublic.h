#pragma once
#ifdef _WIN32
#ifdef NEAPU_HTTP_EXPORTS
#define NEAPU_HTTP_EXPORT __declspec(dllexport)
#else
#define NEAPU_HTTP_EXPORT __declspec(dllimport)
#endif
#else
#define NEAPU_HTTP_EXPORT 
// typedef unsigned long long size_t;
#endif

enum class HttpMethod
{
    ALL,
    GET,
    POST
};