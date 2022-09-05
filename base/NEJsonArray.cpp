#include "NEJsonArray.h"
#include "NEJsonDefault.h"
#include "NEJsonValue.h"
#include "NEString.h"

using namespace neapu;

JsonArray::JsonArray()
{}

JsonArray::JsonArray(const JsonValue &_value)
    : JsonValue(_value)
{}

String JsonArray::ToString() const
{
    String rst;
    rst.Append("[");
    for (auto &item : m_arrayData) {
        rst.Append(item.ToString());
        rst.Append(',');
    }
    if (rst.Back() != '[') {
        rst.RemoveBack(1);
    }
    rst.Append("]");
    return rst;
}