#include "NESettings.h"
#include "base/NEFile.h"
#include "base/NEString.h"
#include <functional>

using namespace neapu;

static void ParseFile(const String& data, std::function<bool(const String& key, const String& value)> cb)
{
    for (auto line : data.Split('\n', true)) {
        line = String::RemoveHeadAndTailSpace(line);
        if (line[0] == '#') {
            continue;
        }
        auto index = line.IndexOf('=');
        if (index == String::npos) {
            continue;
        }
        String key = line.Middle(0, index).RemoveHeadAndTailSpace();
        String value = line.Middle(index + 1, String::end);
        if (!cb(key, value)) {
            break;
        }
    }
}

int Settings::Init(const String& _filename)
{
    m_filename = _filename;

    return LoadFile();
}

String Settings::GetValue(const String& key, const String& _default, bool _loadFile)
{
    if (_loadFile) {
        int ret = LoadFile();
        if (ret < 0) {
            return _default;
        }
    }
    if (!m_items.contains(key)) {
        return _default;
    }
    return m_items[key];
}

int Settings::SetValue(const String& key, const String& value)
{
    File settingFile(m_filename);
    if (!settingFile.Open(File::OpenMode::ReadWrite)) {
        return -1;
    }
    String data = settingFile.Read();
    bool find = false;
    ParseFile(data, [&](const String& skey, const String& svalue) {
        if (skey == key) {
            data.Replace(svalue, value);
            find = true;
            return false;
        }
        return true;
    });

    if (!find) {
        if (data.Length() != 0 && data.Back() != '\n') {
            data.Append('\n');
        }
        data.Append(String("%1=%2").Argument(key).Argument(value));
    }
    settingFile.Write(data);
    settingFile.Close();

    m_items[key] = value;
    return 0;
}

int Settings::LoadFile()
{
    File settingFile(m_filename);
    if (!settingFile.Open(File::OpenMode::ReadOnly)) {
        return -1;
    }
    m_items.clear();
    String data = settingFile.Read();
    settingFile.Close();
    ParseFile(data, [this](const String& key, const String& value) {
        m_items[key] = value;
        return true;
    });
    return 0;
}