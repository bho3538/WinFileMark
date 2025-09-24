#include "stdafx.h"
#include "CInfoBar.h"
#include "CInfoBarManager.h"

void CInfoBarManager::ShowInfoBar(HWND hwnd, IShellView* pSV, LPCWSTR folderPath) {
	if (pSV == nullptr) {
		return;
	}

	CInfoBar* pInfobar = new CInfoBar(folderPath);
	HRESULT hr = pInfobar->Initialize(pSV);
	if (hr == S_OK) {
		hr = pInfobar->ShowMessage();

		if (hr == S_OK) {
			InsertCurrentViewInfoBar(hwnd, pInfobar);
		}
	}

	if (hr != S_OK) {
		delete pInfobar;
	}
}

void CInfoBarManager::CloseInfoBar(HWND hwnd) {
	DeleteInfoBar(hwnd);
}


void CInfoBarManager::InsertCurrentViewInfoBar(HWND hwnd, CInfoBar* pInfobar) {
	if (hwnd == 0 || pInfobar == nullptr) {
		return;
	}

	_infobars[hwnd] = pInfobar;
}

CInfoBar* CInfoBarManager::GetCurrentViewInfoBar(HWND hwnd) {
	auto found = _infobars.find(hwnd);

	if (found == _infobars.end()) {
		return nullptr;
	}

	return found->second;
}

void CInfoBarManager::DeleteInfoBar(HWND hwnd) {
	auto found = _infobars.find(hwnd);
	if (found == _infobars.end()) {
		return;
	}
	CInfoBar* pInfobar = found->second;

	_infobars.erase(hwnd);

	if (pInfobar != nullptr) {
		delete pInfobar;
	}
}