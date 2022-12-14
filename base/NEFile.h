#pragma once
#include "base/base_pub.h"
#include "base/NEString.h"
#include "base/NEByteArray.h"
#include <stdio.h>
namespace neapu {
class NEAPU_BASE_EXPORT File {
public:
    enum class OpenMode : char {
        ReadOnly,
        WriteOnly,
        ReadWrite,
        Append,
        ReadAppend
    };

    File() {}
    File(const String& _filename);
    ~File() noexcept;

    bool Open(OpenMode _type, bool _binary = true);
    bool Open(const String& _filename, OpenMode _type, bool _binary = true);
    void Close();
    void Flush();
    ByteArray Read(size_t _readCount = ByteArray::npos);
    size_t Write(const ByteArray& _ba);
    String Extension();
    bool IsOpened() { return m_file == nullptr; }

private:
    FILE* m_file = nullptr;
    String m_filename;
};
} // namespace neapu