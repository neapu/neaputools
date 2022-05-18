#pragma once
#include <map>
#include <NEString.h>
#include <NEByteArray.h>

namespace neapu {
    class HttpData {
    public:
        static HttpData FromRaw(const ByteArray& _data);
        ByteArray ToRaw() const;
    protected:
        std::tuple<String, String, String> m_stateLine;
        std::map<String, String> m_headers;
        ByteArray m_body;
    };
}