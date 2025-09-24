#include "pch.h"

#include "WindowManager.h"
#include "WinFIleMarkServer.h"
#include "DuplicateRunning.h"

int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow
)
{
	HRESULT hr = ::CoInitialize(0);
	if (hr != S_OK)
	{
		return -1;
	}

	// 중복 실행 확인
	DuplicateRunning* dup = DuplicateRunning::CreateInstance();
	if (dup == nullptr)
	{
		return -1;
	}

	if (dup->Check() == false)
	{
		// 이미 실행중
		dup->ShowPreviousRunning();
		dup->Release();

		return 1;
	}

	WinFIleMarkServer pipeServer;
	pipeServer.Start(L"TEST");
	
	WindowManager windowManager;

	windowManager.Initialize();

	windowManager.Run();

	dup->Release();

	::CoUninitialize();

	return 0;
}