#pragma once
#include "base_pub.h"
#include <string.h>
#include <string>
namespace neapu {
class NEAPU_BASE_EXPORT ByteArray{
public:
    static size_t npos;
    ByteArray();
    ByteArray(const ByteArray& data);
    ByteArray(ByteArray&& data);
    ByteArray(const char* data, size_t len);
    ByteArray(const char* str) : ByteArray(str, strlen(str)) {}
    virtual ~ByteArray();

    ByteArray& append(const ByteArray& data);
    ByteArray& append(const char* data, size_t len);
    ByteArray& append(const char* str);
    ByteArray& append(int number);
    ByteArray& append(long long number);
    ByteArray& append(unsigned int number);
    ByteArray& append(unsigned long long number);
    ByteArray& append(double number);

    char* ptr() {return m_data;}
    const char* data() const {return m_data;}
    size_t length() const {return m_len;}
#ifdef _WIN32
    std::wstring toWString();
#endif
    
    size_t IndexOf(char _c, size_t _begin);
    size_t IndexOf(const ByteArray& _ba, size_t _begin);
    ByteArray Middle(size_t _begin, size_t _end);
    ByteArray Left(size_t _len);
    ByteArray Right(size_t _len);
    void clear();

    void operator=(const ByteArray& ba) {
        clear();
        append(ba);
    }
    inline bool operator<(const ByteArray& str) const { return strcmp(this->data(), str.data()) < 0; }
    inline bool operator==(const ByteArray& ba) const
    {return (m_len==ba.m_len?(memcmp(m_data, ba.m_data, m_len) == 0):false);}
    inline bool operator==(const char* str) const
    {
        if(!str) return m_len==0?true:false;
        return (m_len==strlen(str)?(memcmp(m_data, str, m_len) == 0):false);
    }
    inline bool operator!=(const ByteArray& ba) const
    {return !operator==(ba);}
    inline bool operator!=(const char* str) const
    {return !operator==(str);}
    inline char& operator[](size_t pos)
    { return m_data[pos]; }
protected:
    void extend(size_t len);
protected:
    char* m_data;
    size_t m_len;
    size_t m_max;
};
}
