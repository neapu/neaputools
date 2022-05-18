#pragma once
#include <map>
#include "NEString.h"
#include "base_pub.h"

namespace neapu {
	class NEAPU_BASE_EXPORT Arguments {
	public:
		Arguments(int argc, char** argv);
		bool ExistOpt(String opt);
		String GetValue(String key, String def = String());
	private:
		std::map<String, String> m_args;
	};

	class NEAPU_BASE_EXPORT Settings {
	public:
		int Init(String _filePath);
		String GetValue(String _title, String _key, String _def);
		int SetValue(String _title, String _key, String _value);
	private:
		String m_filePath;
		std::map <String, std::map<String, String>> m_data;
	};
}