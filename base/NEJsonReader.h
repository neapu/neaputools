#pragma once

#include "NEJsonDefault.h"
#include "NEString.h"
#include <cstddef>

namespace neapu{
class NEAPU_BASE_EXPORT JsonReader{
public:
    bool Parse(const String& _jsonString, JsonValue& _root);

private:
    bool ParseImpl(const String& _jsonString, size_t& _index, JsonValue& _root);
};
}