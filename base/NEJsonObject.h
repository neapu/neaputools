#pragma once

#include "NEJsonDefault.h"
#include "NEJsonValue.h"
#include "NEString.h"

namespace neapu {
class JsonObject : public JsonValue {
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

};
} // namespace neapu