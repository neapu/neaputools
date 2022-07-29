#pragma once
#include "base_pub.h"
#include <string.h>
#include <string>
#include <stdint.h>
namespace neapu {
class String;
class NEAPU_BASE_EXPORT ByteArray {
public:
    static size_t npos;
    ByteArray();
    ByteArray(const ByteArray &data);
    ByteArray(ByteArray &&data) noexcept;
    ByteArray(const unsigned char *data, size_t len);
    ByteArray(const String &_str);
    virtual ~ByteArray() noexcept;

    ByteArray &Append(const ByteArray &data);
    ByteArray &Append(const unsigned char *data, size_t len);
    ByteArray &Append(const String& _str);

    unsigned char *Ptr()
    {
        return m_data;
    }
    const unsigned char *Data() const
    {
        return m_data;
    }
    size_t Length() const
    {
        return m_len;
    }

    template <class... Args>
    size_t Find(Args &&...args) const
    {
        return IndexOf(std::forward<Args>(args)...);
    }
    size_t IndexOf(char _c, size_t _begin = 0) const;
    size_t IndexOf(const ByteArray &_ba, size_t _begin = 0) const;
    ByteArray Middle(size_t _begin, size_t _end) const;
    ByteArray Left(size_t _len) const;
    ByteArray Right(size_t _len) const;
    void Clear();
    bool IsEmpty() const
    {
        return m_len == 0;
    }

    void operator=(const ByteArray &ba)
    {
        Clear();
        Append(ba);
    }
    void operator=(ByteArray &&_ba) noexcept;
    bool operator<(const ByteArray &ba) const;
    inline bool operator==(const ByteArray &ba) const
    {
        return (m_len == ba.m_len ? (memcmp(m_data, ba.m_data, m_len) == 0) : false);
    }
    inline bool operator==(const char *str) const
    {
        if (!str) return m_len == 0 ? true : false;
        return (m_len == strlen(str) ? (memcmp(m_data, str, m_len) == 0) : false);
    }
    inline bool operator!=(const ByteArray &ba) const
    {
        return !operator==(ba);
    }
    inline bool operator!=(const char *str) const
    {
        return !operator==(str);
    }
    inline unsigned char &operator[](size_t pos)
    {
        return m_data[pos];
    }

    String ToHex(bool _upper = false);

protected:
    void extend(size_t len);

protected:
    unsigned char *m_data;
    size_t m_len;
    size_t m_max;
};
} // namespace neapu
