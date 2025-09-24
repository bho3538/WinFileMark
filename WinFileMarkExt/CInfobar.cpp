#include "stdafx.h"
#include "CInfoBar.h"

const IID IID_IBrowserProgressConnecion = { 0x20174539,0xb2c7,0x4ec7,{0x97,0x0b,0x04,0x20,0x1f,0x9c,0xdb,0xad} };

CInfoBar::CInfoBar(LPCWSTR folderPath) {
	_folderPath = folderPath;
}


CInfoBar::~CInfoBar() {
	if (this->_infoMsg != nullptr) {
		this->_infoMsg->Release();
	}

	if (this->sessionProvider != nullptr) {
		this->sessionProvider->Release();
	}

	if (this->sprovider != nullptr) {
		this->sprovider->Release();
	}

	if (this->browserProgress != nullptr) {
		this->browserProgress->Release();
	}

	if (this->host != nullptr) {
		this->host->Release();
	}

	if (this->git != nullptr) {
		this->git->Release();
	}
}

HRESULT CInfoBar::ShowMessage() {

	TBINFOBARDATA infobarData;

	DB db(_folderPath);
	if (db.GetInfobarMessage(&infobarData) == false) {
		return S_FALSE;
	}

	this->_infoMsg = new CInfoBarMessageImpl();
	this->_infoMsg->SetMessageW(infobarData.infobarMessage, infobarData.url);

	HRESULT hr = this->host->Inform(this->_infoMsg);

	return hr;
}

HRESULT CInfoBar::Initialize(IShellView* shview) {
	HRESULT hr;
	DWORD dwCookie = 0;

	if (!this->sprovider) {
		hr = shview->QueryInterface(IID_IServiceProvider, (PVOID*)&sprovider);
		if (FAILED(hr)) {
			goto escapeArea;
		}
	}

	if (!this->sessionProvider) {
		hr = this->sprovider->QueryService(IID_IBrowserProgressAggregator, IID_IBrowserProgressSessionProvider, (PVOID*)&this->sessionProvider);
		if (FAILED(hr)) {
			goto escapeArea;
		}
	}

	hr = sessionProvider->GetCurrentSession(&dwCookie);
	if (FAILED(hr)) {
		goto escapeArea;
	}

	//Get GIT
	if (!this->git) {
		hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,
			IID_IGlobalInterfaceTable, (PVOID*)&this->git);
		if (FAILED(hr)) {
			goto escapeArea;
		}
	}

	hr = this->git->GetInterfaceFromGlobal(dwCookie, IID_IBrowserProgressConnecion, (PVOID*)&this->browserProgress);
	if (hr == S_OK) {
		hr = this->browserProgress->QueryInterface(IID_IInfoBarHost, (PVOID*)&this->host);
	}

escapeArea:

	return hr;
}

void CInfoBar::Close() {
	if (this->host && this->_infoMsg) {
		GUID messageId = *(this->_infoMsg->GetMessageGUID());
		this->host->CancelInform(messageId);
	}
}
