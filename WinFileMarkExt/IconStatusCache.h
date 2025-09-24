#pragma once

#include <map>

class IconStatusCache
{
public:
	IconStatusCache(const std::wstring& path);
	~IconStatusCache();

	// bool GetInfobarMessage(PTBINFOBARDATA pInfobarData);
	int GetFileMarkStatus(const std::wstring& fileName);
	void SetFileMarkStatus(const std::wstring& fileName, int iconStatus);
	void UnsetFileMarkStatus(const std::wstring& fileName, int iconStatus);
	void RenameFileInfo(const std::wstring& oldFileName, const std::wstring& newFileName);
	void DeleteFileInfo(const std::wstring& fileName);

private:

	// 폴더 경로
	std::wstring _path;

	// 파일에 대한 아이콘 상태
	std::map<std::wstring, int> _fileIconStatus;
};

