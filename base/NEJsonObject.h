#pragma once

#include "base/NEJsonDefault.h"
#include "base/NEJsonValue.h"
#include "base/NEString.h"

namespace neapu {
class NEAPU_BASE_EXPORT JsonObject : public JsonValue {
    friend class JsonValue;
    friend class JsonArray;

public:
    JsonObject()
    {}
    JsonObject(const JsonValue &_value);

    JsonValue Get(const String &key) const;
    JsonValue Get(const String &key, const JsonValue &defValue) const;

    JsonValue &operator[](const String &key);
    JsonObject &operator=(const JsonValue &_value);

    bool IsEmpty() const
    {
        return m_objectData.empty();
    }

    using iterator = JsonMap::iterator;
    JsonObject::iterator begin()
    {
        return m_objectData.begin();
    }

    JsonObject::iterator end()
    {
        return m_objectData.end();
    }

    String ToString() const;

    int64_t ToInt() const = delete;
    double ToFloat() const = delete;
    bool ToBool() const = delete;
    JsonObject ToObject() const = delete;
    JsonArray ToArray() const = delete;
};
} // namespace neapu