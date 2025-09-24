#pragma once
class WebViewWaitEvent
{
public:
	WebViewWaitEvent();
	~WebViewWaitEvent();

	BOOL Initialize();

	void Set();
	void Wait();

private:

	HANDLE _hWait;
};

