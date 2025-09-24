#include "stdafx.h"
#include "CHookShellFolder.h"
#include "CInfoBar.h"
#include "CInfoBarManager.h"
#include "CColumn.h"
#include "DBCache.h"
#include "IconStatusCacheManager.h"

#include "../extern_lib/Detours/include/detours.h"

#include <string>
#include <propkey.h>

#define SFVM_WINDOWCLOSED 16
#define SFVM_REFRESHED 17


// {1E8F703C-82B1-4FF3-801A-7FB38E8CB235, 100}
//const PROPERTYKEY PKEY_ItemWinFileMarkStatus = { {0x1e8f703c, 0x82b1, 0x4ff3, 0x80, 0x1a, 0x7f, 0xb3, 0x8e, 0x8c, 0xb2, 0x35 }, 100 };
const PROPERTYKEY PKEY_ItemWinFileMarkStatus = { {0x41376319, 0xd83f, 0x4c78, 0x96, 0x73, 0xf0, 0xb9, 0x6, 0x17, 0x45 }, 1 };


// {41376319-D83F-4C78-9673-F0B906174500,1}
const PROPERTYKEY PKEY_ItemWinFileMarkIcon1 = { {0x41376319, 0xd83f, 0x4c78, 0x96, 0x73, 0xf0, 0xb9, 0x6, 0x17, 0x45 }, 2 };
const PROPERTYKEY PKEY_ItemWinFileMarkIcon2 = { {0x41376319, 0xd83f, 0x4c78, 0x96, 0x73, 0xf0, 0xb9, 0x6, 0x17, 0x45 }, 3 };
const PROPERTYKEY PKEY_ItemWinFileMarkIcon3 = { {0x41376319, 0xd83f, 0x4c78, 0x96, 0x73, 0xf0, 0xb9, 0x6, 0x17, 0x45 }, 4 };

// {c9944a21-a406-48fe-8225-aec7e24c211b}
const PROPERTYKEY PKEY_SystemStatusIcon = { { 0xc9944a21, 0xa406, 0x48fe, 0x82, 0x25, 0xae, 0xc7, 0xe2, 0x4c, 0x21, 0x1b }, 16 };
const PROPERTYKEY PKEY_SystemIconFlag = { { 0xC9944A21, 0xA406, 0x48FE, 0x82, 0x25, 0xAE, 0xC7, 0xE2, 0x4C, 0x21, 0x1B }, 18 };

void* _GetRawMethodFromInterface(PVOID interf, DWORD index) {
	return *(PVOID*)(*(DWORD_PTR*)interf + index * sizeof(DWORD_PTR));
}

typedef HRESULT(WINAPI* TSHCreateShellFolderView)(const SFV_CREATE* pcsfv, IShellView** ppsv);
TSHCreateShellFolderView Original_SHCreateShellFolderView = SHCreateShellFolderView;
HRESULT WINAPI Hooked_SHCreateShellFolderView(const SFV_CREATE* pcsfv, IShellView** ppsv);

