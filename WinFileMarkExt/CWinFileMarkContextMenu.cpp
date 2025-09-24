#include "stdafx.h"
#include "CWinFileMarkContextMenu.h"
#include <ShlGuid.h>
#include <Shlwapi.h>
#include "CWinFileMarkEnumContextMenu.h"

CWinFileMarkContextMenu::CWinFileMarkContextMenu() :
	_refCount(1),
	_punkSite(nullptr)
{
}

CWinFileMarkContextMenu::~CWinFileMarkContextMenu() {
	if (_punkSite != nullptr) {
		_punkSite->Release();
	}
}

// IUnknown
HRESULT CWinFileMarkContextMenu::QueryInterface(REFIID riid, void** ppv) {
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

ULONG CWinFileMarkContextMenu::AddRef() {
	return InterlockedIncrement(&_refCount);
}

ULONG CWinFileMarkContextMenu::Release() {
	ULONG refCount = InterlockedDecrement(&_refCount);
	if (refCount == 0) {
		delete this;
	}

	return refCount;
}

HRESULT CWinFileMarkContextMenu::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) {
	if (!ppszName) return E_POINTER;
	return SHStrDupW(L"Mark at file", ppszName);
}

HRESULT CWinFileMarkContextMenu::GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) {
	return E_NOTIMPL;
}

HRESULT CWinFileMarkContextMenu::GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) {
	return E_NOTIMPL;
}

HRESULT CWinFileMarkContextMenu::GetCanonicalName(GUID* pguidCommandName) {
	if (!pguidCommandName) return E_POINTER;
	*pguidCommandName = GUID_NULL;
	return S_OK;
}

HRESULT CWinFileMarkContextMenu::GetState(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState) {
	if (!pCmdState) return E_POINTER;
	*pCmdState = ECS_ENABLED;
	return S_OK;
}

HRESULT CWinFileMarkContextMenu::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) {


	return S_OK;
}

HRESULT CWinFileMarkContextMenu::GetFlags(EXPCMDFLAGS* pFlags) {
	*pFlags = ECF_DEFAULT | ECF_HASSUBCOMMANDS;
	return S_OK;
}

HRESULT CWinFileMarkContextMenu::EnumSubCommands(IEnumExplorerCommand** ppEnum) {

	CWinFileMarkEnumContextMenu* enumMenu = new CWinFileMarkEnumContextMenu(GetCurrentFolderPath());

	HRESULT hr = enumMenu->QueryInterface(IID_IEnumExplorerCommand, reinterpret_cast<void**>(ppEnum));
	if (hr != S_OK) {
		*ppEnum = nullptr;
	}
	enumMenu->Release();

	return hr;
}

std::wstring CWinFileMarkContextMenu::GetCurrentFolderPath() {
	IShellBrowser* pBrowser = nullptr;
	HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_IShellBrowser, reinterpret_cast<void**>(&pBrowser));
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

	LPWSTR pPath = CShellUtils::GetFolderPath(pf);

	pf->Release();
	pFV->Release();
	pSV->Release();
	pBrowser->Release();

	std::wstring path;
	if (pPath != nullptr) {
		path = pPath;
		CoTaskMemFree(pPath);
	}
	else {
		path = L"";
	}

	return path;
}