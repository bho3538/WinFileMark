#pragma once

#include <ShlObj_core.h>

class CShellUtils
{
public:
	static bool IsFilesystemFolder(IShellFolder* pSF);
	static IShellView* GetIShellView(IShellFolderViewCB* pVCB);
	static LPWSTR GetFolderPath(IUnknown* pUnk);
	static LPWSTR GetFolderPathFromView(IShellView* pSV);
	static std::wstring GetFolderPathFromViewCB(IShellFolderViewCB* pCB, IShellView** ppSV);

	static LPITEMIDLIST GetCurrentFolderPidl(IUnknown* pUnk);

	static std::wstring GetItemFilesystemPath(IShellItem* pItem);
	static std::wstring GetItemName(IShellItem* pItem);
	static std::wstring GetItemParentPath(IShellItem* pItem);
};

