#include "NEString.h"
#include <clocale>
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <string.h>
#include "NEByteArray.h"
#include <memory>

#ifdef WIN32
static void* memmem(const void* l, size_t l_len, const void* s, size_t s_len)
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

size_t String::npos = (size_t)(-1);
size_t String::end = (size_t)(-1);

#define BASE_LEN 1024
String::String() noexcept
    : m_max(BASE_LEN)
    , m_data(nullptr)
    , m_len(0)
{
    m_data = static_cast<char*>(malloc(BASE_LEN));
    if (m_data) {
        memset(m_data, 0, BASE_LEN);
    }
    else {
        m_max = 0;
    }
}

String::String(const String& data) noexcept : String()
{
    Append(data);
}

String::String(String&& data) noexcept
{
    m_data = data.m_data;
    data.m_data = nullptr;
    m_len = data.m_len;
    data.m_len = 0;
    m_max = data.m_max;
    data.m_max = 0;
}

String::String(const char* data, size_t len) noexcept : String()
{
    Append(data, len);
}

neapu::String::String(const char* str) noexcept : String()
{
    if (str) {
        Append(str, strlen(str));
    }
}

neapu::String::String(const ByteArray& _ba) noexcept : String(_ba.Data(), _ba.Length())
{
}

String::~String() noexcept
{
    if (m_data)free(m_data);
}

String& String::Append(const String& data)
{
    this->Append(data.m_data, data.m_len);
    return *this;
}

String& String::Append(const char* data, size_t len)
{
    if (len == 0)return (*this);
    if (len + m_len >= m_max)
    {
        extend(len + m_len);
    }
    memcpy(m_data + m_len, data, len);
    m_len += len;
    m_data[m_len] = 0;//保证0结尾
    return (*this);
}

String& String::Append(const char c)
{
    if (1 + m_len >= m_max)
    {
        extend(1 + m_len);
    }
    m_data[m_len] = c;
    m_len += 1;
    m_data[m_len] = 0;//保证0结尾
    return (*this);
}

String& String::Append(int number, NumberBase _base)
{
    if (_base != NumberBase::Decimalism) {
        return Append(static_cast<unsigned int>(number), _base);
    }
    char temp[64];
    sprintf(temp, "%d", number);
    return Append(temp);
}

String& String::Append(const char* str)
{
    return Append(str, strlen(str));
}

String& String::Append(long long number, NumberBase _base)
{
    if (_base != NumberBase::Decimalism) {
        return Append(static_cast<unsigned long long>(number), _base);
    }
    char temp[64];
    sprintf(temp, "%lld", number);
    return Append(temp);
}

String& String::Append(unsigned int number, NumberBase _base)
{
    char temp[64] = { 0 };
    switch (_base)
    {
    case neapu::String::NumberBase::Decimalism:
        sprintf(temp, "%u", number);
        break;
    case neapu::String::NumberBase::Binary: {
        for (int i = 0; i < 32; i++) {
            temp[i] = (number & 0x80000000) ? '1' : '0';
            number <<= 1;
        }
    }
        break;
    case neapu::String::NumberBase::Hexadecimal:
        sprintf(temp, "0x%08x", number);
        break;
    default:
        break;
    }
    return Append(temp);
}

String& String::Append(unsigned long long number, NumberBase _base)
{
    char temp[65];
    switch (_base)
    {
    case neapu::String::NumberBase::Decimalism:
        sprintf(temp, "%llu", number);
        break;
    case neapu::String::NumberBase::Binary:
        for (int i = 0; i < 64; i++) {
            temp[i] = (number & 0x8000000000000000) ? '1' : '0';
            number <<= 1;
        }
        break;
    case neapu::String::NumberBase::Hexadecimal:
        sprintf(temp, "0x%016llx", number);
        break;
    default:
        break;
    }
    
    return Append(temp);
}

String& String::Append(double number)
{
    char temp[64];
    sprintf(temp, "%f", number);
    return Append(temp);
}

