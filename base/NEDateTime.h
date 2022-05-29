#pragma once
#include "NEString.h"
#include "base_pub.h"

namespace neapu {
    class NEAPU_BASE_EXPORT DateTime
    {
    public:
        DateTime() {}
        DateTime(const DateTime& t) noexcept { m_time = t.m_time; }
        DateTime(DateTime&& t) noexcept { m_time = t.m_time; }
        static DateTime CurrentDatetime();

        //返回整形的时间，例如11:24:18 返回112418
        int GetTime();
        //返回整形日期，例如2021-01-23 返回20210123
        int GetDate();

        neapu::String ToString(neapu::String fmt = neapu::String());
        String ToHttpDateTime();
    private:
        unsigned long long m_time = 0;
        static StringList m_weeks;
        static StringList m_months;
    };
}