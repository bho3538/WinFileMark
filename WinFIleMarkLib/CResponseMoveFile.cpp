#include "stdafx.h"
#include "CResponseMoveFile.h"

void CResponseMoveFile::Serialize(CMemoryStream** writer)
{
	*writer = new CMemoryStream();
	if ((*writer)->Initialize(64) == FALSE)
	{
		return;
	}

	(*writer)->WriteDWORD(static_cast<DWORD>(GetDataType()));
	(*writer)->WriteDWORD(IconStatus);
}

void CResponseMoveFile::Deserialize(CMemoryReader* reader)
{
	if (reader == nullptr)
	{
		return;
	}

	if (static_cast<DWORD>(GetDataType()) != reader->ReadDWORD())
	{
		return;
	}

	IconStatus = static_cast<int>(reader->ReadDWORD());
}

CDataType CResponseMoveFile::GetDataType()
{
	return CDataType::REPLY_MOVEFILE;
}