String& neapu::String::Argument(const String& data)
{
    size_t index = IndexOf("%1");
    if (index != npos) {
        Replace("%1", data);
        char temp1[10];
        char temp2[10];
        int count = 2;
        while (true) {
            sprintf(temp1, "%%%d", count);
            sprintf(temp2, "%%%d", count-1);
            if (IndexOf(temp1) == npos) {
                break;
            }
            Replace(temp1, temp2);
            count++;
        }
    }
    return *this;
}

String& neapu::String::Argument(const char* data, size_t len)
{
    return Argument(String(data, len));
}

String& neapu::String::Argument(const char* str)
{
    return Argument(String(str));
}

String& neapu::String::Argument(const char c)
{
    return Argument(String().Append(c));
}

String& neapu::String::Argument(int number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String& neapu::String::Argument(long long number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String& neapu::String::Argument(unsigned int number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String& neapu::String::Argument(unsigned long long number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String& neapu::String::Argument(double number)
{
    return Argument(ToString(number));
}

#ifdef _WIN32
std::wstring String::ToWString() const
{
    setlocale(LC_ALL, "chs");
    int len = MultiByteToWideChar(CP_ACP, 0, m_data, m_len, nullptr, 0);
    int mallocLen = sizeof(wchar_t*) * (len + 1);
    wchar_t* temp = (wchar_t*)malloc(mallocLen);
    len = MultiByteToWideChar(CP_ACP, 0, m_data, m_len, temp, len);
    std::wstring strRst(temp, len);
    free(temp);
    return strRst;
}
#endif

int64_t String::ToInt() const
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

uint64_t String::ToUInt() const
{
    uint64_t rst = 0;
    for (size_t i = 0; i < m_len; i++) {
        if (m_data[i] < '0' || m_data[i]>'9')break;
        rst *= 10;
        rst += m_data[i] - '0';
    }

    return rst;
}

double String::ToFloat() const
{
    double rst = 0.0;
    bool neg = false;
    size_t i = 0;
    if (m_len > 0 && m_data[0] == '-') {
        neg = true;
        i = 1;
    }
    for (; i < m_len; i++) {//整数部分
        if (m_data[i] < '0' || m_data[i]>'9' || m_data[i] == '.')break;
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

size_t String::IndexOf(char _c, size_t _begin) const
{
    if (_begin >= m_len)return -1;
    char* p = (char*)memchr(m_data + _begin, _c, m_len - _begin);
    if (p)
    {
        return p - m_data;
    }
    return -1;
}

size_t String::IndexOf(const String& _ba, size_t _begin) const
{
    if (_begin >= m_len)return -1;
    if (_ba.Length() == 0 || _ba.Length() == -1)return -1;
    auto p = (char*)memmem(m_data + _begin, m_len, _ba.m_data, _ba.Length());
    if (p)
    {
        return p - m_data;
    }
    return -1;
}

size_t neapu::String::LastIndexOf(char _c, size_t _begin) const
{
    if (m_len == 0)return npos;
    size_t pos = _begin;
    if (pos > m_len)pos = m_len;
    do {
        pos--;
        if (m_data[pos] == _c) {
            return pos;
        }
    } while (pos > 0);
    return npos;
}

String String::Middle(size_t _begin, size_t _end) const
{
    String res;
    if (_begin > _end || _begin >= m_len)return res;
    if (_end == npos || _end >= m_len)_end = m_len - 1;
    size_t len = _end - _begin + 1;
    if (len <= 0)return res;
    res.Append(m_data + _begin, len);
    return res;
}

String String::Left(size_t _len) const
{
    _len -= 1;
    if (_len > m_len)_len = m_len;
    return Middle(0, _len);
}

String String::Right(size_t _len) const
{
    if (_len > m_len)_len = m_len;
    return Middle(m_len - _len, m_len);
}


void String::Clear()
{
    if (m_data && m_len != 0)
        memset(m_data, 0, m_len);
    m_len = 0;
}

std::vector<String> String::Split(const String& _separator, bool _skepEmpty)
{
    std::vector<String> rst;
    size_t index = 0;
    size_t oldIndex = 0;
    while(oldIndex < this->Length()) {
        index = this->IndexOf(_separator, oldIndex);
        if (index == npos) {
            break;
        }
        if (index == oldIndex) {
            if (!_skepEmpty) {
                rst.push_back(String());
            }
            oldIndex += 1;
            continue;
        }
        rst.push_back(this->Middle(oldIndex, index - 1));
        oldIndex = index + _separator.Length();
    }
    if (oldIndex < this->Length()) {
        rst.push_back(this->Middle(oldIndex, npos));
    }
    return rst;
}

std::vector<String> neapu::String::Split(const char _separator, bool _skepEmpty)
{
    std::vector<String> rst;
    size_t index = 0;
    size_t oldIndex = 0;
    while (oldIndex < this->Length()) {
        index = this->IndexOf(_separator, oldIndex);
        if (index == npos) {
            break;
        }
        if (index == oldIndex) {
            if (!_skepEmpty) {
                rst.push_back(String());
            }
            oldIndex += 1;
            continue;
        }
        rst.push_back(this->Middle(oldIndex, index - 1));
        oldIndex = index + 1;
    }
    if (oldIndex < this->Length()) {
        rst.push_back(this->Middle(oldIndex, npos));
    }
    return rst;
}

bool neapu::String::Replace(const String& _before, const String& _after)
{
    size_t index = IndexOf(_before);
    if (index == npos)return false;
    int afterSize = _after.Length() - _before.Length();
    size_t newLen = m_len + afterSize;
    if (newLen > m_max) {
        extend(m_len + afterSize);
    }
    std::shared_ptr<char> temp(new char[newLen]);
    size_t offset = 0;
    memcpy(temp.get() + offset, m_data, index);
    offset += index;
    memcpy(temp.get() + offset, _after.m_data, _after.Length());
    offset += _after.Length();
    size_t remSize = m_len - (index + _before.Length());
    memcpy(temp.get() + offset, m_data + (index + _before.Length()), remSize);
    memset(m_data, 0, m_len);
    memcpy(m_data, temp.get(), newLen);
    m_len = newLen;
    return true;
}

bool neapu::String::Contain(const String& _str) const
{
    return IndexOf(_str) != npos;
}

//去除头尾空格
String neapu::String::RemoveHeadAndTailSpace(String str)
{
    size_t begin;
    for (begin = 0; begin < str.Length(); begin++) {
        if (str[begin] != ' ') {
            break;
        }
    }
    int end;
    for (end = (int)str.Length() - 1; end >= 0; end--) {
        if (str[end] != ' ') {
            break;
        }
    }
    return str.Middle(begin, (size_t)end);
}

String neapu::String::ToString(int number, NumberBase _base)
{
    return String().Append(number, _base);
}

String neapu::String::ToString(long long number, NumberBase _base)
{
    return String().Append(number, _base);
}

String neapu::String::ToString(unsigned int number, NumberBase _base)
{
    return String().Append(number, _base);
}

String neapu::String::ToString(unsigned long long number, NumberBase _base)
{
    return String().Append(number, _base);
}

String neapu::String::ToString(double number)
{
    return String().Append(number);
}

void String::extend(size_t len)
{
    size_t newlen = len + BASE_LEN - (len % BASE_LEN);
    m_data = static_cast<char*>(realloc(m_data, newlen));
    if (!m_data)
    {
        perror("realloc error");
        exit(-1);
    }
    memset(m_data + m_max, 0, newlen - m_max);
    m_max = newlen;
}

String String::operator+(const String& _str)
{
    String rst;
    rst.Append(*this);
    rst.Append(_str);
    return rst;
}

void String::operator+=(const String& _str)
{
    this->Append(_str);
}

String neapu::operator+(const char* _cstr, const String& _str)
{
    String rst(_cstr);
    rst.Append(_str);
    return rst;
}

