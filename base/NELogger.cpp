#include "NELogger.h"
#include <cstdio>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <string>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
using namespace neapu;
int Logger::m_nLogLevel = LM_NONE;
int Logger::m_nPrintLevel = LM_DEBUG;
FILE* Logger::m_pFile = nullptr;
String Logger::m_strLogDate;
std::mutex Logger::m_fileMutex;

Logger::Logger(int level)
    : m_nLevel(level)
{

}

Logger::~Logger()
{
    std::time_t now_c = time(nullptr);
    char temp[64];
    strftime(temp,64,"[%Y-%m-%d %H:%M:%S]",localtime(&now_c));
    if(m_nLevel<=m_nLogLevel)
    {
        if(openFile())
        {
            std::unique_lock<std::mutex> locker(m_fileMutex);
            fprintf(m_pFile, "%s%s%s\n", temp, getLevelFlag(m_nLevel, false), m_data.ToCString());
            fflush(m_pFile);
        }
    }
    if(m_nLevel<=m_nPrintLevel)
    {
        fprintf(stderr, "%s%s%s\n", temp, getLevelFlag(m_nLevel, true), m_data.ToCString());
    }
}

void Logger::setLogLevel(int nLogLevel)
{
    const char* path = "./log";
#ifdef _WIN32
    if(0 != _access(path, 0))
    {
        if(0 != _mkdir(path))
#else
    if(0 != access(path, 0))
    {
        if(0 != mkdir(path, 0744))
#endif
        {
            perror("mkdir");
            return;
        }
    }
    m_nLogLevel = nLogLevel;
}

void Logger::setPrintLevel(int nPrintLevel)
{
    m_nPrintLevel = nPrintLevel;
}

Logger& Logger::operator<<(const String& str)
{
    m_data.Append(str);
    return (*this);
}

// Logger& Logger::operator<<(const ByteArray& data)
// {
//     m_data.append(data.to_std_string());
//     return (*this);
// }

Logger& Logger::operator<<(const int n)
{
    m_data.Append(n);
    return (*this);
}

Logger& Logger::operator<<(const double n)
{
    m_data.Append(n);
    return (*this);
}

Logger& Logger::operator<<(const long long int n)
{
    m_data.Append(n);
    return (*this);
}
Logger& Logger::operator<<(const unsigned int n)
{
    m_data.Append(n);
    return (*this);
}
Logger& Logger::operator<<(const unsigned long long int n)
{
    m_data.Append(n);
    return (*this);
}


bool Logger::openFile()
{
    std::time_t now_c = time(nullptr);
    char temp[64];
    strftime(temp,64,"%Y-%m-%d",localtime(&now_c));
    if(m_pFile)
    {
        if(m_strLogDate==temp) return true;
    }
    m_strLogDate=temp;
    char szNewFile[128];
    sprintf(szNewFile, "log/%s.log", temp);
    if(m_pFile) fclose(m_pFile);
    m_pFile = fopen(szNewFile, "a");
    if(!m_pFile)return false;
    return true;
}

const char* Logger::getLevelFlag(int level, bool bColor)
{
    if(bColor)
    {
        switch (level) 
        {
        case LM_DEADLY: return "\033[0;35;40m[Deadly]\033[0m";
        case LM_ERROR:  return "\033[0;31;40m[Error]\033[0m";
        case LM_INFO:   return "\033[0;32;40m[Info]\033[0m";
        case LM_DEBUG:  return "\033[0;34;40m[Debug]\033[0m";
        }
    }
    else
    {
        switch (level) 
        {
        case LM_DEADLY: return "[Deadly]";
        case LM_ERROR:  return "[Error]";
        case LM_INFO:   return "[Info]";
        case LM_DEBUG:  return "[Debug]";
        }
    }
    
    return "[Unknow]";
}