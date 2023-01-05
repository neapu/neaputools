#include "NEByteArray.h"
#include <algorithm>
#include <clocale>
#include <cstddef>
#include <mutex>
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <string.h>
#include "NEString.h"
#include <memory>

#ifdef _WIN32
static void *memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
    char *cur, *last;
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

size_t ByteArray::npos = (size_t)(-1);

#define BASE_LEN 1024
ByteArray::ByteArray()
    : m_max(BASE_LEN)
    , m_data(nullptr)
    , m_len(0)
{
    m_data = static_cast<unsigned char *>(malloc(BASE_LEN));
    memset(m_data, 0, BASE_LEN);
}

ByteArray::ByteArray(const ByteArray &data)
    : ByteArray()
{
    Append(data);
}

ByteArray::ByteArray(ByteArray &&data) noexcept
{
    m_data = data.m_data;
    data.m_data = nullptr;
    m_len = data.m_len;
    data.m_len = 0;
    m_max = data.m_max;
    data.m_max = 0;
}

ByteArray::ByteArray(const unsigned char *data, size_t len)
    : ByteArray()
{
    Append(data, len);
}

neapu::ByteArray::ByteArray(const String &_str)
    : ByteArray(reinterpret_cast<const unsigned char *>(_str.ToCString()), _str.Length())
{
}

ByteArray::~ByteArray() noexcept
{
    if (m_data) free(m_data);
}

void ByteArray::operator=(ByteArray &&_ba) noexcept
{
    Clear();
    m_data = _ba.m_data;
    _ba.m_data = nullptr;
    m_len = _ba.m_len;
    _ba.m_len = 0;
    m_max = _ba.m_max;
    _ba.m_max = 0;
}

bool ByteArray::operator<(const ByteArray &ba) const
{
    if (m_len == 0 && ba.m_len > 0) {
        return true;
    } else if (m_len > 0 && ba.m_len == 0) {
        return false;
    } else {
        size_t len = (std::min)(m_len, ba.m_len);
        int rc = memcmp(m_data, ba.m_data, len);
        if (rc == 0) {
            return m_len < ba.m_len;
        }
        return rc;
    }
}

ByteArray &ByteArray::Append(const ByteArray &data)
{
    this->Append(data.m_data, data.m_len);
    return *this;
}

ByteArray &ByteArray::Append(const unsigned char *data, size_t len)
{
    if (len == 0) return (*this);
    if (len + m_len >= m_max) {
        extend(len + m_len);
    }
    memcpy(m_data + m_len, data, len);
    m_len += len;
    return (*this);
}

ByteArray &ByteArray::Append(const String &_str)
{
    return Append(reinterpret_cast<const unsigned char *>(_str.ToCString()), _str.Length());
}

size_t ByteArray::IndexOf(char _c, size_t _begin) const
{
    if (_begin >= m_len) return -1;
    unsigned char *p = (unsigned char *)memchr(m_data + _begin, _c, m_len - _begin);
    if (p) {
        return p - m_data;
    }
    return -1;
}

size_t ByteArray::IndexOf(const ByteArray &_ba, size_t _begin) const
{
    if (_begin >= m_len) return -1;
    if (_ba.Length() == 0 || _ba.Length() == -1) return -1;
    auto p = (unsigned char *)memmem(m_data + _begin, m_len, _ba.m_data, _ba.Length());
    if (p) {
        return p - m_data;
    }
    return -1;
}

ByteArray ByteArray::Middle(size_t _begin, size_t _end) const
{
    ByteArray res;
    if (_begin > _end || _begin >= m_len) return res;
    if (_end == npos || _end >= m_len) _end = m_len - 1;
    size_t len = _end - _begin + 1;
    if (len <= 0) return res;
    res.Append(m_data + _begin, len);
    return res;
}

ByteArray ByteArray::Left(size_t _len) const
{
    if (_len > m_len) _len = m_len;
    return Middle(0, _len);
}

ByteArray ByteArray::Right(size_t _len) const
{
    if (_len > m_len) _len = m_len;
    return Middle(m_len - _len, m_len);
}

void ByteArray::Clear()
{
    if (m_data && m_len != 0)
        memset(m_data, 0, m_len);
    m_len = 0;
}

void ByteArray::extend(size_t len)
{
    size_t newlen = len + BASE_LEN - (len % BASE_LEN);
    m_data = static_cast<unsigned char *>(realloc(m_data, newlen));
    if (!m_data) {
        perror("realloc error");
        exit(-1);
    }
    memset(m_data + m_max, 0, newlen - m_max);
    m_max = newlen;
}

String ByteArray::ToHex(bool _upper)
{
    auto buf = std::unique_ptr<char>(new char[m_len * 2]);
    static char hexu[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    static char hexd[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    auto GetChar = [&](unsigned char c) -> char {
        return _upper ? hexu[c] : hexd[c];
    };

    for (size_t i = 0; i < m_len; i++) {
        buf.get()[i * 2] = GetChar((m_data[i] >> 4) & 0x0f);
        buf.get()[i * 2 + 1] = GetChar((m_data[i]) & 0x0f);
    }
    return String(buf.get(), m_len*2);
}

NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, const neapu::ByteArray& _byteArray)
{
    return _logger << std::string((char*)_byteArray.Data(), _byteArray.Length());
}

NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::ByteArray& _byteArray)
{
    return _logger << std::string((char*)_byteArray.Data(), _byteArray.Length());
}

NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::ByteArray&& _byteArray)
{
    return _logger << std::string((char*)_byteArray.Data(), _byteArray.Length());
}