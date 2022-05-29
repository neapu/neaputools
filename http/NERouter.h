#pragma once
#include <NEString.h>
#include <functional>
#include <vector>
#include <map>
#include "NEHttpPublic.h"
#include <memory>

namespace neapu {
    class HttpHandle;
    class Path {
    public:
        Path(const String& _path) :m_path(_path) {}
        inline bool operator==(const Path& _path) const;
        inline bool operator<(const Path& _path) const;
    private:
        String m_path;
    };
    class Router {
    public:
        using HttpServerCallback = std::function<void(std::shared_ptr<HttpHandle>)>;
        void Insert(const String& _path, HttpServerCallback _cb, HttpMethod _method);
        HttpServerCallback Find(const String& _path, HttpMethod _method);
    private:
        std::map<std::tuple<Path, HttpMethod>, HttpServerCallback> m_routeMap;
    };
}