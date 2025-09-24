#pragma once

#include "CDataModelBase.h"
#include <vector>

class CIconStatusItem
{
public:
	CIconStatusItem() :
		IconStatus(0)
	{}

	~CIconStatusItem() = default;

	std::wstring ItemName;
	int IconStatus;
};

class CResponseIconStatus : public CDataModelBase
{
public:
	CResponseIconStatus() = default;
	~CResponseIconStatus();

	void Serialize(CMemoryStream** writer);
	void Deserialize(CMemoryReader* reader);
	CDataType GetDataType();

	void AddItem(CIconStatusItem* item);
	void AddItem(const std::wstring& fileName, int status);

	const std::vector<CIconStatusItem*>& GetItems();

private:
	std::vector<CIconStatusItem*> _items;
};

