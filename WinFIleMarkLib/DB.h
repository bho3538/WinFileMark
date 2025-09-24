#pragma once

typedef struct _tbInfoBar {
	std::wstring infobarMessage;
	std::wstring url;
	INT64 flags;
} TBINFOBARDATA, * PTBINFOBARDATA;

#include "CResponseIconStatus.h"
#include "CRequestSetIconStatus.h"

class DB
{
public:
	DB(LPCWSTR folderPath);
	DB(const std::wstring& folderPath);

	bool IsCurrentFolderHasDbFile();

	bool GetInfobarMessage(PTBINFOBARDATA pInfobarData);
	int GetFileMarkStatus(const std::wstring& fileName);
	void SetFileMarkStatus(const std::wstring& fileName, int iconStatus, int currentIconStatus);
	void UnsetFileMarkStatus(const std::wstring& fileName, int iconStatus, int currentIconStatus);
	void RenameFileInfo(const std::wstring& oldFileName, const std::wstring& newFileName);
	void DeleteFileInfo(const std::wstring& fileName);

	void FindFilesMarkStatus(CResponseIconStatus* storage, DWORD offset, DWORD length);

	void SetFilesMarkStatus(CRequestSetIconStatus* req);
	void UnSetFilesMarkStatus(CRequestSetIconStatus* req);

private:
	bool CreateDbFileAndInitializeIt();

	std::wstring _dbPath;
	std::string _dbPathUtf8;
};

