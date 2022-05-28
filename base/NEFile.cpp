#include "NEFile.h"
using namespace neapu;

constexpr auto BUF_SIZE = 1024;

neapu::File::File(const String& _filename)
    : m_filename(_filename)
{
}

neapu::File::~File() noexcept
{
    Close();
}

bool neapu::File::Open(OpenMode _type, bool _binary)
{
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
    String mode;
    switch (_type)
    {
    case neapu::File::OpenMode::READ:
        mode = "r";
        break;
    case neapu::File::OpenMode::WRITE:
        mode = "w";
        break;
    case neapu::File::OpenMode::READWRITE:
        mode = "w+";
        break;
    case neapu::File::OpenMode::APPEND:
        mode = "a";
        break;
    case neapu::File::OpenMode::READAPPEND:
        mode = "a+";
        break;
    default:
        return false;
        break;
    }
    if (_binary) {
        mode += "b";
    }
#ifdef _WIN32
    int rc = fopen_s(&m_file, m_filename.ToCString(), mode.ToCString());
    if (rc == 0) {
        return true;
    }
#else
    m_file = fopen(m_filename.ToCString(), mode.ToCString());
    if (m_file) {
        return true;
    }
#endif
    return false;
}

bool neapu::File::Open(const String& _filename, OpenMode _type, bool _binary)
{
    m_filename = _filename;
    return Open(_type, _binary);
    return false;
}

void neapu::File::Close()
{
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
}

void neapu::File::Flush()
{
    if (m_file) {
        fflush(m_file);
    }
}

ByteArray neapu::File::Read(size_t _readCount)
{
    ByteArray rst;
    if (!m_file) {
        return rst;
    }
    size_t count = 0;
    size_t readSize = 0;
    char buf[BUF_SIZE] = { 0 };
    while (!feof(m_file) && count < _readCount) {
        readSize = fread(buf, 1, BUF_SIZE, m_file);
        if (readSize > 0) {
            rst.Append(buf, readSize);
            count += readSize;
        }
    }
    return rst;
}

size_t neapu::File::Write(const ByteArray& _ba)
{
    if (!m_file) {
        return 0;
    }
    size_t offset = 0;
    size_t writeSize = 0;
    while (offset < _ba.Length()) {
        writeSize = BUF_SIZE < _ba.Length() - offset ? BUF_SIZE : _ba.Length() - offset;
        fwrite(_ba.Data() + offset, writeSize, 1, m_file);
        offset += writeSize;
    }
    return offset;
}

String neapu::File::Extension()
{
    String rst;
    if (m_filename.IsEmpty())return rst;
    size_t index = m_filename.LastIndexOf('.');
    if (index != String::npos) {
        return m_filename.Middle(index + 1, String::end);
    }
    return String();
}

