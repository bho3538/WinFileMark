#pragma once

#include <vector>
#include <string>
#include "CPipeContext.h"
#include "CDataModelBase.h"
#include "CMemoryReader.h"
#include "CMemoryStream.h"

class CPipeServer
{
public:
	CPipeServer();
	~CPipeServer();

	DWORD Start(LPCWSTR pipeServerName);
	DWORD Stop();

protected:

	virtual void OnDataReceived(CMemoryReader* reader, CMemoryStream** ppResponse) = 0;

private:

	static DWORD WINAPI WorkerThreads(PVOID args);

	DWORD PrepareReadData(CPipeContext* ctx);
	DWORD ReadData(CPipeContext* ctx);
	DWORD PrepareWriteData(CPipeContext* ctx, BYTE* data, DWORD dataLen);
	DWORD WriteData(CPipeContext* ctx);
	DWORD CreateHandleAndWaitConnection(CPipeContext* ctx);
	DWORD DisconnectAndReset(CPipeContext* ctx);

	void SetRecvBuffer(CPipeContext* ctx, DWORD dataLen);
	void FreeRecvBuffer(CPipeContext* ctx, BOOL cleanup);

	void FreeSendBuffer(CPipeContext* ctx);

	HANDLE _hIocp;
	std::vector<HANDLE> _workerThreads;
	std::vector<CPipeContext*> _ctxs;
	std::wstring _pipeServerName;
};

