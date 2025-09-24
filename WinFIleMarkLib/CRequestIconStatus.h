#pragma once

#include "CDataModelBase.h"

class CRequestIconStatus : public CDataModelBase
{
public:
	CRequestIconStatus() = default;
	~CRequestIconStatus() = default;

	void Serialize(CMemoryStream** writer);
	void Deserialize(CMemoryReader* reader);
	CDataType GetDataType();

	std::wstring Path;
	DWORD Offset;
	DWORD Length;
};

