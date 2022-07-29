#include "NEUtil.h"
#include <utility>
#include "NEByteArray.h"
#include "sha/sha256.h"
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



int neapu::Settings::Init(String _filePath)
{
	int readSize = 0;
	char buf[BUFF_SIZE];
	m_filePath = _filePath;
	FILE* f = nullptr;
#ifdef _WIN32
	if (fopen_s(&f, _filePath.ToCString(), "r") != 0) {
		return -1;
	}
#else
	f = fopen(_filePath.ToCString(), "r");
	if (!f) {
		return -1;
	}
#endif
	String title = "non";
	while (!feof(f)) {
		//读取一行
		auto rst = fgets(buf, BUFF_SIZE-1, f);
		if (rst) {
			String str = buf;
			if (str.Back() == '\n') {
				str = str.Left(str.Length() - 1);
			}
			//处理注释
			size_t index = str.IndexOf('#');
			if (index != String::npos) {
				if (index == 0)continue;
				//井号左边的保留
				str = str.Middle(index, String::npos);
			}
			if (str.Front() == '[' && str.Back() == ']') {
				title = str.Middle(1, str.Length() - 2);
			}
			else {
				size_t index = str.IndexOf('=');
				if (index == String::npos) {
					continue;
				}
				String key = String::RemoveHeadAndTailSpace(str.Middle(0, index - 1));
				String value = String::RemoveHeadAndTailSpace(str.Middle(index + 1, String::end));
				if (value.Front() == '\"' && value.Back() == '\"') {
					value = value.Middle(1, value.Length() - 2);
				}
				m_data[title][key] = value;
			}
		}
	}
	fclose(f);
	return 0;
}

String neapu::Settings::GetValue(String _title, String _key, String _def)
{
	if (m_data.find(_title) != m_data.end()) {
		auto& kv = m_data[_title];
		if (kv.find(_key) != kv.end()) {
			return kv[_key];
		}
	}
	return _def;
}

int neapu::Settings::SetValue(String _title, String _key, String _value)
{
	FILE* f = nullptr;
#ifdef _WIN32
	if (fopen_s(&f, m_filePath.ToCString(), "rb") != 0) {
		return -1;
	}
#else
	f = fopen(m_filePath.ToCString(), "rb");
	if (!f) {
		return -1;
	}
#endif
	m_data[_title][_key] = _value;

	int readSize = 0;
	char buf[BUFF_SIZE];
	String byteBuf;
	String rst;
	while (!feof(f)) {
		readSize = fread(buf, 1, BUFF_SIZE, f);
		byteBuf.Append(buf, readSize);
	}
	fclose(f);
	f = nullptr;

	String title = "[" + _title + "]";
	size_t titlePos;
	titlePos = byteBuf.IndexOf(title);
	if (titlePos == String::npos) {
		//没有这个title也没有这个key
		//直接往文件后面追加title和key-value就好
		rst = byteBuf;
		if (rst.Back() != '\n')rst.Append('\n');
		rst.Append(title);
		rst += _key + "=" + _value;
	}
	else {
		size_t nextTitlePos = byteBuf.IndexOf("\n[");
		size_t begin = byteBuf.IndexOf(_key, titlePos);
		if (begin == String::npos || begin>nextTitlePos) {
			//有title没有key
			//在title的位置并在后面插一个key-value
			size_t titleEnd = byteBuf.IndexOf('\n', titlePos);
			if (titleEnd == String::npos) {
				//title后面什么也没有了(不应该出现的情况,结尾要是空行)
				//直接在后面加
				rst = byteBuf + "\n" + _key + "=" + _value + "\n";
			}
			else {
				rst = byteBuf.Middle(0, titleEnd);
				rst += _key + "=" + _value + "\n";
				rst += byteBuf.Middle(titleEnd + 1, String::npos);
			}
		}
		else {
			//都有就替换
			size_t end = byteBuf.IndexOf('\n', begin);
			if (end == String::npos) {
				//后面什么也没有了(不应该出现的情况,结尾要是空行)
				rst = byteBuf.Middle(0, begin-1);
				rst += _key + "=" + _value + "\n";
			}
			else {
				rst = byteBuf.Middle(0, begin - 1);
				rst += _key + "=" + _value + "\n";
				rst += byteBuf.Middle(end, String::npos);
			}
		}
	}

	//写回去
#ifdef _WIN32
	if (fopen_s(&f, m_filePath.ToCString(), "wb") != 0) {
		return -1;
	}
#else
	f = fopen(m_filePath.ToCString(), "wb");
	if (!f) {
		return -1;
	}
#endif
	size_t offset = 0;
	while (offset < rst.Length()) {
		size_t writeSize = BUFF_SIZE < rst.Length() - offset ? BUFF_SIZE : rst.Length() - offset;
		fwrite(rst.ToCString() + offset, writeSize, 1, f);
		offset += writeSize;
	}
	fclose(f);
	return 0;
}

ByteArray Encryption::sha256(const ByteArray &data)
{
	unsigned char rst[32] = {0};
	::sha256(data.Data(), data.Length(), rst);
	return ByteArray(rst, 32);
}