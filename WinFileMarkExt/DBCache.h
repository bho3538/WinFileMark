#pragma once

#include <map>
#include <memory>
#include "../WinFIleMarkLib/DB.h"

class DBCache
{
public:
	void InitializeDBCache(const std::wstring& folderPath);
	void ReleaseDBCache(const std::wstring& folderPath);
	std::shared_ptr<DB> GetDBCache(const std::wstring& folderPath, bool createNewIfNotExists);

	static bool CurrentFolderHasDBFile(const std::wstring& folderPath);
private:

	// todo
	//   이것이 thread safe 해야 하나?
	std::map<std::wstring, std::shared_ptr<DB>> _dbStorage;
};

