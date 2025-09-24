#include "stdafx.h"
#include "CResponseIconStatus.h"


CResponseIconStatus::~CResponseIconStatus()
{
	size_t itemCnt = _items.size();

	for (size_t i = 0; i < itemCnt; i++)
	{
		delete _items[i];
	}

	_items.clear();
}

void CResponseIconStatus::Serialize(CMemoryStream** writer)
{
	if (writer == nullptr)
	{
		return;
	}

	*writer = new CMemoryStream();
	if ((*writer)->Initialize(4096) == FALSE)
	{
		return;
	}

	(*writer)->WriteDWORD(static_cast<DWORD>(GetDataType()));

	DWORD itemCount = (DWORD)_items.size();

	(*writer)->WriteDWORD(itemCount);
	for (DWORD i = 0; i < itemCount; i++)
	{
		(*writer)->WriteWSTR(_items[i]->ItemName.c_str());
		(*writer)->WriteDWORD(_items[i]->IconStatus);
	}
}

void CResponseIconStatus::Deserialize(CMemoryReader* reader)
{
	if (reader == nullptr)
	{
		return;
	}

	if (static_cast<DWORD>(GetDataType()) != reader->ReadDWORD())
	{
		return;
	}

	DWORD itemCount = reader->ReadDWORD();

	for (DWORD i = 0; i < itemCount; i++)
	{
		CIconStatusItem* item = new CIconStatusItem();
		item->ItemName = reader->ReadWSTR();
		item->IconStatus = (int)reader->ReadDWORD();

		_items.push_back(item);
	}
}

// db 에서 받아온걸로
void CResponseIconStatus::AddItem(CIconStatusItem* item)
{
	if (item == nullptr)
	{
		return;
	}

	if (_items.size() >= MAXINT32)
	{
		return;
	}

	_items.push_back(item);
}

void CResponseIconStatus::AddItem(const std::wstring& fileName, int status)
{
	if (fileName.empty() == true)
	{
		return;
	}

	CIconStatusItem* item = new CIconStatusItem();

	item->ItemName = fileName;
	item->IconStatus = status;

	AddItem(item);
}

const std::vector<CIconStatusItem*>& CResponseIconStatus::GetItems()
{
	return _items;
}

CDataType CResponseIconStatus::GetDataType()
{
	return CDataType::REPLY_ICONSTATUS;
}