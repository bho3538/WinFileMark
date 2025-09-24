#include "stdafx.h"
#include "CShellUtils.h"


bool CShellUtils::IsFilesystemFolder(IShellFolder* pSF) {
	if (pSF == nullptr) {
		return false;
	}

	IPersist* pPersist = nullptr;
	HRESULT hookHR = pSF->QueryInterface(IID_IPersist, reinterpret_cast<void**>(&pPersist));
	if (hookHR != S_OK) {
		return false;
	}

	// shell folder 의 clsid 를 비교
	CLSID folderClsid = { 0, };
	hookHR = pPersist->GetClassID(&folderClsid);
	if (hookHR != S_OK) {
		pPersist->Release();
		return false;
	}
	pPersist->Release();

	if (::IsEqualCLSID(folderClsid, CLSID_ShellFSFolder) == FALSE) {
		// filesystemfolder 가 아님 (홈, 갤러리 등)
		return false;
	}

	return true;
}

IShellView* CShellUtils::GetIShellView(IShellFolderViewCB* pVCB) {
	if (pVCB == nullptr) {
		return nullptr;
	}

	IShellView* pSV = nullptr;

	IObjectWithSite* pSite = nullptr;
	HRESULT hr = pVCB->QueryInterface(IID_IObjectWithSite, reinterpret_cast<void**>(&pSite));
	if (hr == S_OK) {
		hr = pSite->GetSite(IID_IShellView, reinterpret_cast<void**>(&pSV));
	}

	pSite->Release();

	return pSV;
}

LPITEMIDLIST CShellUtils::GetCurrentFolderPidl(IUnknown* pUnk) {
	if (pUnk == nullptr) {
		return nullptr;
	}

	LPITEMIDLIST currentFolderPidl = nullptr;

	IPersistFolder2* pf2 = nullptr;
	HRESULT hr = pUnk->QueryInterface(IID_IPersistFolder2, reinterpret_cast<void**>(&pf2));
	if (hr != S_OK) {
		return nullptr;
	}

	hr = pf2->GetCurFolder(&currentFolderPidl);
	if (hr != S_OK) {
		pf2->Release();

		return nullptr;
	}

	pf2->Release();

	return currentFolderPidl;
}

LPWSTR CShellUtils::GetFolderPath(IUnknown* pUnk) {
	if (pUnk == nullptr) {
		return nullptr;
	}

	LPITEMIDLIST currentFolderPidl = CShellUtils::GetCurrentFolderPidl(pUnk);
	if (currentFolderPidl == nullptr) {
		return nullptr;
	}

	LPWSTR path = nullptr;

	IShellItem* pItem = nullptr;
	HRESULT hr = SHCreateItemFromIDList(currentFolderPidl, IID_IShellItem, reinterpret_cast<void**>(&pItem));
	if (hr == S_OK) {

		pItem->GetDisplayName(SIGDN_FILESYSPATH, &path);
		pItem->Release();
	}

	ILFree(currentFolderPidl);

	return path;
}

LPWSTR CShellUtils::GetFolderPathFromView(IShellView* pSV) {
	IFolderView* pFV = nullptr;
	HRESULT hr = pSV->QueryInterface(IID_IFolderView, reinterpret_cast<void**>(&pFV));
	if (hr != S_OK) {
		return nullptr;
	}

	IShellFolder* pSF = nullptr;
	hr = pFV->GetFolder(IID_IShellFolder, reinterpret_cast<void**>(&pSF));
	if (hr != S_OK) {
		pFV->Release();

		return nullptr;
	}

	LPWSTR path = CShellUtils::GetFolderPath(pSF);

	pFV->Release();
	pSF->Release();

	return path;
}

std::wstring CShellUtils::GetItemFilesystemPath(IShellItem* pItem) {
	if (pItem == nullptr) {
		return L"";
	}

	std::wstring path;
	LPWSTR pPath = nullptr;

	pItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

	if (pPath != nullptr) {
		path = pPath;

		CoTaskMemFree(pPath);
	}
	else {
		path = L"";
	}

	return path;
}

std::wstring CShellUtils::GetItemName(IShellItem* pItem) {
	if (pItem == nullptr) {
		return L"";
	}

	std::wstring path;
	LPWSTR pPath = nullptr;

	pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &pPath);

	if (pPath != nullptr) {
		path = pPath;

		CoTaskMemFree(pPath);
	}
	else {
		path = L"";
	}

	return path;
}

std::wstring CShellUtils::GetItemParentPath(IShellItem* pItem) {
	std::wstring path = GetItemFilesystemPath(pItem);

	if (path.empty() == true) {
		return path;
	}

	size_t lastSeperator = path.find_last_of(L'\\');
	if (lastSeperator == std::wstring::npos) {
		return path;
	}

	path = path.substr(0, lastSeperator);

	return path;
}

std::wstring CShellUtils::GetFolderPathFromViewCB(IShellFolderViewCB* pCB, IShellView** ppSV)
{
	if (pCB == nullptr)
	{
		return L"";
	}

	IShellView* pSV = GetIShellView(pCB);
	if (pSV == nullptr)
	{
		return L"";
	}

	LPWSTR folderPath = GetFolderPathFromView(pSV);
	if (folderPath == nullptr)
	{
		pSV->Release();

		return L"";
	}

	if (ppSV != nullptr)
	{
		*ppSV = pSV;
	}
	else
	{
		pSV->Release();
	}

	std::wstring path(folderPath);

	::CoTaskMemFree(folderPath);

	return path;
}