#pragma once
#include "base_pub.h"
#include <string.h>
#include <string>
#include <stdint.h>
#define Find IndexOf
namespace neapu {
    class ByteArray;
    class NEAPU_BASE_EXPORT String {
    public:
        static size_t npos;
        String() noexcept;
        String(const String& data) noexcept;
        String(String&& data) noexcept;
        String(const char* data, size_t len) noexcept;
        String(const char* str) noexcept;
        String(const ByteArray& _ba) noexcept;
        virtual ~String() noexcept;

        String& Append(const String& data);
        String& Append(const char* data, size_t len);
        String& Append(const char* str);
        String& Append(const char c);
        String& Append(int number);
        String& Append(long long number);
        String& Append(unsigned int number);
        String& Append(unsigned long long number);
        String& Append(double number);

        
        size_t Length() const { return m_len; }
        char Front() const {
            if (m_len > 0)return m_data[0];
            return 0;
        }
        char Back() const {
            if (m_len > 0) {
                return m_data[m_len - 1];
            }
            return 0;
        }
#ifdef _WIN32
        std::wstring ToWString() const;
#endif
        int64_t ToInt() const;
        uint64_t ToUInt() const;
        double ToFloat() const;
        const char* ToCString() const { return m_data; }
        char* ToCString() { return m_data; }

        const char* Ptr() const { return m_data; }
        char* Ptr() { return m_data; }

        size_t IndexOf(char _c, size_t _begin = 0) const;
        size_t IndexOf(const String& _ba, size_t _begin = 0) const;
        String Middle(size_t _begin, size_t _end) const;
        String Left(size_t _len) const;
        String Right(size_t _len) const;
        void Clear();
        bool IsEmpty() const { return m_len == 0; }

        void operator=(const String& ba) {
            Clear();
            Append(ba);
        }
        inline bool operator<(const String& str) const 
        { 
            if (this->m_data && str.m_data) {
                return strcmp(this->m_data, str.m_data) < 0;
            }
            else if(this->m_data) {
                return false;
            }
            else {
                return true;
            }
        }
        inline bool operator>(const String& str) const
        {
            if (this->m_len && str.m_len) {
                return strcmp(this->m_data, str.m_data) > 0;
            }
            else if (this->m_len) {
                return true;
            }
            else {
                return false;
            }
        }
        inline bool operator==(const String& ba) const
        {
            return (m_len == ba.m_len ? (memcmp(m_data, ba.m_data, m_len) == 0) : false);
        }
        inline bool operator==(const char* str) const
        {
            if (!str) return m_len == 0 ? true : false;
            return (m_len == strlen(str) ? (memcmp(m_data, str, m_len) == 0) : false);
        }
        inline bool operator!=(const String& ba) const
        {
            return !operator==(ba);
        }
        inline bool operator!=(const char* str) const
        {
            return !operator==(str);
        }
        inline char& operator[](size_t pos)
        {
            return m_data[pos];
        }
        String operator+(const String& _str);
        void operator+=(const String& _str);
    protected:
        void extend(size_t len);
    protected:
        char* m_data;
        size_t m_len;
        size_t m_max;
    };
    NEAPU_BASE_EXPORT String operator+(const char* _cstr, const String& _str);
}
