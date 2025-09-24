#include "stdafx.h"
#include "IconStatusCacheManager.h"

#include "../WinFIleMarkLib/CPipeClient.h"
#include "../WinFIleMarkLib/CRequestIconStatus.h"
#include "../WinFIleMarkLib/CResponseIconStatus.h"
#include "../WinFIleMarkLib/CRequestMoveFile.h"
#include "../WinFIleMarkLib/CResponseMoveFile.h"

IconStatusCache* IconStatusCacheManager::GetFolderCache(const std::wstring& folderPath)
{
	if (folderPath.empty() == true)
	{
		return nullptr;
	}

	const auto& exists = _iconStatusInfos.find(folderPath);
	if (exists == _iconStatusInfos.end())
	{
		return nullptr;
	}

	return exists->second;
}

IconStatusCache* IconStatusCacheManager::CreateFolderCache(const std::wstring& folderPath)
{
	IconStatusCache* cache = GetFolderCache(folderPath);
	if (cache == nullptr)
	{
		cache = new IconStatusCache(folderPath);

		CRequestIconStatus req;
		req.Path = folderPath;
		req.Offset = 1;
		req.Length = 1;

		CResponseIconStatus res;

		CPipeClient client;
		client.Connect(_PIPE_SERVER_NAME);
		client.Send(&req);
		client.Receive(&res);
		client.Close();

		const auto& item = res.GetItems();
		DWORD itemCnt = item.size();

		for (DWORD i = 0; i < itemCnt; i++)
		{
			cache->SetFileMarkStatus(item[i]->ItemName, item[i]->IconStatus);
		}

		_iconStatusInfos[folderPath] = cache;
	}
	else
	{
		C_ASSERT(L"bomb");
	}

	return cache;
}

void IconStatusCacheManager::ReleaseFolderCache(const std::wstring& folderPath)
{
	IconStatusCache* cache = GetFolderCache(folderPath);
	if (cache != nullptr)
	{
		_iconStatusInfos.erase(folderPath);

		delete cache;
	}
}

void IconStatusCacheManager::MoveItemCache(const std::wstring& originalFolderPath, const std::wstring& originalFileName, const std::wstring& newFolderPath, const std::wstring& newFileName)
{
	CRequestMoveFile req;
	req.OldPath = originalFolderPath;
	req.OldFileName = originalFileName;
	req.NewPath = newFolderPath;
	req.NewFileName = newFileName;

	CResponseMoveFile res;

	CPipeClient client;
	client.Connect(_PIPE_SERVER_NAME);
	client.Send(&req);
	client.Receive(&res);
	client.Close();

	if (res.IconStatus <= 0)
	{
		return;
	}

	IconStatusCache* cache = GetFolderCache(originalFolderPath);
	if (cache != nullptr)
	{
		cache->DeleteFileInfo(originalFileName);
	}

	cache = GetFolderCache(newFolderPath);
	if (cache != nullptr)
	{
		cache->SetFileMarkStatus(newFileName, res.IconStatus);
	}
}