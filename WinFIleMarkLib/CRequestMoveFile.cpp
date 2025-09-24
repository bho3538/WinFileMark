#include "stdafx.h"
#include "CRequestMoveFile.h"


void CRequestMoveFile::Serialize(CMemoryStream** writer)
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
	(*writer)->WriteWSTR(OldPath.c_str());
	(*writer)->WriteWSTR(OldFileName.c_str());
	(*writer)->WriteWSTR(NewPath.c_str());
	(*writer)->WriteWSTR(NewFileName.c_str());
}

void CRequestMoveFile::Deserialize(CMemoryReader* reader)
{
	if (reader == nullptr)
	{
		return;
	}

	if (static_cast<DWORD>(GetDataType()) != reader->ReadDWORD())
	{
		return;
	}

	OldPath = reader->ReadWSTR();
	OldFileName = reader->ReadWSTR();
	NewPath = reader->ReadWSTR();
	NewFileName = reader->ReadWSTR();
}

CDataType CRequestMoveFile::GetDataType()
{
	return CDataType::REQUEST_MOVEFILE;
}