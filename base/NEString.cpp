#include "NEString.h"
#include <clocale>
#include <cstddef>
#include <map>
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include "NEByteArray.h"
#include <memory>
#include <string.h>
#include "NEUtil.h"

#ifdef WIN32
static void *memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
    register char *cur, *last;
    const char *cl = (const char *)l;
    const char *cs = (const char *)s;

    /* we need something to compare */
    if (l_len == 0 || s_len == 0)
        return NULL;

    /* "s" must be smaller or equal to "l" */
    if (l_len < s_len)
        return NULL;

    /* special case where s_len == 1 */
    if (s_len == 1)
        return (void *)memchr(l, (int)*cs, l_len);

    /* the last position where its possible to find "s" in "l" */
    last = (char *)cl + l_len - s_len;

    for (cur = (char *)cl; cur <= last; cur++)
        if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
            return cur;

    return NULL;
}
#endif // WIN32

using namespace neapu;

size_t String::npos = (size_t)(-1);
size_t String::end = (size_t)(-1);

static constexpr size_t BASE_LEN = 4096;
String::String(size_t len)
    : m_max(BASE_LEN)
    , m_data(nullptr)
    , m_len(0)
{
    extend(len);
}

String::String(const String &data)
    : String()
{
    Append(data);
}

String::String(String &&data) noexcept
{
    m_data = std::move(data.m_data);
    data.m_data = nullptr;
    m_len = data.m_len;
    data.m_len = 0;
    m_max = data.m_max;
    data.m_max = 0;
}

String::String(const char *data, size_t len)
    : String()
{
    Append(data, len);
}

neapu::String::String(const char *str)
    : String()
{
    if (str) {
        Append(str, strlen(str));
    }
}

neapu::String::String(const ByteArray &_ba)
    : String(_ba.Length())
{
    memcpy(m_data.get(), _ba.Data(), _ba.Length());
    m_len = _ba.Length();
    m_data.get()[m_len] = 0;
}

neapu::String::String(const std::string &_str)
    : String(_str.c_str(), _str.length())
{}

String::~String() noexcept
{
}

String &String::Append(const String &data)
{
    this->Append(data.ToCString(), data.Length());
    return *this;
}

String &String::Append(const char *data, size_t len)
{
    if (len == 0)
        return (*this);
    if (len + m_len >= m_max) {
        extend(len + m_len);
    }
    memcpy(m_data.get() + m_len, data, len);
    m_len += len;
    m_data.get()[m_len] = 0; //保证0结尾
    return (*this);
}

String &String::Append(const char c)
{
    if (1 + m_len >= m_max) {
        extend(1 + m_len);
    }
    m_data.get()[m_len] = c;
    m_len += 1;
    m_data.get()[m_len] = 0; //保证0结尾
    return (*this);
}

String &String::Append(int number, NumberBase _base)
{
    if (_base != NumberBase::Decimalism) {
        return Append(static_cast<unsigned int>(number), _base);
    }
    char temp[64];
    sprintf(temp, "%d", number);
    return Append(temp);
}

String &String::Append(const char *str)
{
    return Append(str, strlen(str));
}

String &String::Append(long long number, NumberBase _base)
{
    if (_base != NumberBase::Decimalism) {
        return Append(static_cast<unsigned long long>(number), _base);
    }
    char temp[64];
    sprintf(temp, "%lld", number);
    return Append(temp);
}

String &String::Append(unsigned int number, NumberBase _base)
{
    char temp[64] = {0};
    switch (_base) {
    case neapu::String::NumberBase::Decimalism:
        sprintf(temp, "%u", number);
        break;
    case neapu::String::NumberBase::Binary: {
        for (int i = 0; i < 32; i++) {
            temp[i] = (number & 0x80000000) ? '1' : '0';
            number <<= 1;
        }
    } break;
    case neapu::String::NumberBase::Hexadecimal:
        sprintf(temp, "0x%08x", number);
        break;
    default:
        break;
    }
    return Append(temp);
}

