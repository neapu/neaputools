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
};
} // namespace neapu