#pragma once

#include "CDataModelBase.h"

class CRequestMoveFile : public CDataModelBase
{
public:
	CRequestMoveFile() = default;
	~CRequestMoveFile() = default;

	void Serialize(CMemoryStream * *writer);
	void Deserialize(CMemoryReader * reader);
	CDataType GetDataType();

	std::wstring OldPath;
	std::wstring OldFileName;
	std::wstring NewPath;
	std::wstring NewFileName;
};