typedef HRESULT(WINAPI* TMessageSFVCB)(IShellFolderViewCB* svCB, UINT uMsg, WPARAM wParam, LPARAM lParam);
TMessageSFVCB Original_MessageSFVCB = nullptr;
HRESULT WINAPI Hooked_MessageSFVCB(IShellFolderViewCB* svCB, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(WINAPI* TGetValue)(IPropertyStore* ps, REFPROPERTYKEY key, PROPVARIANT* pv);
TGetValue Original_GetValue = nullptr;
HRESULT WINAPI Hooked_GetValue(IPropertyStore* ps, REFPROPERTYKEY key, PROPVARIANT* pv);

typedef HRESULT(WINAPI* TRenameItem)(ITransferSource* pTS, IShellItem* psiSource, LPCWSTR pszNewName, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);
TRenameItem Original_RenameItem = nullptr;
HRESULT WINAPI Hooked_RenameItem(ITransferSource* pTS, IShellItem* psiSource, LPCWSTR pszNewName, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);

typedef HRESULT(WINAPI* TRemoveItem)(ITransferSource* pTS, IShellItem* psiSource, TRANSFER_SOURCE_FLAGS flags);
TRemoveItem Original_RemoveItem = nullptr;
HRESULT WINAPI Hooked_RemoveItem(ITransferSource* pTS, IShellItem* psiSource, TRANSFER_SOURCE_FLAGS flags);

typedef HRESULT(WINAPI* TRecycleItem)(ITransferSource* pTS, IShellItem* psiSource, IShellItem* psiParentDest, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);
TRecycleItem Original_RecycleItem = nullptr;
HRESULT WINAPI Hooked_RecycleItem(ITransferSource* pTS, IShellItem* psiSource, IShellItem* psiParentDest, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);

typedef HRESULT(WINAPI* TMoveItem)(ITransferSource* pTS, IShellItem* psiSource, IShellItem* psiParentDest, LPCWSTR pszNameDst, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);
TMoveItem Original_MoveItem = nullptr;
HRESULT WINAPI Hooked_MoveItem(ITransferSource* pTS, IShellItem* psiSource, IShellItem* psiParentDest, LPCWSTR pszNameDst, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);

CInfoBarManager g_InfobarManager;

IconStatusCacheManager g_IconStatusCache;

void CHookShellFolder::Hook() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&Original_SHCreateShellFolderView, Hooked_SHCreateShellFolderView);
	DetourTransactionCommit();
}

void CHookShellFolder::Unhook() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&Original_SHCreateShellFolderView, Hooked_SHCreateShellFolderView);
	DetourTransactionCommit();
}


HRESULT WINAPI Hooked_SHCreateShellFolderView(const SFV_CREATE* pcsfv, IShellView** ppsv) {

	// 원본 함수 호출
	HRESULT hr = Original_SHCreateShellFolderView(pcsfv, ppsv);
	if (hr != S_OK) {
		return hr;
	}

	// 원본 함수 호출이 성공하였고 ppsv 값이 유효한 경우 내부 interface 후킹 진행 (최초 1회)
	if (Original_MessageSFVCB == nullptr && CShellUtils::IsFilesystemFolder(pcsfv->pshf) == true) {
		HRESULT hookHr = S_OK;

		if (pcsfv->psfvcb != nullptr) {
			// IUnknown + 첫번째 메서드를 취득
			Original_MessageSFVCB = (TMessageSFVCB)_GetRawMethodFromInterface(pcsfv->psfvcb, 3);

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&Original_MessageSFVCB, Hooked_MessageSFVCB);
			DetourTransactionCommit();
		}

		// 현재 폴더의 PIDL을 조회 
		LPITEMIDLIST currentFolderPidl = CShellUtils::GetCurrentFolderPidl(pcsfv->pshf);
		if (currentFolderPidl != nullptr) {
			IPropertyStore* ps = nullptr;
			 hookHr = SHGetPropertyStoreFromIDList(currentFolderPidl, GPS_DEFAULT, IID_IPropertyStore, reinterpret_cast<void**>(&ps));

			if (hookHr == S_OK) {

				Original_GetValue = (TGetValue)_GetRawMethodFromInterface(ps, 5);

				// 커스텀 컬럼을 표시하기 위해 hook
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourAttach(&Original_GetValue, Hooked_GetValue);
				DetourTransactionCommit();

				ps->Release();
			}

			ILFree(currentFolderPidl);
		}

		// 쉘 파일 operation 을 탐지하기 위한 hook

		ITransferSource* pTS = nullptr;
		hookHr = pcsfv->pshf->CreateViewObject(nullptr, IID_ITransferSource, reinterpret_cast<void**>(&pTS));

		if (hookHr == S_OK) {
			Original_RenameItem = (TRenameItem)_GetRawMethodFromInterface(pTS, 10);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&Original_RenameItem, Hooked_RenameItem);
			DetourTransactionCommit();

			Original_RemoveItem = (TRemoveItem)_GetRawMethodFromInterface(pTS, 9);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&Original_RemoveItem, Hooked_RemoveItem);
			DetourTransactionCommit();

			Original_RecycleItem = (TRecycleItem)_GetRawMethodFromInterface(pTS, 8);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&Original_RecycleItem, Hooked_RecycleItem);
			DetourTransactionCommit();

			Original_MoveItem = (TMoveItem)_GetRawMethodFromInterface(pTS, 7);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&Original_MoveItem, Hooked_MoveItem);
			DetourTransactionCommit();

			pTS->Release();
		}

	}

	return hr;
}

