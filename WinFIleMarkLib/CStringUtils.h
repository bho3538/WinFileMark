#pragma once

class CStringUtils
{
public:
	static std::wstring Utf8ToUtf16(const std::string& str);
	static std::string Utf16ToUtf8(const std::wstring& str);
};

