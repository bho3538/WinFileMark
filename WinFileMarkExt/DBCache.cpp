#include "stdafx.h"
#include "DBCache.h"


void DBCache::InitializeDBCache(const std::wstring& folderPath) {
	if (folderPath.empty() == true) {
		return;
	}

	if (_dbStorage.find(folderPath) != _dbStorage.end()) {
		C_ASSERT("initialize duplicate");
		return;
	}

	std::shared_ptr<DB> db = std::make_shared<DB>(folderPath);

	_dbStorage[folderPath] = db;
}

void DBCache::ReleaseDBCache(const std::wstring& folderPath) {
	if (folderPath.empty() == true) {
		return;
	}

	if (_dbStorage.find(folderPath) == _dbStorage.end()) {
		C_ASSERT("release not found");
		return;
	}

	_dbStorage.erase(folderPath);
}

std::shared_ptr<DB> DBCache::GetDBCache(const std::wstring& folderPath, bool createNewIfNotExists) {
	if (folderPath.empty() == true) {
		return nullptr;
	}

	auto found = _dbStorage.find(folderPath);
	if (found == _dbStorage.end()) { 
		if (createNewIfNotExists == true) {
			return std::make_shared<DB>(folderPath);
		}
		else {
			return nullptr;
		}
	}

	return _dbStorage[folderPath];
}

bool DBCache::CurrentFolderHasDBFile(const std::wstring& folderPath) {
	DB db(folderPath);

	return db.IsCurrentFolderHasDbFile();
}