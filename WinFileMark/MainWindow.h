#pragma once

#include "WebViewBase.h"

class MainWindow : public WebViewBase
{
public:
	MainWindow();
	~MainWindow();

private:

	LPCWSTR GetTitle();
	LPCWSTR GetDefaultUrl();
	int GetWindowDefaultWidth();
	int GetWindowDefaultHeight();
	void OnMessageReceived(std::wstring message);
	void OnBackgroundTask(std::wstring message);
	void OnBackgroundTaskEnd(const std::wstring& result, DWORD status);
};