HRESULT WINAPI Hooked_MessageSFVCB(IShellFolderViewCB* svCB, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	HRESULT originalHr = Original_MessageSFVCB(svCB, uMsg, wParam, lParam);

	switch (uMsg) 
	{
		case SFVM_WINDOWCREATED :
		{
			IShellView* pSV = nullptr;
			std::wstring folderPath = CShellUtils::GetFolderPathFromViewCB(svCB, &pSV);
			if (folderPath.empty() == false)
			{
				// 컬럼 관리 (커스텀 컬럼)
				CColumn column;
				HRESULT hr = column.Initialize(pSV);

				if (hr == S_OK)
				{
					hr = column.ShowWinFileMarkStatusColumn();
				}

				g_IconStatusCache.CreateFolderCache(folderPath);
			}

			if (pSV != nullptr)
			{
				pSV->Release();
			}

			break;
		}; 
		case SFVM_WINDOWCLOSED : 
		{
			g_InfobarManager.CloseInfoBar(reinterpret_cast<HWND>(wParam));

			std::wstring folderPath = CShellUtils::GetFolderPathFromViewCB(svCB, nullptr);
			if (folderPath.empty() == false)
			{
				g_IconStatusCache.ReleaseFolderCache(folderPath);
			}

			break;
		}; 
		case SFVM_REFRESHED : 
		{
			if (wParam & 1)
			{
				// 사용자에 의한 refresh (F5 등)
				// 캐시 무효화 후 갱신

				std::wstring folderPath = CShellUtils::GetFolderPathFromViewCB(svCB, nullptr);
				if (folderPath.empty() == false)
				{
					g_IconStatusCache.ReleaseFolderCache(folderPath);
					g_IconStatusCache.CreateFolderCache(folderPath);
				}
			}

			break;
		}; 
	}

	return originalHr;
}

HRESULT WINAPI Hooked_GetValue(IPropertyStore* ps, REFPROPERTYKEY key, PROPVARIANT* pv) {
	if (key == PKEY_ItemWinFileMarkStatus) {
		// 파일의 경로 취득
		PROPVARIANT pathInfo = { 0, };
		HRESULT hr = Original_GetValue(ps, PKEY_ParsingPath, &pathInfo);

		if (hr != S_OK || pathInfo.vt == VT_EMPTY) {
			return E_FAIL;
		}

		// 순수 파일 이름 취득
		LPWSTR filename = PathFindFileNameW(pathInfo.pwszVal);

		// 부모 경로 취득
		PathRemoveFileSpecW(pathInfo.pwszVal);

		IconStatusCache* cache = g_IconStatusCache.GetFolderCache(pathInfo.pwszVal);
		if (cache != NULL)
		{
			int status = cache->GetFileMarkStatus(filename);

			hr = CHookShellFolder::SetIconColumnInfo(pv, status);
		}
		else
		{
			hr = E_NOTIMPL;
		}


		PropVariantClear(&pathInfo);

		return hr;
	}
	else {
		HRESULT hr = Original_GetValue(ps, key, pv);
		return hr;
	}
}

