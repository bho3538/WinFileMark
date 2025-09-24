#pragma once

#include "IInfoBarHost.h"
#include "IInfoBarMessage.h"
#include "IBrowserProgress.h"

#include "CInfoBarMessageImpl.h"

#include <string>
#include <ShlObj_core.h>



class CInfoBar
{
public:
	CInfoBar(LPCWSTR folderPath);
	~CInfoBar();

	HRESULT Initialize(IShellView* shview);
	HRESULT ShowMessage();

	void Close();

private:

	std::wstring _folderPath;
	CInfoBarMessageImpl* _infoMsg = NULL;

	IServiceProvider* sprovider = NULL;
	IBrowserProgressSessionProvider* sessionProvider = NULL;
	IGlobalInterfaceTable* git = NULL;
	IInfoBarHost* host = NULL;
	IUnknown* browserProgress = NULL;
};

