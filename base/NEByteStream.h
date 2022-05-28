#pragma once
#include "NEByteArray.h"
#include "base_pub.h"

namespace neapu {
    class NEAPU_BASE_EXPORT ByteStream : public ByteArray {
    public:
        using ByteArray::ByteArray;

        ByteArray Read(size_t _len);
        /*******************************************
        * 读取从offset到子串_sub所在的index
        * 如果没找到子串则读取到末尾
        * _includeSubstring: 读取的内容是否包含该子串，不管是否包含，offset都会移动到子串的下一个字符
        ********************************************/
        ByteArray ReadTo(const ByteArray& _sub, bool _includeSubstring = true);
        ByteArray ReadLineCRLF(bool _includeSubstring = false);
        ByteArray ReadLineLF(bool _includeSubstring = false);
        ByteArray ReadToEnd();

        size_t Offset() const { return m_offset; }
        void MoveTo(size_t _offset);
    private:
        size_t m_offset = 0;
    };
}