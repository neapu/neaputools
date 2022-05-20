#include "NERouter.h"
using namespace neapu;

void neapu::Router::Insert(const String& _path, HttpServerCallback _cb, HttpMethod _method)
{
    m_routeMap.insert(std::make_pair(std::make_tuple(Path(_path), _method), _cb));
}

Router::HttpServerCallback neapu::Router::Find(const String& _path, HttpMethod _method)
{
    if (m_routeMap.find(std::make_tuple(Path(_path), _method)) != m_routeMap.end()) {
        return m_routeMap[std::make_tuple(Path(_path), _method)];
    }
    return {};
}

bool neapu::Path::operator==(const Path& _path) const
{
    if (m_path.Back() == '*') {
        String t1 = m_path.Left(m_path.Length() - 2);
        String t2 = _path.m_path.Left(t1.Length());
        return t1 == t2;
    }
    else {
        return m_path == _path.m_path;
    }
    return false;
}

bool neapu::Path::operator<(const Path& _path) const
{
    return m_path<_path.m_path;
}
