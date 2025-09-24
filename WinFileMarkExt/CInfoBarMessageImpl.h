#pragma once

#include "IInfoBarMessage.h"
#include <string>

class CInfoBarMessageImpl : 
	public IInfoBarMessage
{
public:

	CInfoBarMessageImpl() {
		//Create Random GUID
		CoCreateGuid(&guid);

		AddRef();
	}

	~CInfoBarMessageImpl() {
	}

	// IUnknown
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	HRESULT __stdcall GetMessageID(GUID* guid, INT* intVal);
	HRESULT __stdcall GetMessageW(LPWSTR* message);
	HRESULT __stdcall CreateMenu(HMENU* pMwnu);
	HRESULT __stdcall HandleMenu(HWND hwnd, INT intVal);
	
	GUID* GetMessageGUID();

	void SetMessageW(const std::wstring& message, const std::wstring& url);

private:
	LONG _refCount = 0;

	GUID guid;

	std::wstring _message;
	std::wstring _url;
};

