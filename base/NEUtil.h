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
}