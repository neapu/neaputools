#include "NEUtil.h"
using namespace neapu;

Arguments::Arguments(int argc, char** argv)
{
	for (int i = 0; i < argc; i++) {
		String str(argv[i]);
		// --opt »ò --key=val
		if (str.length() >= 3 && (str[0] == '-' && str[1] == '-')) {
			auto index = str.IndexOf('=');
			String key, val;
			if (index != String::npos) {
				key = str.Middle(2, index-1);
				val = str.Middle(index + 1, String::npos);
			}
			m_args[key] = val;
		}
		// -opt »ò -key val
		else if (str.length() >= 2 && (str[0] == '-')) {
			String key, val;
			key = str.Middle(1, String::npos);
			if (i + 1 < argc && argv[i+1][0] != '-') {
				val = argv[i + 1];
			}
			m_args[key] = val;
		}
	}
}

bool Arguments::ExistOpt(String opt)
{
	if (m_args.find(opt) != m_args.end()) {
		return true;
	}
	return false;
}

String Arguments::GetValue(String key, String def)
{
	if (m_args.find(key) != m_args.end()) {
		return m_args[key];
	}
	return def;
}
