#pragma once

#include <vector>
#include "CWinFileMarkChildContextMenu.h"

class CWinFileMarkEnumContextMenu :
	public IEnumExplorerCommand
{
public:
	CWinFileMarkEnumContextMenu(const std::wstring& folderPath);
	~CWinFileMarkEnumContextMenu();

	// IUnknown
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	// IEnumExplorerCommand
    HRESULT __stdcall Next(ULONG celt, IExplorerCommand** pUICommand, ULONG* pceltFetched);
    HRESULT __stdcall Skip(ULONG celt);
    HRESULT __stdcall Reset();
    HRESULT __stdcall Clone(IEnumExplorerCommand** ppenum);

private:
	ULONG _fetched;
	ULONG _index;
	LONG _refCount;

	std::vector<CWinFileMarkChildContextMenu*> _commands;
};

