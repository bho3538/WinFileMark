#include "stdafx.h"
#include "CRequestIconStatus.h"

void CRequestIconStatus::Serialize(CMemoryStream** writer)
{
	if (writer == nullptr)
	{
		return;
	}

	*writer = new CMemoryStream();
	if ((*writer)->Initialize(512) == FALSE)
	{
		return;
	}

	(*writer)->WriteDWORD(static_cast<DWORD>(GetDataType()));
	(*writer)->WriteWSTR(Path.c_str());
	(*writer)->WriteDWORD(Offset);
	(*writer)->WriteDWORD(Length);
}

void CRequestIconStatus::Deserialize(CMemoryReader* reader)
{
	if (reader == nullptr)
	{
		return;
	}

	if (static_cast<DWORD>(GetDataType()) != reader->ReadDWORD())
	{
		return;
	}

	Path = reader->ReadWSTR();
	Offset = reader->ReadDWORD();
	Length = reader->ReadDWORD();
}

CDataType CRequestIconStatus::GetDataType()
{
	return CDataType::REQUEST_ICONSTATUS;
}