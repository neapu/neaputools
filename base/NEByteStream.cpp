#include "NEByteStream.h"

using namespace neapu;

ByteArray ByteStream::Read(size_t _len)
{
    if (m_offset >= m_len)return ByteArray();
    size_t count = _len;
    if (m_offset + _len > m_len) {
        count = m_len - m_offset;
    }
    ByteArray rst(m_data + m_offset, count);
    m_offset += count;
    return rst;
}

ByteArray ByteStream::ReadTo(const ByteArray& _sub, bool _includeSubstring)
{
    ByteArray rst;
    if (m_offset >= m_len || _sub.IsEmpty())return rst;
    size_t index = Find(_sub, m_offset);
    if (index == npos) {
        rst = Middle(m_offset, npos);
        m_offset = m_len;
    }
    else if (index == 0) {
        m_offset += _sub.Length();
        if (_includeSubstring) {
            rst = _sub;
        }
    }
    else if (_includeSubstring) {
        index += _sub.Length();
        rst = Middle(m_offset, index - 1);
        m_offset = index;
    }
    else {
        rst = Middle(m_offset, index - 1);
        index += _sub.Length();
        m_offset = index;
    }
    return rst;
}

ByteArray neapu::ByteStream::ReadLineCRLF(bool _includeSubstring)
{
    return ReadTo(ByteArray("\r\n", 2), _includeSubstring);
}

ByteArray neapu::ByteStream::ReadLineLF(bool _includeSubstring)
{
    return ReadTo(ByteArray("\n", 1), _includeSubstring);
}

ByteArray neapu::ByteStream::ReadToEnd()
{
    ByteArray rst;
    if (m_offset >= m_len)return rst;
    rst = Middle(m_offset, m_len);
    m_offset = m_len;
    return rst;
}

void neapu::ByteStream::MoveTo(size_t _offset)
{
    if (_offset > m_len) {
        m_offset = m_len;
    }
    else {
        m_offset = _offset;
    }
}
