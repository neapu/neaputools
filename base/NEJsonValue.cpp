#include "NEJsonValue.h"
#include "NEJsonDefault.h"
#include "NEString.h"
#include "NEJsonObject.h"
#include "NEJsonArray.h"
#include <cstdint>

using namespace neapu;

JsonValue::JsonValue()
    : m_type(Type::nullValue)
{
}

JsonValue::JsonValue(int _data)
    : m_intData(static_cast<int64_t>(_data)), m_type(Type::intValue)
{
}

JsonValue::JsonValue(int64_t _data)
    : m_intData(_data), m_type(Type::intValue)
{
}

JsonValue::JsonValue(double _data)
    : m_floatData(_data), m_type(Type::floatValue)
{
}

JsonValue::JsonValue(bool _data)
    : m_intData(static_cast<int64_t>(_data)), m_type(Type::boolValue)
{
}

JsonValue::JsonValue(const String &_data)
    : m_stringData(_data), m_type(Type::stringValue)
{
}

JsonObject JsonValue::ToObject() const
{
    if (m_type == Type::objectValue) {
        return JsonObject(*this);
    }
    return JsonObject();
}

JsonArray JsonValue::ToArray() const
{
    if (m_type == Type::arrayValue) {
        return JsonArray(*this);
    }
    return JsonArray();
}

JsonValue &JsonValue::operator=(int _data)
{
    Clear();
    m_type = Type::intValue;
    m_intData = static_cast<int64_t>(_data);
    return (*this);
}

JsonValue &JsonValue::operator=(int64_t _data)
{
    Clear();
    m_type = Type::intValue;
    m_intData = _data;
    return (*this);
}

JsonValue &JsonValue::operator=(double _data)
{
    Clear();
    m_type = Type::floatValue;
    m_floatData = _data;
    return (*this);
}

JsonValue &JsonValue::operator=(bool _data)
{
    Clear();
    m_type = Type::boolValue;
    m_intData = static_cast<bool>(_data);
    return (*this);
}

JsonValue &JsonValue::operator=(const String &_data)
{
    Clear();
    m_type = Type::stringValue;
    m_stringData = _data;
    return (*this);
}

JsonValue &JsonValue::operator=(const JsonObject &_data)
{
    Clear();
    m_type = Type::objectValue;
    m_objectData = _data.m_objectData;
    return (*this);
}

JsonValue &JsonValue::operator=(const JsonArray &_data)
{
    Clear();
    m_type = Type::arrayValue;
    m_arrayData = _data.m_arrayData;
    return (*this);
}

void JsonValue::Clear()
{
    m_type = Type::nullValue;
    m_intData = 0;
    m_floatData = 0;
    m_stringData.Clear();
    m_arrayData.clear();
    m_objectData.clear();
}

String JsonValue::ToString() const
{
    switch (m_type) {
    case Type::nullValue:
        return "NULL";
        break;
    case Type::intValue:
        return String::ToString((long long)m_intData);
        break;
    case Type::floatValue:
        return String::ToString(m_floatData);
        break;
    case Type::boolValue:
        return m_intData ? "true" : "false";
        break;
    case Type::stringValue:
        return m_stringData;
        break;
    case Type::arrayValue:
        return static_cast<const JsonArray *>(this)->ToString();
        break;
    case Type::objectValue:
        return static_cast<const JsonObject *>(this)->ToString();
        break;
    }
    return String("NULL");
}