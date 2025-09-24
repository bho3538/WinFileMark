#pragma once

#include "stdafx.h"

class CNotificationEditContextMenu :
	public IExplorerCommand,
	public IObjectWithSite
{
public:
	CNotificationEditContextMenu();
	~CNotificationEditContextMenu();

	// IUnknown
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	// IExplorerCommand
	HRESULT __stdcall GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName);
	HRESULT __stdcall GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon);
	HRESULT __stdcall GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip);
	HRESULT __stdcall GetCanonicalName(GUID* pguidCommandName);
	HRESULT __stdcall GetState(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState);
	HRESULT __stdcall Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc);
	HRESULT __stdcall GetFlags(EXPCMDFLAGS* pFlags);
	HRESULT __stdcall EnumSubCommands(IEnumExplorerCommand** ppEnum);

	// IObjectWithSite
	IFACEMETHODIMP SetSite(IUnknown* punkSite)
	{
		if (_punkSite != nullptr) {
			_punkSite->Release();
		}

		_punkSite = punkSite;

		if (_punkSite != nullptr) {
			_punkSite->AddRef();
		}

		return S_OK;
	}

	IFACEMETHODIMP GetSite(REFIID riid, void** ppv)
	{
		*ppv = nullptr;
		if (_punkSite == nullptr) {
			return E_FAIL;
		}

		return _punkSite->QueryInterface(riid, ppv);
	}

private:

	std::wstring GetCurrentFolderPath();

	LONG _refCount;
	IUnknown* _punkSite;
};

