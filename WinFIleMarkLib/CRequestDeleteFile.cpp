#include "stdafx.h"
#include "CRequestDeleteFile.h"

void CRequestDeleteFile::Serialize(CMemoryStream** writer)
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
	(*writer)->WriteWSTR(FileName.c_str());
}

void CRequestDeleteFile::Deserialize(CMemoryReader* reader)
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
	FileName = reader->ReadWSTR();
}

CDataType CRequestDeleteFile::GetDataType()
{
	return CDataType::REQUEST_DELETEFILE;
}