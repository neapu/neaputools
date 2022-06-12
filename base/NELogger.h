#pragma once
#include "base_pub.h"
#include "NEString.h"
#include <stdio.h>
#include <mutex>
#include <utility>
// #include "neapuobject.h"

#define LM_NONE     0
#define LM_DEADLY   1
#define LM_ERROR    2
#define LM_INFO     3
#define LM_DEBUG    4

#define LOG neapu::Logger

#define LOG_DEADLY neapu::Logger(LM_DEADLY)
#define LOG_ERROR neapu::Logger(LM_ERROR)
#define LOG_INFO neapu::Logger(LM_INFO)
#define LOG_DEBUG neapu::Logger(LM_DEBUG)

namespace neapu{
    class NEAPU_BASE_EXPORT Logger
    {
    public:
        Logger(int level);
        ~Logger();

        static void setLogLevel(int nLogLevel);
        static void setPrintLevel(int nPrintLevel);

        //template<class... Args>
        //Logger& operator<<(Args&&... args)
        //{
        //    m_data.Append(std::forward<Args>(args)...);
        //    return *this;
        //}
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