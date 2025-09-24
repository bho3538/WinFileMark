#include "stdafx.h"
#include "CWinFileMarkEnumContextMenu.h"


CWinFileMarkEnumContextMenu::CWinFileMarkEnumContextMenu(const std::wstring& folderPath) :
	_refCount(1),
	_fetched(0),
	_index(0)
{
	_commands.push_back(new CWinFileMarkChildContextMenu(folderPath, L"Red", WINFILEMARK_REDICON));
	_commands.push_back(new CWinFileMarkChildContextMenu(folderPath, L"Green", WINFILEMARK_GREENICON));
	_commands.push_back(new CWinFileMarkChildContextMenu(folderPath, L"Blue", WINFILEMARK_BLUEICON));
}

CWinFileMarkEnumContextMenu::~CWinFileMarkEnumContextMenu() {
	for (auto command : _commands) {
		command->Release();
	}

	_commands.clear();
}


HRESULT CWinFileMarkEnumContextMenu::QueryInterface(REFIID riid, void** ppv) {
	if (ppv == nullptr) {
		return E_POINTER;
	}

	*ppv = nullptr;

	if (riid == IID_IUnknown) {
		*ppv = static_cast<IUnknown*>(this);
	}
	else if (riid == IID_IEnumExplorerCommand) {
		*ppv = static_cast<IEnumExplorerCommand*>(this);
	}

	if (*ppv != nullptr) {
		this->AddRef();
		return S_OK;
	}
	else {
		return E_NOINTERFACE;
	}
}

ULONG CWinFileMarkEnumContextMenu::AddRef() {
	return InterlockedIncrement(&_refCount);
}

ULONG CWinFileMarkEnumContextMenu::Release() {
	ULONG refCount = InterlockedDecrement(&_refCount);
	if (refCount == 0) {
		delete this;
	}

	return refCount;
}

HRESULT CWinFileMarkEnumContextMenu::Next(ULONG celt, IExplorerCommand** pUICommand, ULONG* pceltFetched) {
	if (pUICommand == nullptr) {
		return E_POINTER;
	}

	if (pceltFetched) *pceltFetched = 0;

	ULONG fetched = 0;

	while ((fetched < celt) && (_index < _commands.size())) {
		pUICommand[fetched] = _commands[_index];
		pUICommand[fetched]->AddRef();
		_index++;
		fetched++;
	}

	if (pceltFetched) *pceltFetched = fetched;

	return (fetched == celt) ? S_OK : S_FALSE;
}

HRESULT CWinFileMarkEnumContextMenu::Skip(ULONG celt) {
	return E_NOTIMPL;
}

HRESULT CWinFileMarkEnumContextMenu::Reset() {
	_index = 0;

	return S_OK;
}

HRESULT CWinFileMarkEnumContextMenu::Clone(IEnumExplorerCommand** ppenum) {
	return E_NOTIMPL;
}