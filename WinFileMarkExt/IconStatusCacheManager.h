#pragma once

#include <map>
#include "IconStatusCache.h"

class IconStatusCacheManager
{
public:
	IconStatusCache* GetFolderCache(const std::wstring& folderPath);
	IconStatusCache* CreateFolderCache(const std::wstring& folderPath);
	void ReleaseFolderCache(const std::wstring& folderPath);

	void MoveItemCache(const std::wstring& originalFolderPath, const std::wstring& originalFileName, const std::wstring& newFolderPath, const std::wstring& newFileName);

private:

	std::map<std::wstring, IconStatusCache*> _iconStatusInfos;
};

