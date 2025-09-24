#pragma once

#include <map>
#include <ShObjIdl_core.h>

class CInfoBarManager
{
public:
	void ShowInfoBar(HWND hwnd, IShellView* pSV, LPCWSTR folderPath);
	void CloseInfoBar(HWND hwnd);

private:
	void InsertCurrentViewInfoBar(HWND hwnd, CInfoBar* pInfobar);
	CInfoBar* GetCurrentViewInfoBar(HWND hwnd);
	void DeleteInfoBar(HWND hwnd);

	std::map<HWND, CInfoBar*> _infobars;
};

