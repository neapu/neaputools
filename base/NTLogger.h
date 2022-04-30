#pragma once
#include "base_pub.h"
#include "NTString.h"
#include <stdio.h>
#include <mutex>
// #include "neapuobject.h"

#define LM_NONE     0
#define LM_DEADLY   1
#define LM_ERROR    2
#define LM_INFO     3
#define LM_DEBUG    4

#define LOG neapu::Logger

namespace neapu{
class NEAPU_BASE_EXPORT Logger
{
public:
    Logger(int level);
    ~Logger();

    static void setLogLevel(int nLogLevel);
    static void setPrintLevel(int nPrintLevel);

    Logger& operator<<(const String& str);
    // Logger& operator<<(const ByteArray& data);
    Logger& operator<<(const double n);
    Logger& operator<<(const int n);
    Logger& operator<<(const long long int n);
    Logger& operator<<(const unsigned int n);
    Logger& operator<<(const unsigned long long int n);

private:
    static bool openFile();
    static const char* getLevelFlag(int level, bool bColor);

private:
    int m_nLevel;
    String m_data;

    static int m_nLogLevel;
    static int m_nPrintLevel;
    static FILE* m_pFile;
    static String m_strLogDate;
    static std::mutex m_fileMutex;
};
}