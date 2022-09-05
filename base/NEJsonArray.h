#pragma once

#include "NEJsonDefault.h"
#include "NEJsonValue.h"
#include "NEString.h"

namespace neapu {
class JsonArray : public JsonValue {
    friend class JsonValue;
    friend class JsonObject;

public:
    JsonArray();
    JsonArray(const JsonValue &_value);
    String ToString() const;

    using iterator = JsonVector::iterator;
    JsonArray::iterator begin()
    {
        return m_arrayData.begin();
    }
    JsonArray::iterator end()
    {
        return m_arrayData.end();
    }

    int64_t ToInt() const = delete;
    double ToFloat() const = delete;
    bool ToBool() const = delete;
    JsonObject ToObject() const = delete;
    JsonArray ToArray() const = delete;
};
} // namespace neapu