#include "NEUtil.h"
#include <cstddef>
#include <memory>
#include <utility>
#include "NEByteArray.h"
#include "NEString.h"
#include "sha/sha256.h"
#include "base64/base64.h"
using namespace neapu;
constexpr auto BUFF_SIZE = 8192;

Arguments::Arguments(int argc, char** argv)
{
	for (int i = 0; i < argc; i++) {
		String str(argv[i]);
		// --opt 或 --key=val
		if (str.Length() >= 3 && (str[0] == '-' && str[1] == '-')) {
			auto index = str.IndexOf('=');
			String key, val;
			if (index != String::npos) {
				key = str.Middle(2, index-1);
				val = str.Middle(index + 1, String::npos);
			}
			m_args[key] = val;
		}
		// -opt 或 -key val
		else if (str.Length() >= 2 && (str[0] == '-')) {
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


ByteArray Encryption::sha256(const ByteArray &data)
{
	unsigned char rst[32] = {0};
	::sha256(data.Data(), data.Length(), rst);
	return ByteArray(rst, 32);
}

String Encryption::Base64Encode(const ByteArray &data)
{
	auto buf = std::unique_ptr<char>(new char[data.Length()*2]);
	base64_encode(data.Data(), buf.get(), (int)data.Length());
	return String(buf.get());
}

ByteArray Encryption::Base64Decode(const String &data)
{
	auto buf = std::unique_ptr<unsigned char>(new unsigned char[data.Length()]);
	int len = base64_decode(data.ToCString(), buf.get());
	return ByteArray(buf.get(), size_t(len));
}