#pragma once
#include <map>
#include "base/NEByteArray.h"
#include "base/NEString.h"
#include "base/base_pub.h"

namespace neapu {
class NEAPU_BASE_EXPORT Arguments {
public:
    Arguments(int argc, char **argv);
    bool ExistOpt(String opt);
    String GetValue(String key, String def = String());

private:
    std::map<String, String> m_args;
};

class NEAPU_BASE_EXPORT Encryption {
public:
    static ByteArray sha256(const ByteArray &data);
    static String Base64Encode(const ByteArray &data);
    static ByteArray Base64Decode(const String &data);
};
} // namespace neapu