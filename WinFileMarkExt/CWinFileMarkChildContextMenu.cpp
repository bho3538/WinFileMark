#include "stdafx.h"
#include "CWinFileMarkChildContextMenu.h"
#include <ShlGuid.h>
#include <Shlwapi.h>

#include "IconStatusCacheManager.h"
#include "../WinFIleMarkLib/CRequestSetIconStatus.h"
#include "../WinFIleMarkLib/CPipeClient.h"

CWinFileMarkChildContextMenu::CWinFileMarkChildContextMenu(const std::wstring& folderPath, const std::wstring& label, int iconFlags) :
	_refCount(1),
	_punkSite(nullptr),
	_folderPath(folderPath),
	_label(label),
	_iconFlags(iconFlags),
	_isUnsetMode(false)
{
}

CWinFileMarkChildContextMenu::~CWinFileMarkChildContextMenu() {
	if (_punkSite != nullptr) {
		_punkSite->Release();
	}
}

// IUnknown
HRESULT CWinFileMarkChildContextMenu::QueryInterface(REFIID riid, void** ppv) {
	if (ppv == nullptr) {
		return E_POINTER;
	}

	*ppv = nullptr;

	if (riid == IID_IUnknown) {
		*ppv = static_cast<IExplorerCommand*>(this);
	}
	else if (riid == IID_IObjectWithSite) {
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

ULONG CWinFileMarkChildContextMenu::AddRef() {
	return InterlockedIncrement(&_refCount);
}

ULONG CWinFileMarkChildContextMenu::Release() {
	ULONG refCount = InterlockedDecrement(&_refCount);
	if (refCount == 0) {
		delete this;
	}

	return refCount;
}

HRESULT CWinFileMarkChildContextMenu::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) {
	if (!ppszName) return E_POINTER;
	return SHStrDupW(_label.c_str(), ppszName);
}

HRESULT CWinFileMarkChildContextMenu::GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) {
	return E_NOTIMPL;
}

HRESULT CWinFileMarkChildContextMenu::GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) {
	return E_NOTIMPL;
}

HRESULT CWinFileMarkChildContextMenu::GetCanonicalName(GUID* pguidCommandName) {
	if (!pguidCommandName) return E_POINTER;
	*pguidCommandName = GUID_NULL;
	return S_OK;
}

HRESULT CWinFileMarkChildContextMenu::GetState(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState) {
	if (!pCmdState) return E_POINTER;
	*pCmdState = ECS_ENABLED | ECS_CHECKBOX;

	if (psiItemArray != nullptr) {

		IShellItem* pFirstItem = nullptr;
		psiItemArray->GetItemAt(0, &pFirstItem);

		if (pFirstItem != nullptr) {

			const std::wstring fileName = GetItemName(pFirstItem, SIGDN_NORMALDISPLAY);
			if (fileName.empty() == false) {

				if (_folderPath.empty() == true)
				{
					// 드물게 특정 상황에서 folderPath 를 구할 수 없음 (q-dir 등)
					// 이럴 경우는 ShellItem 의 FS 경로를 사용하여 부모 폴더 경로를 취득
					const std::wstring dirPath = GetItemName(pFirstItem, SIGDN_FILESYSPATH);
					if (dirPath.empty() == false)
					{
						size_t lastOffset = dirPath.find_last_of(L"\\");
						_folderPath = dirPath.substr(0, lastOffset);
					}
				}

				// todo
				//  이 방식이 최선일까????
				extern IconStatusCacheManager g_IconStatusCache;

				IconStatusCache* cache = g_IconStatusCache.GetFolderCache(_folderPath);
				if (cache != NULL)
				{
					int itemIconFlags = cache->GetFileMarkStatus(fileName);
					if (itemIconFlags > 0 && (itemIconFlags & _iconFlags) == _iconFlags)
					{
						*pCmdState |= ECS_CHECKED;
						_isUnsetMode = true;
					}
				}
			}

			pFirstItem->Release();
		}
	}

	return S_OK;
}

HRESULT CWinFileMarkChildContextMenu::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) {

	if (psiItemArray == nullptr) {
		return E_INVALIDARG;
	}

	DWORD cnt = 0;
	HRESULT hr = psiItemArray->GetCount(&cnt);
	if (hr != S_OK) {
		return hr;
	}

	// todo
	//  이 방식이 최선일까????
	extern IconStatusCacheManager g_IconStatusCache;

	IconStatusCache* cache = g_IconStatusCache.GetFolderCache(_folderPath);
	if (cache == NULL)
	{
		return S_OK;
	}

	CRequestSetIconStatus reqSetIcon(_folderPath);
	reqSetIcon.StatusIconFlags = _iconFlags;
	reqSetIcon.UnsetMode = _isUnsetMode;

	for (DWORD i = 0; i < cnt; i++) {
		IShellItem* pItem = nullptr;
		psiItemArray->GetItemAt(i, &pItem);

		if (pItem == nullptr) {
			continue;
		}

		const std::wstring fileName = GetItemName(pItem, SIGDN_NORMALDISPLAY);
		if (fileName.empty() == false) {

			int itemIconFlags = cache->GetFileMarkStatus(fileName);

			if (_isUnsetMode == false) {
				if (itemIconFlags <= 0 || (itemIconFlags & _iconFlags) != _iconFlags) {
					cache->SetFileMarkStatus(fileName, _iconFlags);
				}
			}
			else {
				if ((itemIconFlags & _iconFlags) == _iconFlags) {
					cache->UnsetFileMarkStatus(fileName, _iconFlags);
				}
			}

			// refresh icon status
			const std::wstring filePath = GetItemName(pItem, SIGDN_FILESYSPATH);
			if (filePath.empty() == false) {
				SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSH, filePath.c_str(), nullptr);
			}

			reqSetIcon.AddItem(fileName);
		}

		pItem->Release();
	}

	// 서버로 갱신 정보 전달
	CPipeClient client;
	client.Connect(_PIPE_SERVER_NAME);
	client.Send(&reqSetIcon);
	client.Close();
	
	return S_OK;
}

HRESULT CWinFileMarkChildContextMenu::GetFlags(EXPCMDFLAGS* pFlags) {
	*pFlags = ECF_DEFAULT;
	return S_OK;
}

HRESULT CWinFileMarkChildContextMenu::EnumSubCommands(IEnumExplorerCommand** ppEnum) {
	return E_NOTIMPL;
}

const std::wstring CWinFileMarkChildContextMenu::GetItemName(IShellItem* pItem, SIGDN nameFlags) {
	if (pItem == nullptr) {
		return L"";
	}

	LPWSTR pName = nullptr;

	HRESULT hr = pItem->GetDisplayName(nameFlags, &pName);
	if (hr != S_OK) {
		return L"";
	}

	std::wstring name = pName;

	CoTaskMemFree(pName);

	return name;
}