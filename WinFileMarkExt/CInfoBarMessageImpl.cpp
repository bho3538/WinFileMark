#include "stdafx.h"
#include "CInfoBarMessageImpl.h"

#include <shellapi.h>

#define INFOBARMENU_OPENURL 1

HRESULT CInfoBarMessageImpl::QueryInterface(REFIID riid, void** ppv) {
	if (ppv == nullptr) {
		return E_POINTER;
	}

	*ppv = nullptr;

	if (riid == IID_IUnknown) {
		*ppv = static_cast<IUnknown*>(this);
	}
	else if (riid == IID_InfoBarMessage) {
		*ppv = static_cast<IInfoBarMessage*>(this);
	}

	if (*ppv != nullptr) {
		this->AddRef();
		return S_OK;
	}
	else {
		return E_NOINTERFACE;
	}
}

ULONG CInfoBarMessageImpl::AddRef() {
	return InterlockedIncrement(&_refCount);
}

ULONG CInfoBarMessageImpl::Release() {
	ULONG refCount = InterlockedDecrement(&_refCount);
	if (refCount == 0) {
		delete this;
	}

	return refCount;
}

HRESULT STDMETHODCALLTYPE CInfoBarMessageImpl::GetMessageID(GUID* guid, INT* intVal) {
	//Our InformationBar GUID
	*guid = this->guid;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CInfoBarMessageImpl::GetMessageW(LPWSTR* message) {
	if (message == nullptr || _message.empty()) {
		return E_FAIL;
	}

	*message = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(sizeof(WCHAR) * (_message.length() + 1)));
	if (*message == nullptr) {
		return E_FAIL;
	}

	wcscpy_s(*message, _message.length() + 1, _message.c_str());

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CInfoBarMessageImpl::CreateMenu(HMENU* pMenu) {
	if (_url.empty() == true) {
		return E_NOTIMPL;
	}

	if (pMenu == nullptr) {
		return E_INVALIDARG;
	}

	MENUITEMINFOW menuItem = { sizeof(MENUITEMINFOW), 0, };
	HMENU hMenu = CreatePopupMenu();
	if (hMenu == nullptr) {
		return E_FAIL;
	}

	InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, INFOBARMENU_OPENURL, L"Open web site");
	*pMenu = hMenu;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CInfoBarMessageImpl::HandleMenu(HWND hwnd, INT menuId) {

	switch (menuId) {
		case INFOBARMENU_OPENURL: {
			if (_url.empty() == false) {
				ShellExecuteW(hwnd, L"open", _url.c_str(), nullptr, nullptr, SW_SHOW);
			}

		}; break;
	}

	return S_OK;
}

GUID* CInfoBarMessageImpl::GetMessageGUID() {
	return &(this->guid);
}

void CInfoBarMessageImpl::SetMessageW(const std::wstring& message, const std::wstring& url) {
	_message = message;
	_url = url;
}