#include "NEJsonReader.h"
#include "NEJsonDefault.h"
#include "NEJsonValue.h"
#include "NEString.h"
#include <cstddef>
#include <cstdint>

using namespace neapu;

bool JsonReader::Parse(const String &jsonString, JsonValue &root)
{
    size_t index = 0;
    return ParseImpl(jsonString, index, root);
}

static void SkipEmpty(const String &_jsonString, size_t &_index)
{
    for (; _index < _jsonString.Length(); _index++) {
        if (_jsonString[_index] == ' ' || _jsonString[_index] == '\r'
            || _jsonString[_index] == '\n'
            || _jsonString[_index] == '\t') {
        } else {
            break;
        }
    }
}

bool JsonReader::ParseImpl(const String &_jsonString, size_t &_index, JsonValue &_root)
{
    if (_index >= _jsonString.Length()) {
        return false;
    }
    if (_jsonString[_index] == '{') {
        _root.m_type = JsonValue::Type::objectValue;
        _index++;
        while (true) {
            SkipEmpty(_jsonString, _index);
            if (_index >= _jsonString.Length()) {
                return false;
            }

            if (_jsonString[_index] == '}') {
                _index++;
                return true;
            }

            if (_jsonString[_index] != '\"') {
                return false;
            }
            size_t startIndex = _index;
            size_t endIndex = startIndex;
            _index++;
            // 找结束引号
            for (; _index < _jsonString.Length(); _index++) {
                if (_jsonString[_index] == '\"' && _jsonString[_index - 1] != '\\') {
                    endIndex = _index;
                    break;
                }
            }
            if (endIndex == startIndex) {
                return false;
            }

            String key = _jsonString.Middle(startIndex + 1, endIndex - 1);
            if (_root.m_objectData.find(key) != _root.m_objectData.end()) {
                return false;
            }
            _index++;
            SkipEmpty(_jsonString, _index);
            if (_index >= _jsonString.Length()) {
                return false;
            }

            if (_jsonString[_index] != ':') {
                return false;
            }
            _index++;
            SkipEmpty(_jsonString, _index);
            if (_index >= _jsonString.Length()) {
                return false;
            }

            JsonValue value;
            bool ret = ParseImpl(_jsonString, _index, value);
            if (!ret) return ret;

            _root.m_objectData[key] = value;

            SkipEmpty(_jsonString, _index);
            if (_index >= _jsonString.Length()) {
                return false;
            }

            if (_jsonString[_index] == ',') {
                _index++;
                SkipEmpty(_jsonString, _index);
                if (_index >= _jsonString.Length()) {
                    return false;
                }
            }
        }

    } else if (_jsonString[_index] == '[') {
        _root.m_type = JsonValue::Type::arrayValue;
        _index++;
        while (true) {
            SkipEmpty(_jsonString, _index);
            if (_index >= _jsonString.Length()) {
                return false;
            }
            if (_jsonString[_index] == ']') {
                _index++;
                return true;
            }
            JsonValue value;
            bool ret = ParseImpl(_jsonString, _index, value);
            if (!ret) return ret;
            _root.m_arrayData.push_back(value);

            SkipEmpty(_jsonString, _index);
            if (_index >= _jsonString.Length()) {
                return false;
            }

            if (_jsonString[_index] == ',') {
                _index++;
                SkipEmpty(_jsonString, _index);
                if (_index >= _jsonString.Length()) {
                    return false;
                }
            }
        }
    } else if (_jsonString[_index] == '\"') {
        _root.m_type = JsonValue::Type::stringValue;
        size_t startIndex = _index;
        size_t endIndex = startIndex;
        _index++;
        for (; _index < _jsonString.Length(); _index++) {
            if (_jsonString[_index] == '\"' && _jsonString[_index - 1] != '\\') {
                endIndex = _index;
                break;
            }
        }
        if (endIndex == startIndex) {
            return false;
        }
        _root.m_stringData = _jsonString.Middle(startIndex + 1, endIndex - 1);
        _index++;
        return true;
    } else if (_jsonString[_index] >= '0' && _jsonString[_index] <= '9') {
        int64_t front = 0;
        int64_t back = 0;
        for (; _index < _jsonString.Length(); _index++) {
            if (_jsonString[_index] < '0' || _jsonString[_index] > '9') {
                break;
            }
            front *= 10;
            front += _jsonString[_index] - '0';
        }
        if (_index >= _jsonString.Length()) {
            return false;
        }
        if (_jsonString[_index] != '.') {
            _root.m_type = JsonValue::Type::intValue;
            _root.m_intData = front;
            return true;
        }
        _index++;

        for (; _index < _jsonString.Length(); _index++) {
            if (_jsonString[_index] < '0' || _jsonString[_index] > '9') {
                break;
            }
            back *= 10;
            back += _jsonString[_index] - '0';
        }
        double temp = 0;
        while (back) {
            temp += (back % 10);
            temp /= 10.f;
            back /= 10;
        }
        temp += front;
        _root.m_type = JsonValue::Type::floatValue;
        _root.m_floatData = temp;
        return true;
    } else {
        if (_index + 4 < _jsonString.Length() && memcmp(_jsonString.ToCString() + _index, "true", 4) == 0) {
            _root.m_type = JsonValue::Type::boolValue;
            _root.m_intData = 1;
            _index += 4;
        } else if (_index + 5 < _jsonString.Length() && memcmp(_jsonString.ToCString() + _index, "false", 5) == 0) {
            _root.m_type = JsonValue::Type::boolValue;
            _root.m_intData = 0;
            _index += 5;
        }
        return true;
    }
    return false;
}