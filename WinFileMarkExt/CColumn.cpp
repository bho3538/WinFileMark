#include "stdafx.h"
#include "CColumn.h"

#include <shlwapi.h>
#include <propkey.h>

const PROPERTYKEY PKEY_ItemWinFileMarkStatus = { {0x41376319, 0xd83f, 0x4c78, 0x96, 0x73, 0xf0, 0xb9, 0x6, 0x17, 0x45 }, 1 };

CColumn::CColumn() {

}

CColumn::~CColumn() {
	if (_pColumnManager != nullptr) {
		_pColumnManager->Release();
	}
}

HRESULT CColumn::Initialize(IShellView* shview) {
	if (shview == nullptr) {
		return E_INVALIDARG;
	}

	HRESULT hr = IUnknown_QueryService(shview, IID_IFolderView, IID_IColumnManager, reinterpret_cast<void**>(&_pColumnManager));
	if (hr != S_OK) {
		return hr;
	}

	return hr;
}

HRESULT CColumn::ShowWinFileMarkStatusColumn() {
	if (_pColumnManager == nullptr) {
		return E_FAIL;
	}


	// WinFileMark 컬럼 정보를 조회
	CM_COLUMNINFO colInfo = { sizeof(CM_COLUMNINFO), CM_MASK_STATE, 0, };
	HRESULT hr = _pColumnManager->GetColumnInfo(PKEY_ItemWinFileMarkStatus, &colInfo);

	if (hr == S_OK) {
		// 해당 컬럼을 visible 로 설정
		colInfo.dwState = CM_STATE_VISIBLE;
		hr = _pColumnManager->SetColumnInfo(PKEY_ItemWinFileMarkStatus, &colInfo);
	}

	return hr;
}