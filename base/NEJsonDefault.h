#pragma once

#include "base/base_pub.h"
#include <map>
#include <vector>
#include <memory>
#include "base/NEString.h"

namespace neapu {
class JsonObject;
class JsonArray;
class JsonValue;

using JsonString = neapu::String;
using JsonMap = std::map<JsonString, JsonValue>;
using JsonVector = std::vector<JsonValue>;
// using JsonMapPtr = std::shared_ptr<JsonMap>;
// using JsonVectorPtr = std::shared_ptr<JsonVector>;
} // namespace neapu