HRESULT WINAPI Hooked_RenameItem(ITransferSource* pTS, IShellItem* psiSource, LPCWSTR pszNewName, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest) {
	HRESULT hr = Original_RenameItem(pTS, psiSource, pszNewName, flags, ppsiNewDest);

	if (*ppsiNewDest == nullptr) {
		return hr;
	}

	// 이전 파일의 이름
	std::wstring originalName = CShellUtils::GetItemName(psiSource);
	if (originalName.empty() == true) {
		return hr;
	}

	// 변경된 파일의 이름
	std::wstring newName = CShellUtils::GetItemName(*ppsiNewDest);
	if (newName.empty() == true) {
		return hr;
	}

	// 이름이 변경된 경우
	if (originalName != newName) {
		std::wstring folderPath = CShellUtils::GetItemParentPath(psiSource);
		if (folderPath.empty() == true) {
			return hr;
		}

		// 갱신
		IconStatusCache* cache = g_IconStatusCache.GetFolderCache(folderPath);
		if (cache != nullptr)
		{
			cache->RenameFileInfo(originalName, newName);
		}
	}

	return hr;
}

HRESULT WINAPI Hooked_RemoveItem(ITransferSource* pTS, IShellItem* psiSource, TRANSFER_SOURCE_FLAGS flags) {
	HRESULT hr = Original_RemoveItem(pTS, psiSource, flags);

	std::wstring itemPath = CShellUtils::GetItemFilesystemPath(psiSource);
	if (itemPath.empty() == false) {
		if (GetFileAttributesW(itemPath.c_str()) == INVALID_FILE_ATTRIBUTES) {

			std::wstring itemName = CShellUtils::GetItemName(psiSource);
			if (itemName.empty() == true) {
				return hr;
			}

			std::wstring folderPath = CShellUtils::GetItemParentPath(psiSource);
			if (folderPath.empty() == true) {
				return hr;
			}

			IconStatusCache* cache = g_IconStatusCache.GetFolderCache(folderPath);
			if (cache != nullptr)
			{
				cache->DeleteFileInfo(itemName);
			}
		}
	}

	return hr;
}

HRESULT WINAPI Hooked_RecycleItem(ITransferSource* pTS, IShellItem* psiSource, IShellItem* psiParentDest, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest) {
	HRESULT hr = Original_RecycleItem(pTS, psiSource, psiParentDest, flags, ppsiNewDest);

	std::wstring itemPath = CShellUtils::GetItemFilesystemPath(psiSource);
	if (itemPath.empty() == false) {
		// 파일이 성공적으로 삭제된 경우 (찾을 수 없는 경우)
		if (GetFileAttributesW(itemPath.c_str()) == INVALID_FILE_ATTRIBUTES) {

			std::wstring itemName = CShellUtils::GetItemName(psiSource);
			if (itemName.empty() == true) {
				return hr;
			}

			std::wstring folderPath = CShellUtils::GetItemParentPath(psiSource);
			if (folderPath.empty() == true) {
				return hr;
			}

			IconStatusCache* cache = g_IconStatusCache.GetFolderCache(folderPath);
			if (cache != nullptr)
			{
				cache->DeleteFileInfo(itemName);
			}
		}
	}

	return hr;
}

HRESULT WINAPI Hooked_MoveItem(ITransferSource* pTS, IShellItem* psiSource, IShellItem* psiParentDest, LPCWSTR pszNameDst, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest) {
	HRESULT hr = Original_MoveItem(pTS, psiSource, psiParentDest, pszNameDst, flags, ppsiNewDest);

	if (*ppsiNewDest == nullptr)
	{
		return hr;
	}

	std::wstring originalItemName = CShellUtils::GetItemName(psiSource);
	if (originalItemName.empty() == true)
	{
		return hr;
	}

	std::wstring originalFolderPath = CShellUtils::GetItemParentPath(psiSource);
	if (originalFolderPath.empty() == true)
	{
		return hr;
	}

	std::wstring newItemName = CShellUtils::GetItemName(*ppsiNewDest);
	if (newItemName.empty() == true)
	{
		return hr;
	}

	std::wstring newFolderPath = CShellUtils::GetItemParentPath(*ppsiNewDest);
	if (newFolderPath.empty() == true)
	{
		return hr;
	}

	if (newFolderPath == originalFolderPath)
	{
		// rename
		IconStatusCache* cache = g_IconStatusCache.GetFolderCache(newFolderPath);
		if (cache != nullptr)
		{
			cache->RenameFileInfo(originalItemName, newItemName);
		}
	}
	else
	{
		// move
		g_IconStatusCache.MoveItemCache(originalFolderPath, originalItemName, newFolderPath, newItemName);
	}

	return hr;
}

