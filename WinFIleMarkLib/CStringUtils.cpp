#include "stdafx.h"
#include "CStringUtils.h"

std::wstring CStringUtils::Utf8ToUtf16(const std::string& str) {
	if (str.empty() == true) {
		return L"";
	}

	std::wstring buffer;
	int bufferSize = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.data(), -1, nullptr, 0);
	if (bufferSize == 0) {
		return L"";
	}

	buffer.resize(bufferSize);

	int re = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.data(), -1, const_cast<LPWSTR>(buffer.data()), static_cast<int>(buffer.capacity()));
	if (re == 0) {
		return L"";
	}

	return buffer;
}

std::string CStringUtils::Utf16ToUtf8(const std::wstring& str) {
	if (str.empty() == true) {
		return "";
	}

	std::string buffer;
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, str.data(), str.length(), nullptr, 0, nullptr, nullptr);
	if (bufferSize == 0) {
		return "";
	}

	buffer.resize(bufferSize);

	int re = WideCharToMultiByte(CP_UTF8, 0, str.data(), str.length(), const_cast<LPSTR>(buffer.c_str()), static_cast<int>(buffer.capacity()), nullptr, nullptr);
	if (re == 0) {
		return "";
	}

	return buffer;
}