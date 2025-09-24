#pragma once

#include "WebView2.h"
#include <string>

class WebViewBase
{
public:
	static BOOL InitializeWebViewWindow();
	static void ReleaseWebViewWindow();

	HWND GetHwnd();

	void Show();
	void Hide();
	void Release();

protected:
	WebViewBase();
	virtual ~WebViewBase();

	void Navigate(LPCWSTR url);
	bool StartBackgroundTask(std::wstring message);
	void ReportBackgroundTaskEnd(const std::wstring& result, DWORD status);

	virtual LPCWSTR GetTitle() = 0;
	virtual LPCWSTR GetDefaultUrl() = 0;
	virtual int GetWindowDefaultWidth() = 0;
	virtual int GetWindowDefaultHeight() = 0;
	virtual void OnMessageReceived(std::wstring message) = 0;
	virtual void OnBackgroundTask(std::wstring message) = 0;
	virtual void OnBackgroundTaskEnd(const std::wstring& result, DWORD status) = 0;

	void SendCommand(std::wstring message);

private:
	static LRESULT WINAPI WndProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
	);

	static DWORD WINAPI BackgroundTask(PVOID args);

	void ResizeWebView();
	void ResizeWebView(RECT* rect);
	void SetSettings();
	void PrepareMessageReceivedHandler();
	void ReleaseMessageReceivedHandler();
	void ResetBackgroundTask();

	static WebViewBase* GetInstance(HWND hwnd);

	HANDLE _backgroundThread;
	HANDLE _backgroundThreadWake;
	BOOL _isBackgroundTaskRunning;
	std::wstring _backgroundTaskCommand;
	CRITICAL_SECTION _section;
	BOOL _WillWindowClosed;

	ICoreWebView2Controller* _webViewController;
	EventRegistrationToken _messageReceivedToken;
	HWND _hwnd;
};

