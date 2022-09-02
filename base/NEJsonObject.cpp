#include "NEJsonObject.h"
#include "NEJsonDefault.h"
#include "NEJsonValue.h"
#include "NEString.h"

using namespace neapu;

JsonObject::JsonObject(const JsonValue &_value)
    : JsonValue(_value)
{}

JsonValue JsonObject::Get(const String &key) const
{
    return Get(key, JsonValue());
}

JsonValue JsonObject::Get(const String &key, const JsonValue &defValue) const
{
    if (m_objectData.find(key) != m_objectData.end()) {
        return m_objectData.at(key);
    }
    return defValue;
}

JsonValue &JsonObject::operator[](const String &key)
{
    if (m_objectData.find(key) == m_objectData.end()) {
        m_objectData.emplace(key, JsonValue());
    }
    return m_objectData[key];
}

JsonObject &JsonObject::operator=(const JsonValue &_value)
{
    m_objectData = _value.m_objectData;
    return (*this);
}

String JsonObject::ToString() const
{
    String rst;
    rst.Append("{");
    for(auto& item:m_objectData){
        rst.Append("\"%1\":").Argument(item.first);
        if(item.second.IsString()){
            rst.Append("\"%1\",").Argument(item.second.ToString());
        }else{
            rst.Append(item.second.ToString());
            rst.Append(',');
        }
    }
    if(rst.Back()!='{'){
        rst.RemoveBack(1);
    }
    rst.Append("}");
    return rst;
}