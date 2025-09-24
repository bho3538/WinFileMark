#pragma once

#include "CDataModelBase.h"
#include <vector>

class CRequestSetIconStatus : public CDataModelBase
{
public:
	CRequestSetIconStatus(const std::wstring& path);
	CRequestSetIconStatus();
	~CRequestSetIconStatus();

	void Serialize(CMemoryStream** writer);
	void Deserialize(CMemoryReader* reader);
	CDataType GetDataType();
	void AddItem(const std::wstring& fileName);
	const std::vector<std::wstring>& GetItems();

	std::wstring Path;
	bool UnsetMode;
	int StatusIconFlags;

private:

	std::vector<std::wstring> _fileNames;
};

