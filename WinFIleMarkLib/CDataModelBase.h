#pragma once

#include "CMemoryReader.h"
#include "CMemoryStream.h"

enum class CDataType
{
	REQUEST_ICONSTATUS = 0x1,
	REPLY_ICONSTATUS,
	REQUEST_SETICONSTATUS,
	REPLY_SETICONSTATUS,
	REQUEST_RENAMEFILE,
	REPLY_RENAMEFILE,
	REQUEST_DELETEFILE,
	REPLY_DELETEFILE,
	REQUEST_MOVEFILE,
	REPLY_MOVEFILE
};

class CDataModelBase
{
public:
	virtual void Serialize(CMemoryStream** writer) = 0;
	virtual void Deserialize(CMemoryReader* reader) = 0;
	virtual CDataType GetDataType() = 0;

protected:
	CDataModelBase() = default;
	virtual ~CDataModelBase() = default;
};