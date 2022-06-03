#pragma once
#ifdef _WIN32
#ifdef NEAPU_READ_CONNECTOR_EXPORTS
#define NEAPU_READ_CONNECTOR_EXPORT __declspec(dllexport)
#else
#define NEAPU_READ_CONNECTOR_EXPORT __declspec(dllimport)
#endif
#else
#define NEAPU_READ_CONNECTOR_EXPORT 
// typedef unsigned long long size_t;
#endif