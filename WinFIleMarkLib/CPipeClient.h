#pragma once

#include "CDataModelBase.h"

class CPipeClient
{
public:
	CPipeClient();
	~CPipeClient();

	DWORD Connect(LPCWSTR pipeServerName);

	void Close();

	BOOL IsConnected();

	DWORD Send(const BYTE* data, DWORD dataLen);
	DWORD Send(CDataModelBase* data);

	DWORD Receive(BYTE** data);
	DWORD Receive(CDataModelBase* data);

private:
	HANDLE _hPipe;
};

