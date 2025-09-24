#include "pch.h"
#include "InstallHelper.h"
#include <PathCch.h>

#pragma comment(lib, "Pathcch.lib")

BOOL InstallHelper::IsExtensionInstalled()
{
	LPCWSTR registryPath = L"CLSID\\{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}";
	HKEY targetKey = 0;

	if (::RegOpenKeyExW(
		HKEY_CLASSES_ROOT,
		registryPath,
		0,
		KEY_READ,
		&targetKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	::RegCloseKey(targetKey);

	return TRUE;
}

BOOL InstallHelper::IsExtensionInstalledWow64()
{
	LPCWSTR registryPath = L"CLSID\\WOW6432Node\\{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}";
	HKEY targetKey = 0;

	if (::RegOpenKeyExW(
		HKEY_CLASSES_ROOT,
		registryPath,
		0,
		KEY_READ,
		&targetKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	::RegCloseKey(targetKey);

	return TRUE;
}

DWORD InstallHelper::InstallModule()
{
	std::wstring modulePath = GetCurrentProgramPath(L"WinFileMarkExt.dll");

	// 모듈이 있는지 확인
	if (::GetFileAttributesW(modulePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return ERROR_FILE_NOT_FOUND;
	}

	// silent
	modulePath += L" /s";


	return RunRegsvr32(modulePath.c_str());
}

DWORD InstallHelper::UnInstallModule()
{
	std::wstring modulePath = GetCurrentProgramPath(L"WinFileMarkExt.dll");

	// 모듈이 있는지 확인
	if (::GetFileAttributesW(modulePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return ERROR_FILE_NOT_FOUND;
	}

	// silent + uninstall
	modulePath += L" /u /s";

	return RunRegsvr32(modulePath.c_str());
}

BOOL InstallHelper::RunRegsvr32(LPCWSTR params)
{
	SHELLEXECUTEINFOW exInfo = { 0, };
	exInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
	exInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	exInfo.lpVerb = L"runas";
	exInfo.lpFile = L"regsvr32.exe";
	exInfo.lpParameters = params;
	exInfo.nShow = SW_SHOWNORMAL;

	if (::ShellExecuteExW(&exInfo) == FALSE)
	{
		// 사용자가 Uac 대화상자에서 no 를 선택한 경우 (1223) 혹은 실행 실패한 경우
		DWORD err = ::GetLastError();

		return err;
	}

	// 종료 코드
	DWORD exitCode = 1;
	if (exInfo.hProcess != NULL)
	{
		// 종료 대기
		::WaitForSingleObject(exInfo.hProcess, INFINITE);

		::GetExitCodeProcess(
			exInfo.hProcess,
			&exitCode
		);

		::CloseHandle(exInfo.hProcess);
	}

	// 등록 성공한 경우
	if (exitCode == NO_ERROR)
	{
		return NO_ERROR;
	}

	// 등록 실패한 경우
	return exitCode;
}

std::wstring InstallHelper::GetCurrentProgramPath(LPCWSTR dllName)
{
	WCHAR currentExePath[MAX_PATH] = { 0, };

	if (::GetModuleFileNameW(
		NULL,
		currentExePath,
		MAX_PATH
	) == 0)
	{
		return L"";
	}

	// 파일 이름 제거
	if (::PathCchRemoveFileSpec(
		currentExePath,
		MAX_PATH
	) != S_OK)
	{
		return L"";
	}

	// 구하려는 파일 이름이 있는 경우
	if (dllName)
	{
		std::wstring modulePath = currentExePath;
		modulePath += L"\\";
		modulePath += dllName;

		return modulePath;
	}
	else
	{
		return currentExePath;
	}
}