HRESULT CHookShellFolder::SetIconColumnInfo(PROPVARIANT* pv, int flags) {
	PROPVARIANT tmp;
	IPropertyStore* ppropstore = NULL;
	IPersistSerializedPropStorage* psps = NULL;
	DWORD cbSize = 0;
	SERIALIZEDPROPSTORAGE* psp = NULL;

	LPCWSTR prop = L"prop:WinFileMark.Status;WinFileMark.Status2;WinFileMark.Status3";
	size_t propLen = wcslen(prop);

	HRESULT hr = PSCreateMemoryPropertyStore(IID_IPropertyStore, reinterpret_cast<void**>(&ppropstore));
	if (FAILED(hr)) {
		goto cleanArea;
	}

	tmp.vt = VT_LPWSTR;
	tmp.pwszVal = reinterpret_cast<LPWSTR>(CoTaskMemAlloc((propLen + 1) * sizeof(WCHAR)));
	if (tmp.pwszVal == nullptr) {
		hr = E_FAIL;
		goto cleanArea;
	}

	wcscpy_s(tmp.pwszVal, propLen + 1, prop);

	ppropstore->SetValue(PKEY_SystemStatusIcon, tmp);

	tmp.vt = VT_UI4;
	tmp.uintVal = 2;
	ppropstore->SetValue(PKEY_SystemIconFlag, tmp);

	if (flags & WINFILEMARK_REDICON) {
		tmp.vt = VT_UI4;
		tmp.uintVal = 1;
		ppropstore->SetValue(PKEY_ItemWinFileMarkIcon1, tmp);
	}
	else {
		tmp.vt = VT_UI4;
		tmp.uintVal = 0;
		ppropstore->SetValue(PKEY_ItemWinFileMarkIcon1, tmp);
	}

	if (flags & WINFILEMARK_GREENICON) {
		tmp.vt = VT_UI4;
		tmp.uintVal = 1;
		ppropstore->SetValue(PKEY_ItemWinFileMarkIcon2, tmp);
	}
	else {
		tmp.vt = VT_UI4;
		tmp.uintVal = 0;
		ppropstore->SetValue(PKEY_ItemWinFileMarkIcon2, tmp);
	}

	if (flags & WINFILEMARK_BLUEICON) {
		tmp.vt = VT_UI4;
		tmp.uintVal = 1;
		ppropstore->SetValue(PKEY_ItemWinFileMarkIcon3, tmp);
	}
	else {
		tmp.vt = VT_UI4;
		tmp.uintVal = 0;
		ppropstore->SetValue(PKEY_ItemWinFileMarkIcon3, tmp);
	}

	hr = ppropstore->QueryInterface(IID_IPersistSerializedPropStorage, reinterpret_cast<void**>(&psps));
	if (FAILED(hr)) {
		goto cleanArea;
	}

	hr = psps->GetPropertyStorage(&psp, &cbSize);
	if (FAILED(hr)) {
		goto cleanArea;
	}

	pv->vt = VT_BLOB;
	pv->blob.pBlobData = reinterpret_cast<BYTE*>(psp);
	pv->blob.cbSize = cbSize;

cleanArea:
	if (psps) {
		psps->Release();
	}
	if (ppropstore) {
		ppropstore->Release();
	}

	return S_OK;
}