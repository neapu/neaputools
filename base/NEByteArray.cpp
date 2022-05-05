#include "NEByteArray.h"
#include <clocale>
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <string.h>

#ifdef WIN32

void*
memmem(const void* l, size_t l_len, const void* s, size_t s_len)
{
    register char* cur, * last;
    const char* cl = (const char*)l;
    const char* cs = (const char*)s;

    /* we need something to compare */
    if (l_len == 0 || s_len == 0)
        return NULL;

    /* "s" must be smaller or equal to "l" */
    if (l_len < s_len)
        return NULL;

    /* special case where s_len == 1 */
    if (s_len == 1)
        return (void*)memchr(l, (int)*cs, l_len);

    /* the last position where its possible to find "s" in "l" */
    last = (char*)cl + l_len - s_len;

    for (cur = (char*)cl; cur <= last; cur++)
        if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
            return cur;

    return NULL;
}
#endif // WIN32


using namespace neapu;

size_t ByteArray::npos=(size_t)(-1);

#define BASE_LEN 1024
ByteArray::ByteArray() noexcept
    : m_max(BASE_LEN)
    , m_data(nullptr)
    , m_len(0)
{
    m_data = static_cast<char*>(malloc(BASE_LEN));
    memset(m_data, 0, BASE_LEN);
}

ByteArray::ByteArray(const ByteArray& data) noexcept : ByteArray()
{
    append(data);
}

ByteArray::ByteArray(ByteArray&& data) noexcept
{
    m_data = data.m_data;
    data.m_data = nullptr;
    m_len = data.m_len;
    data.m_len = 0;
    m_max = data.m_max;
    data.m_max = 0;
}

ByteArray::ByteArray(const char* data, size_t len) : ByteArray()
{
    append(data, len);
}

ByteArray::~ByteArray() 
{
    if(m_data)free(m_data);
}

ByteArray& ByteArray::append(const ByteArray& data) 
{
    this->append(data.m_data, data.m_len);
    return *this;
}

ByteArray& ByteArray::append(const char* data, size_t len) 
{
    if(len==0)return (*this);
    if(len+m_len>=m_max)
    {
        extend(len+m_len);
    }
    memcpy(m_data+m_len, data, len);
    m_len += len;
    return (*this);
}

ByteArray& ByteArray::append(int number)
{
    char temp[64];
    sprintf(temp, "%d", number);
    return append(temp);
}

ByteArray& ByteArray::append(const char* str)
{
    return append(str, strlen(str));
}

ByteArray& ByteArray::append(long long number)
{
    char temp[64];
    sprintf(temp, "%lld", number);
    return append(temp);
}

ByteArray& ByteArray::append(unsigned int number)
{
    char temp[64];
    sprintf(temp, "%u", number);
    return append(temp);
}

ByteArray& ByteArray::append(unsigned long long number)
{
    char temp[64];
    sprintf(temp, "%llu", number);
    return append(temp);
}

ByteArray& ByteArray::append(double number)
{
    char temp[64];
    sprintf(temp, "%f", number);
    return append(temp);
}

#ifdef _WIN32
std::wstring ByteArray::toWString() 
{
    setlocale(LC_ALL, "chs");
    int len = MultiByteToWideChar(CP_ACP, 0, m_data, m_len, nullptr, 0);
    int mallocLen = sizeof(wchar_t*)*(len+1);
    wchar_t* temp = (wchar_t*)malloc(mallocLen);
    len = MultiByteToWideChar(CP_ACP, 0, m_data, m_len, temp, len);
    std::wstring strRst(temp, len);
    free(temp);
    return strRst;
}
#endif

int64_t ByteArray::ToInt()
{
    int64_t rst = 0;
    bool neg = false;
    size_t i = 0;
    if (m_len > 0 && m_data[0] == '-') {
        neg = true;
        i = 1;
    }
    for (; i < m_len; i++) {
        if (m_data[i] < '0' || m_data[i]>'9')break;
        rst *= 10;
        rst += m_data[i] - '0';
    }
    if (neg) {
        rst = -rst;
    }
    return rst;
}

uint64_t ByteArray::ToUInt()
{
    uint64_t rst = 0;
    for (size_t i = 0; i < m_len; i++) {
        if (m_data[i] < '0' || m_data[i]>'9')break;
        rst *= 10;
        rst += m_data[i] - '0';
    }

    return rst;
}

double ByteArray::ToFloat()
{
    double rst = 0.0;
    bool neg = false;
    size_t i = 0;
    if (m_len > 0 && m_data[0] == '-') {
        neg = true;
        i = 1;
    }
    for (; i < m_len; i++) {//整数部分
        if (m_data[i] < '0' || m_data[i]>'9' || m_data[i]=='.')break;
        rst *= 10;
        rst += m_data[i];
    }
    if (i < m_len && m_data[i] == '.') {//小数部分
        i++;
        double coef = 0.1;
        for (; i < m_len; i++) {
            rst += m_data[i] * coef;
            coef /= 10;
        }
    }
    if (neg) {
        rst = -rst;
    }
    return rst;
}

size_t ByteArray::IndexOf(char _c, size_t _begin)
{
    if(_begin>=m_len)return -1;
    char* p = (char*)memchr(m_data + _begin, _c, m_len-_begin);
    if(p)
    {
        return p-m_data;
    }
    return -1;
}

size_t ByteArray::IndexOf(const ByteArray& _ba, size_t _begin)
{
    if(_begin>=m_len)return -1;
    if(_ba.length()==0||_ba.length()==-1)return -1;
    auto p = (char*)memmem(m_data + _begin, m_len, _ba.m_data, _ba.length());
    if(p)
    {
        return p-m_data;
    }
    return -1;
}

ByteArray ByteArray::Middle(size_t _begin, size_t _end)
{
    ByteArray res;
    if(_begin>_end || _begin>=m_len)return res;
    if(_end == npos || _end >= m_len)_end = m_len-1;
    size_t len = _end - _begin + 1;
    if(len<=0)return res;
    res.append(m_data+_begin, len);
    return res;
}

ByteArray ByteArray::Left(size_t _len)
{
    if(_len>m_len)_len=m_len;
    return Middle(0, _len);
}

ByteArray ByteArray::Right(size_t _len)
{
    if(_len>m_len)_len=m_len;
    return Middle(m_len-_len, m_len);
}


void ByteArray::clear() 
{
    if(m_data && m_len!=0)
        memset(m_data, 0, m_len);
    m_len=0;
}

void ByteArray::extend(size_t len) 
{
    size_t newlen = len+BASE_LEN-(len%BASE_LEN);
    m_data = static_cast<char*>(realloc(m_data, newlen));
    if(!m_data)
    {
        perror("realloc error");
        exit(-1);
    }
    memset(m_data+m_max, 0, newlen-m_max);
    m_max = newlen;
}