String &String::Append(unsigned long long number, NumberBase _base)
{
    char temp[65];
    switch (_base) {
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

String &String::Append(double number)
{
    char temp[64];
    sprintf(temp, "%f", number);
    return Append(temp);
}

String &neapu::String::Argument(const String &data)
{
    size_t index = IndexOf("%1");
    if (index != npos) {
        Replace("%1", data);
        char temp1[10];
        char temp2[10];
        int count = 2;
        while (true) {
            sprintf(temp1, "%%%d", count);
            sprintf(temp2, "%%%d", count - 1);
            if (IndexOf(temp1) == npos) {
                break;
            }
            Replace(temp1, temp2);
            count++;
        }
    }
    return *this;
}

String &neapu::String::Argument(const char *data, size_t len)
{
    return Argument(String(data, len));
}

String &neapu::String::Argument(const char *str)
{
    return Argument(String(str));
}

String &neapu::String::Argument(const char c)
{
    return Argument(String().Append(c));
}

String &neapu::String::Argument(int number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String &neapu::String::Argument(long long number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String &neapu::String::Argument(unsigned int number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String &neapu::String::Argument(unsigned long long number, NumberBase _base)
{
    return Argument(ToString(number, _base));
}

String &neapu::String::Argument(double number)
{
    return Argument(ToString(number));
}

#ifdef _WIN32
std::wstring String::ToWString() const
{
    setlocale(LC_ALL, "chs");
    int len = MultiByteToWideChar(CP_ACP, 0, m_data, m_len, nullptr, 0);
    int mallocLen = sizeof(wchar_t *) * (len + 1);
    wchar_t *temp = (wchar_t *)malloc(mallocLen);
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
    if (m_len > 0 && m_data.get()[0] == '-') {
        neg = true;
        i = 1;
    }
    for (; i < m_len; i++) {
        if (m_data.get()[i] < '0' || m_data.get()[i] > '9')
            break;
        rst *= 10;
        rst += m_data.get()[i] - '0';
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
        if (m_data.get()[i] < '0' || m_data.get()[i] > '9')
            break;
        rst *= 10;
        rst += m_data.get()[i] - '0';
    }

    return rst;
}

double String::ToFloat() const
{
    double rst = 0.0;
    bool neg = false;
    size_t i = 0;
    if (m_len > 0 && m_data.get()[0] == '-') {
        neg = true;
        i = 1;
    }
    for (; i < m_len; i++) { //整数部分
        if (m_data.get()[i] < '0' || m_data.get()[i] > '9' || m_data.get()[i] == '.')
            break;
        rst *= 10;
        rst += m_data.get()[i];
    }
    if (i < m_len && m_data.get()[i] == '.') { //小数部分
        i++;
        double coef = 0.1;
        for (; i < m_len; i++) {
            rst += m_data.get()[i] * coef;
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
    if (_begin >= m_len)
        return -1;
    char *datePtr = m_data.get();
    char *p = (char *)memchr(datePtr + _begin, _c, m_len - _begin);
    if (p) {
        return p - datePtr;
    }
    return -1;
}

size_t String::IndexOf(const String &_ba, size_t _begin) const
{
    if (_begin >= m_len)
        return -1;
    if (_ba.Length() == 0 || _ba.Length() == -1)
        return -1;
    auto p = (char *)memmem(Ptr() + _begin, m_len, _ba.Ptr(), _ba.Length());
    if (p) {
        return p - Ptr();
    }
    return -1;
}

size_t neapu::String::LastIndexOf(char _c, size_t _begin) const
{
    if (m_len == 0)
        return npos;
    size_t pos = _begin;
    if (pos > m_len)
        pos = m_len;
    do {
        pos--;
        if (m_data.get()[pos] == _c) {
            return pos;
        }
    } while (pos > 0);
    return npos;
}

String String::Middle(size_t _begin, size_t _end) const
{
    String res;
    if (_begin > _end || _begin >= m_len)
        return res;
    if (_end == npos || _end >= m_len)
        _end = m_len - 1;
    size_t len = _end - _begin + 1;
    if (len <= 0)
        return res;
    res.Append(m_data.get() + _begin, len);
    return res;
}

String String::Left(size_t _len) const
{
    _len -= 1;
    if (_len > m_len)
        _len = m_len;
    return Middle(0, _len);
}

String String::Right(size_t _len) const
{
    if (_len > m_len)
        _len = m_len;
    return Middle(m_len - _len, m_len);
}

void String::Clear()
{
    m_data.reset();
    m_len = 0;
    m_max = 0;
    extend(0);
}

std::vector<String> String::Split(const String &_separator, bool _skepEmpty)
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
        oldIndex = index + _separator.Length();
    }
    if (oldIndex < this->Length()) {
        rst.push_back(this->Middle(oldIndex, npos));
    }
    return rst;
}

std::vector<String> neapu::String::Split(const char _separator,
                                         bool _skepEmpty)
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

bool neapu::String::Replace(const String &_before, const String &_after)
{
    size_t index = IndexOf(_before);
    if (index == npos)
        return false;
    int afterSize = _after.Length() - _before.Length();
    size_t newLen = m_len + afterSize;
    if (newLen > m_max) {
        extend(m_len + afterSize);
    }
    std::shared_ptr<char> temp(new char[newLen]);
    size_t offset = 0;
    memcpy(temp.get() + offset, m_data.get(), index);
    offset += index;
    memcpy(temp.get() + offset, _after.m_data.get(), _after.Length());
    offset += _after.Length();
    size_t remSize = m_len - (index + _before.Length());
    memcpy(temp.get() + offset, m_data.get() + (index + _before.Length()), remSize);
    memset(m_data.get(), 0, m_len);
    memcpy(m_data.get(), temp.get(), newLen);
    m_len = newLen;
    return true;
}

bool neapu::String::Contain(const String &_str) const
{
    return IndexOf(_str) != npos;
}

//去除头尾空格
String neapu::String::RemoveHeadAndTailSpace(String &&str)
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
    char *newData = new char[newlen];
    memset(newData, 0, newlen);
    m_max = newlen;
    memcpy(newData, m_data.get(), m_len);
    m_data.reset(newData);
}

String String::operator+(const String &_str)
{
    String rst;
    rst.Append(*this);
    rst.Append(_str);
    return rst;
}

void String::operator+=(const String &_str)
{
    this->Append(_str);
}

String String::ToHex(bool _upper) const
{
    auto buf = std::unique_ptr<char>(new char[m_len * 2]);
    static char hexu[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    static char hexd[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    auto GetChar = [&](char c) -> char {
        return _upper ? hexu[c] : hexd[c];
    };

    for (size_t i = 0; i < m_len; i++) {
        buf.get()[i * 2] = GetChar((m_data.get()[i] >> 4) & 0x0f);
        buf.get()[i * 2 + 1] = GetChar((m_data.get()[i]) & 0x0f);
    }
    return String(buf.get(), m_len * 2);
}

String String::FromHex(const String &_hex)
{
    size_t len = _hex.Length() / 2 + 1;
    auto buf = std::unique_ptr<char>(new char[len]);
    static std::map<char, char> hexu = {
        {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}, {'A', 10}, {'B', 11}, {'C', 12}, {'D', 13}, {'E', 14}, {'F', 15}, {'0', 0}};
    static std::map<char, char> hexd = {
        {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}, {'a', 10}, {'b', 11}, {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15}, {'0', 0}};
    auto GetChar = [&](const char c) -> char {
        if (hexu.find(c) != hexu.end()) return hexu[c];
        if (hexd.find(c) != hexd.end()) return hexd[c];
        return 0;
    };
    for (size_t i = 0; i < len - 1; i++) {
        buf.get()[i] = (GetChar(_hex[i * 2]) << 8) & 0xf0;
        buf.get()[i] += GetChar(_hex[i * 2 + 1]) & 0x0f;
    }
    buf.get()[len - 1] = 0;
    return String(buf.get(), len);
}

String String::ToBase64() const
{
    return Encryption::Base64Encode(*this);
}

String String::FromBase64(const String &_base64)
{
    return String(Encryption::Base64Decode(_base64));
}

String neapu::operator+(const char *_cstr, const String &_str)
{
    String rst(_cstr);
    rst.Append(_str);
    return rst;
}

void String::RemoveBack(size_t _count)
{
    if (_count > m_len) _count = m_len;
    m_len -= _count;
    m_data.get()[m_len] = 0;
}