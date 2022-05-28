#include "NEDateTime.h"
#include "NEString.h"
#include "time.h"
using namespace neapu;

StringList DateTime::m_weeks = { "Sun","Mon","Tues","Wed","Thur","Fri","Sat" };
StringList DateTime::m_months = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec" };

DateTime DateTime::CurrentDatetime()
{
    DateTime rst;
    rst.m_time = time(nullptr);
    return rst;
}

int DateTime::GetTime()
{
    time_t _t = m_time;
    auto _tm = localtime(&_t);
    int rst = 0;
    rst += _tm->tm_hour;
    rst *= 100;
    rst += _tm->tm_min;
    rst *= 100;
    rst += _tm->tm_sec;
    return rst;
}

int DateTime::GetDate()
{
    time_t _t = m_time;
    auto _tm = localtime(&_t);
    int rst = 0;
    rst += _tm->tm_year + 1900;
    rst *= 10000;
    rst += _tm->tm_mon + 1;
    rst *= 100;
    rst += _tm->tm_mday;
    return rst;
}

neapu::String DateTime::ToString(neapu::String fmt)
{
    if (fmt.IsEmpty())fmt = "%Y-%M-%d %h:%m:%s";
    time_t _t = m_time;
    auto _tm = localtime(&_t);
    fmt.Replace("%Y", String::ToString(_tm->tm_year + 1900));
    fmt.Replace("%y", String::ToString((_tm->tm_year + 1900) % 100));
    char szTemp[4];
    sprintf(szTemp, "%02d", _tm->tm_mon + 1);
    fmt.Replace("%M", szTemp);
    sprintf(szTemp, "%02d", _tm->tm_mday);
    fmt.Replace("%d", szTemp);
    sprintf(szTemp, "%02d", _tm->tm_hour);
    fmt.Replace("%h", szTemp);
    sprintf(szTemp, "%02d", _tm->tm_min);
    fmt.Replace("%m", szTemp);
    sprintf(szTemp, "%02d", _tm->tm_sec);
    fmt.Replace("%s", szTemp);
    return fmt;
}

String neapu::DateTime::ToHttpDateTime()
{
    time_t _t = m_time;
    auto _tm = localtime(&_t);
    return String("%1, %2 %3 %4 %5:%6:%7 GMT")
        .Argument(m_weeks[_tm->tm_wday]).Argument(_tm->tm_mday).Argument(m_months[_tm->tm_mon]).Argument(_tm->tm_year)
        .Argument(_tm->tm_hour).Argument(_tm->tm_min).Argument(_tm->tm_sec);
}
