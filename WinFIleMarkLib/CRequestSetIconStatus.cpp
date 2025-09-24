#include "stdafx.h"
#include "CRequestSetIconStatus.h"

CRequestSetIconStatus::CRequestSetIconStatus() :
	StatusIconFlags(0),
	UnsetMode(false)
{}

CRequestSetIconStatus::CRequestSetIconStatus(const std::wstring& path) :
	StatusIconFlags(0),
	UnsetMode(false),
	Path(path)
{}

CRequestSetIconStatus::~CRequestSetIconStatus()
{}

void CRequestSetIconStatus::Serialize(CMemoryStream** writer)
{
	if (writer == nullptr)
	{
		return;
	}

	*writer = new CMemoryStream();
	if ((*writer)->Initialize(2048) == FALSE)
	{
		return;
	}

	(*writer)->WriteDWORD(static_cast<DWORD>(GetDataType()));
	(*writer)->WriteWSTR(Path.c_str());
	(*writer)->WriteDWORD(UnsetMode == true ? 1 : 0);
	(*writer)->WriteDWORD(StatusIconFlags);

	DWORD fileCount = (DWORD)_fileNames.size();
	(*writer)->WriteDWORD(fileCount);

	for (DWORD i = 0; i < fileCount; i++)
	{
		(*writer)->WriteWSTR(_fileNames[i].c_str());
	}
}

void CRequestSetIconStatus::Deserialize(CMemoryReader* reader)
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
	UnsetMode = reader->ReadDWORD() == 0 ? false : true;
	StatusIconFlags = (int)reader->ReadDWORD();

	DWORD fileCount = reader->ReadDWORD();

	for (int i = 0; i < fileCount; i++)
	{
		_fileNames.push_back(reader->ReadWSTR());
	}
}

CDataType CRequestSetIconStatus::GetDataType()
{
	return CDataType::REQUEST_SETICONSTATUS;
}

void CRequestSetIconStatus::AddItem(const std::wstring& fileName)
{
	if (fileName.empty() == false)
	{
		_fileNames.push_back(fileName);
	}
}

const std::vector<std::wstring>& CRequestSetIconStatus::GetItems()
{
	return _fileNames;
}