#pragma once
class CColumn
{
public:
	CColumn();
	~CColumn();

	HRESULT Initialize(IShellView* shview);

	HRESULT ShowWinFileMarkStatusColumn();

private:

	IColumnManager* _pColumnManager = nullptr;
};

