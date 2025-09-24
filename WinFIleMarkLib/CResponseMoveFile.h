#pragma once

#include "CDataModelBase.h"

class CResponseMoveFile : public CDataModelBase
{
public:
	CResponseMoveFile() :
		IconStatus(0)
	{};

	~CResponseMoveFile() = default;

	void Serialize(CMemoryStream** writer);
	void Deserialize(CMemoryReader* reader);
	CDataType GetDataType();

	int IconStatus;
};

