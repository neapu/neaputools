/* ************************************************************************************ *
 * 说明：字符串类
 * 最后修改：20220611
 * 更新说明：
 * 20220611：从整形转换成字符串时，增加对16进制和2进制的支持
 * ************************************************************************************ */
#pragma once
#include "base/base_pub.h"
#include <cstddef>
#include <memory>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <logger/logger.h>

namespace neapu {
class ByteArray;
class NEAPU_BASE_EXPORT String {
public:
    enum class NumberBase : char {
        Decimalism,
        Binary,
        Hexadecimal
    };

public:
    static size_t npos;
    static size_t end;
    String(size_t len = 0);
    String(const String &data);
    String(String &&data) noexcept;
    String(const char *data, size_t len);
    String(const char *str);
    String(const ByteArray &_ba);
    String(const std::string &_str);

    virtual ~String() noexcept;

    String &Append(const String &data);
    String &Append(const char *data, size_t len);
    String &Append(const char *str);
    String &Append(const char c);
    String &Append(int number, NumberBase _base = NumberBase::Decimalism);
    String &Append(long long number, NumberBase _base = NumberBase::Decimalism);
    String &Append(unsigned int number, NumberBase _base = NumberBase::Decimalism);
    String &Append(unsigned long long number, NumberBase _base = NumberBase::Decimalism);
    String &Append(double number);

    String &Argument(const String &data);
    String &Argument(const char *data, size_t len);
    String &Argument(const char *str);
    String &Argument(const char c);
    String &Argument(int number, NumberBase _base = NumberBase::Decimalism);
    String &Argument(long long number, NumberBase _base = NumberBase::Decimalism);
    String &Argument(unsigned int number, NumberBase _base = NumberBase::Decimalism);
    String &Argument(unsigned long long number, NumberBase _base = NumberBase::Decimalism);
    String &Argument(double number);

    size_t Length() const
    {
        return m_len;
    }
    char Front() const
    {
        if (m_len > 0) return m_data.get()[0];
        return 0;
    }
    char Back() const
    {
        if (m_len > 0) {
            return m_data.get()[m_len - 1];
        }
        return 0;
    }
#ifdef _WIN32
    std::wstring ToWString() const;
#endif
    int64_t ToInt() const;
    uint64_t ToUInt() const;
    double ToFloat() const;
    const char *ToCString() const
    {
        return m_data.get();
    }
    char *ToCString()
    {
        return m_data.get();
    }
    std::string ToStdString() const
    {
        return std::string(ToCString(), Length());
    }

    const char *Ptr() const
    {
        return m_data.get();
    }
    char *Ptr()
    {
        return m_data.get();
    }

    size_t IndexOf(char _c, size_t _begin = 0) const;
    size_t IndexOf(const String &_ba, size_t _begin = 0) const;
    size_t LastIndexOf(char _c, size_t _begin = npos) const;
    template <class... Args>
    size_t Find(Args &&...args)
    {
        return IndexOf(std::forward<Args>(args)...);
    }

    // 闭区间，包前包后
    String Middle(size_t _begin, size_t _end) const;
    String Left(size_t _len) const;
    String Right(size_t _len) const;
    void Clear();
    bool IsEmpty() const
    {
        return m_len == 0;
    }
    std::vector<String> Split(const String &_separator, bool _skepEmpty = false) const;
    std::vector<String> Split(const char _separator, bool _skepEmpty = false) const;
    bool Replace(const String &_before, const String &_after);
    bool Contain(const String &_str) const;

    void operator=(const String &ba)
    {
        Clear();
        Append(ba);
    }
    inline bool operator<(const String &str) const
    {
        if (!this->IsEmpty() && !str.IsEmpty()) {
            return strcmp(this->ToCString(), str.ToCString()) < 0;
        } else if (!this->IsEmpty()) {
            return false;
        } else {
            return true;
        }
    }
    inline bool operator>(const String &str) const
    {
        if (!this->IsEmpty() && !str.IsEmpty()) {
            return strcmp(this->ToCString(), str.ToCString()) > 0;
        } else if (!this->IsEmpty()) {
            return true;
        } else {
            return false;
        }
    }
    inline bool operator==(const String &str) const
    {
        if (this->IsEmpty() && str.IsEmpty()) {
            return true;
        } else if (this->Length() == str.Length()) {
            return strcmp(this->ToCString(), str.ToCString()) == 0;
        }
        return false;
    }
    inline bool operator==(const char *str) const
    {
        if (!str) return m_len == 0 ? true : false;
        return (m_len == strlen(str) ? (strcmp(ToCString(), str) == 0) : false);
    }
    inline bool operator!=(const String &ba) const
    {
        return !operator==(ba);
    }
    inline bool operator!=(const char *str) const
    {
        return !operator==(str);
    }
    inline char &operator[](size_t pos)
    {
        return m_data.get()[pos];
    }
    const char &operator[](size_t pos) const
    {
        return m_data.get()[pos];
    }
    String operator+(const String &_str);
    void operator+=(const String &_str);

    String ToHex(bool _upper) const;
    String ToBase64() const;

    void RemoveBack(size_t _count);
    //去除头尾空格
    String RemoveHeadAndTailSpace();

public:
    //去除头尾空格
    static String RemoveHeadAndTailSpace(const String &str);
    static String ToString(int number, NumberBase _base = NumberBase::Decimalism);
    static String ToString(long long number, NumberBase _base = NumberBase::Decimalism);
    static String ToString(unsigned int number, NumberBase _base = NumberBase::Decimalism);
    static String ToString(unsigned long long number, NumberBase _base = NumberBase::Decimalism);
    static String ToString(double number);
    static String FromHex(const String &_hex);
    static String FromBase64(const String &_base64);

protected:
    void extend(size_t len);

protected:
    std::unique_ptr<char> m_data;
    size_t m_len;
    size_t m_max;
};
using StringList = std::vector<String>;
NEAPU_BASE_EXPORT String operator+(const char *_cstr, const String &_str);

} // namespace neapu
NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, const neapu::String& _err);
NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::String& _err);
NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::String&& _err);