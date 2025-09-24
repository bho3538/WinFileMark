#pragma once

#include "CDataModelBase.h"

class CRequestDeleteFile : public CDataModelBase
{
public:
	CRequestDeleteFile() = default;
	~CRequestDeleteFile() = default;

	void Serialize(CMemoryStream * *writer);
	void Deserialize(CMemoryReader * reader);
	CDataType GetDataType();

	std::wstring Path;
	std::wstring FileName;
};

