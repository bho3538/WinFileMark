#include "stdafx.h"
#include "CRequestRenameFile.h"

void CRequestRenameFile::Serialize(CMemoryStream** writer)
{
	if (writer == nullptr)
	{
		return;
	}

	*writer = new CMemoryStream();
	if ((*writer)->Initialize(1024) == FALSE)
	{
		return;
	}

	(*writer)->WriteDWORD(static_cast<DWORD>(GetDataType()));
	(*writer)->WriteWSTR(Path.c_str());
	(*writer)->WriteWSTR(OldFileName.c_str());
	(*writer)->WriteWSTR(NewFileName.c_str());
}

void CRequestRenameFile::Deserialize(CMemoryReader* reader)
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
	OldFileName = reader->ReadWSTR();
	NewFileName = reader->ReadWSTR();
}

CDataType CRequestRenameFile::GetDataType()
{
	return CDataType::REQUEST_RENAMEFILE;
}