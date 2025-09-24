#include "stdafx.h"
#include "CNotificationEditContextMenu.h"
#include <Shlwapi.h>
#include <ShlGuid.h>

CNotificationEditContextMenu::CNotificationEditContextMenu() :
	_refCount(1),
	_punkSite(nullptr)
{
}

CNotificationEditContextMenu::~CNotificationEditContextMenu() {
	if (_punkSite != nullptr) {
		_punkSite->Release();
	}
}

// IUnknown
HRESULT CNotificationEditContextMenu::QueryInterface(REFIID riid, void** ppv) {
	if (ppv == nullptr) {
		return E_POINTER;
	}

	*ppv = nullptr;

	if (riid == IID_IUnknown) {
		*ppv = static_cast<IExplorerCommand*>(this);
	}
	if (riid == IID_IObjectWithSite) {
		*ppv = static_cast<IObjectWithSite*>(this);
	}
	else if (riid == IID_IExplorerCommand) {
		*ppv = static_cast<IExplorerCommand*>(this);
	}

	if (*ppv != nullptr) {
		this->AddRef();
		return S_OK;
	}
	else {
		return E_NOINTERFACE;
	}
}

ULONG CNotificationEditContextMenu::AddRef() {
	return InterlockedIncrement(&_refCount);
}

ULONG CNotificationEditContextMenu::Release() {
	ULONG refCount = InterlockedDecrement(&_refCount);
	if (refCount == 0) {
		delete this;
	}

	return refCount;
}

HRESULT CNotificationEditContextMenu::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) {
	if (!ppszName) return E_POINTER;
	return SHStrDupW(L"Edit folder message", ppszName);
}

HRESULT CNotificationEditContextMenu::GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) {
	return E_NOTIMPL;
}

HRESULT CNotificationEditContextMenu::GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) {
	return E_NOTIMPL;
}

HRESULT CNotificationEditContextMenu::GetCanonicalName(GUID* pguidCommandName) {
	if (!pguidCommandName) return E_POINTER;
	*pguidCommandName = GUID_NULL;
	return S_OK;
}

HRESULT CNotificationEditContextMenu::GetState(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState) {
	if (!pCmdState) return E_POINTER;
	*pCmdState = ECS_ENABLED;
	return S_OK;
}

HRESULT CNotificationEditContextMenu::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) {

	std::wstring path = GetCurrentFolderPath();

	return S_OK;
}

HRESULT CNotificationEditContextMenu::GetFlags(EXPCMDFLAGS* pFlags) {
	*pFlags = ECF_DEFAULT;
	return S_OK;
}

HRESULT CNotificationEditContextMenu::EnumSubCommands(IEnumExplorerCommand** ppEnum) {
	return E_NOTIMPL;
}

std::wstring CNotificationEditContextMenu::GetCurrentFolderPath() {
	IShellBrowser* pBrowser = nullptr;
	HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_IShellBrowser, reinterpret_cast<void**>(& pBrowser));
	if (hr != S_OK) {
		return L"";
	}

	IShellView* pSV = nullptr;
	hr = pBrowser->QueryActiveShellView(&pSV);
	if (hr != S_OK) {
		pBrowser->Release();

		return L"";
	}

	IFolderView* pFV = nullptr;
	hr = pSV->QueryInterface(IID_IFolderView, reinterpret_cast<void**>(&pFV));
	if (hr != S_OK) {
		pSV->Release();
		pBrowser->Release();

		return L"";
	}

	IShellFolder* pf = nullptr;
	hr = pFV->GetFolder(IID_IShellFolder, reinterpret_cast<void**>(&pf));
	if (hr != S_OK) {
		pFV->Release();
		pSV->Release();
		pBrowser->Release();

		return L"";
	}

	std::wstring path;
	LPWSTR pPath = CShellUtils::GetFolderPath(pf);

	if (pPath != nullptr) {
		path = pPath;
		CoTaskMemFree(pPath);
	}
	else {
		path = L"";
	}

	pf->Release();
	pFV->Release();
	pSV->Release();
	pBrowser->Release();

	return path;
}