#pragma once

#include "CDataModelBase.h"

class CRequestRenameFile : public CDataModelBase
{
public:
	CRequestRenameFile() = default;
	~CRequestRenameFile() = default;

	void Serialize(CMemoryStream** writer);
	void Deserialize(CMemoryReader* reader);
	CDataType GetDataType();

	std::wstring Path;
	std::wstring OldFileName;
	std::wstring NewFileName;
};

