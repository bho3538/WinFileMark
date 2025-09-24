#pragma once

#include "MainWindow.h"

class WindowManager
{
public:
	WindowManager();
	~WindowManager();

	BOOL Initialize();
	void Run();

private:

	void Release();

	static LRESULT WINAPI WndProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
	);

	HWND _hwnd;
	NOTIFYICONDATAW _trayMenu;
	MainWindow _mainWindow;
};

