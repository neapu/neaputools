#pragma once
#include <map>
#include "NEByteArray.h"
#include "NEString.h"
#include "base_pub.h"

namespace neapu {
class NEAPU_BASE_EXPORT Arguments {
public:
    Arguments(int argc, char **argv);
    bool ExistOpt(String opt);
    String GetValue(String key, String def = String());

private:
    std::map<String, String> m_args;
};

class NEAPU_BASE_EXPORT Settings {
public:
    int Init(String _filePath);
    String GetValue(String _title, String _key, String _def = String());
    int SetValue(String _title, String _key, String _value);

private:
    String m_filePath;
    std::map<String, std::map<String, String>> m_data;
};

class NEAPU_BASE_EXPORT Encryption {
public:
    static ByteArray sha256(const ByteArray &data);
};
} // namespace neapu