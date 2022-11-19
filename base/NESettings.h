#ifndef __NESETTINGS_H__
#define __NESETTINGS_H__

#include "base/NEFile.h"
#include "base/NEString.h"
#include <map>
namespace neapu {
class Settings {
public:
    Settings() {}

    int Init(const String& _filename);
    String GetValue(const String& key, bool _loadFile = false);
    int SetValue(const String& key, const String& value);
private:
    int LoadFile();
private:
    String m_filename;
    std::map<String, String> m_items;
};
} // namespace neapu
#endif // __NESETTINGS_H__