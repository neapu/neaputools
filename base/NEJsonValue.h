#pragma once

#include "base/NEJsonDefault.h"
#include "base/NEString.h"
#include "logger/logger.h"
#include <cstdint>

namespace neapu {
class NEAPU_BASE_EXPORT JsonValue {
    friend class JsonObject;
    friend class JsonArray;
    friend class JsonReader;
public:
    enum class Type {
        nullValue,
        intValue,
        floatValue,
        boolValue,
        stringValue,
        arrayValue,
        objectValue,
    };

    JsonValue();
    JsonValue(int _data);
    JsonValue(int64_t _data);
    JsonValue(double _data);
    JsonValue(bool _data);
    JsonValue(const String &_data);

    bool IsInt() const
    {
        return m_type == Type::intValue;
    }
    bool IsFloat() const
    {
        return m_type == Type::floatValue;
    }
    bool IsBool() const
    {
        return m_type == Type::boolValue;
    }
    bool IsString() const
    {
        return m_type == Type::stringValue;
    }
    bool IsObject() const
    {
        return m_type == Type::objectValue;
    }
    bool IsArray() const
    {
        return m_type == Type::arrayValue;
    }

    JsonValue& operator=(int _data);
    JsonValue& operator=(int64_t _data);
    JsonValue& operator=(double _data);
    JsonValue& operator=(bool _data);
    JsonValue& operator=(const String& _data);
    JsonValue& operator=(const JsonObject& _data);
    JsonValue& operator=(const JsonArray& _data);

    int ToInt() const
    {
        return static_cast<int>(m_intData);
    }
    int64_t ToInt64() const
    {
        return m_intData;
    }
    double ToFloat() const
    {
        return m_floatData;
    }
    bool ToBool() const
    {
        return static_cast<bool>(m_intData);
    }
    JsonObject ToObject() const;
    JsonArray ToArray() const;

    void Clear();

    String ToString() const;

protected:
    Type m_type;
    int64_t m_intData = 0;
    double m_floatData = 0.f;
    JsonString m_stringData;
    JsonMap m_objectData;
    JsonVector m_arrayData;
};

} // namespace neapu

NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, const neapu::JsonValue& _jsonValue);
NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::JsonValue& _jsonValue);
NEAPU_BASE_EXPORT neapu::Logger& operator<<(neapu::Logger& _logger, neapu::JsonValue&& _jsonValue);