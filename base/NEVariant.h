#pragma once

#include "NEString.h"
#include <cstddef>
#include <type_traits>
namespace neapu {
class Variant {
public:
    // enum class Type{
    //     typeChar,
    //     typeShort,
    //     typeInt,
    //     typeUInt,
    //     typeInt64,
    //     typeUInt64,
    //     typeFloat,
    //     typeDouble
    // };

    Variant()
    {}

    Variant(const char str[])
    {
        m_stringData = str;
        m_typeLength = m_stringData.Length();
        m_typeName = GetTypeName<String>();
    }

    Variant(const String& str)
    {
        m_stringData = str;
        m_typeLength = m_stringData.Length();
        m_typeName = GetTypeName<String>();
    }

    template <typename T>
    Variant(T data)
    {
        m_typeLength = sizeof(data);
        m_typeName = GetTypeName<T>();
        memcpy(m_baseData, &data, m_typeLength);
    }

    template<typename T>
    T To() const
    {
        if constexpr (std::is_same_v<T, neapu::String>){
            return ToString();
        } else if constexpr (std::is_same_v<T, const char*>){
            return ToCString();
        } else {
            T rst;
            memcpy(&rst, m_baseData, m_typeLength);
            return rst;
        }
    }

    String ToString() const
    {
        return m_stringData;
    }

    const char* ToCString() const
    {
        return m_stringData.ToCString();
    }

private:
    template <typename T>
    String GetTypeName()
    {
        const char *temp = __PRETTY_FUNCTION__;
        auto begine = strstr(temp, "T = ");
        begine += 4;
        char rst[128] = {0};
        size_t pos = 0;
        while ((*begine) != 0 && (*begine) != ';' && (*begine) != ']') {
            rst[pos++] = (*begine++);
        }
        return rst;
    }

private:
    // union {
    //     char charData;
    //     short shortData;
    //     int intData;
    //     unsigned int uintData;
    //     long long int64Data;
    //     unsigned long long uint64Data;
    //     float floatData;
    //     double doubleData;
    // } m_baseData;

    char m_baseData[16] = {0};
    String m_stringData;

    String m_typeName;
    size_t m_typeLength;
};
} // namespace neapu