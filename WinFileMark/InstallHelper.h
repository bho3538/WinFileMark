#pragma once

#include <string>

class InstallHelper
{
public:

	static BOOL IsExtensionInstalled();
	static BOOL IsExtensionInstalledWow64();

	static DWORD InstallModule();
	static DWORD UnInstallModule();

private:
	
	static BOOL RunRegsvr32(LPCWSTR params);

	static std::wstring GetCurrentProgramPath(LPCWSTR dllName);
};

