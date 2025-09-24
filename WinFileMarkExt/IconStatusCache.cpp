#include "stdafx.h"
#include "IconStatusCache.h"

#include "../WinFIleMarkLib/CPipeClient.h"
#include "../WinFIleMarkLib/CRequestRenameFile.h"
#include "../WinFIleMarkLib/CRequestDeleteFile.h"

IconStatusCache::IconStatusCache(const std::wstring& path) :
	_path(path)
{}

IconStatusCache::~IconStatusCache()
{}

int IconStatusCache::GetFileMarkStatus(const std::wstring& fileName)
{
	if (fileName.empty() == true)
	{
		return 0;
	}

	const auto& exists = _fileIconStatus.find(fileName);
	if (exists == _fileIconStatus.end())
	{
		// 없음
		return 0;
	}

	return exists->second;
}

void IconStatusCache::SetFileMarkStatus(const std::wstring& fileName, int iconStatus)
{
	int previousStatus = GetFileMarkStatus(fileName);

	iconStatus |= previousStatus;
	
	// 캐시 갱신 (혹은 추가)
	_fileIconStatus[fileName] = iconStatus;
}

void IconStatusCache::UnsetFileMarkStatus(const std::wstring& fileName, int iconStatus)
{
	int previousStatus = GetFileMarkStatus(fileName);
	if (previousStatus <= 0)
	{
		return;
	}

	previousStatus &= ~iconStatus;

	if (previousStatus == 0)
	{
		_fileIconStatus.erase(fileName);
	}
	else
	{
		// 캐시 갱신
		_fileIconStatus[fileName] = previousStatus;
	}
}

void IconStatusCache::RenameFileInfo(const std::wstring& oldFileName, const std::wstring& newFileName)
{
	int previousStatus = GetFileMarkStatus(oldFileName);
	if (previousStatus <= 0)
	{
		return;
	}

	_fileIconStatus.erase(oldFileName);
	_fileIconStatus[newFileName] = previousStatus;

	CRequestRenameFile req;
	req.Path = _path;
	req.OldFileName = oldFileName;
	req.NewFileName = newFileName;

	CPipeClient client;
	client.Connect(_PIPE_SERVER_NAME);
	client.Send(&req);
	client.Close();
}

void IconStatusCache::DeleteFileInfo(const std::wstring& fileName)
{
	int previousStatus = GetFileMarkStatus(fileName);
	if (previousStatus <= 0)
	{
		return;
	}

	_fileIconStatus.erase(fileName);

	CRequestDeleteFile req;
	req.Path = _path;
	req.FileName = fileName;

	CPipeClient client;
	client.Connect(_PIPE_SERVER_NAME);
	client.Send(&req);
	client.Close();
}