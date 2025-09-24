#include "pch.h"
#include "WebViewWaitEvent.h"

WebViewWaitEvent::WebViewWaitEvent() :
	_hWait(NULL)
{}

WebViewWaitEvent::~WebViewWaitEvent()
{
	if (_hWait != NULL)
	{
		::CloseHandle(_hWait);
	}
}

BOOL WebViewWaitEvent::Initialize()
{
	_hWait = ::CreateEventW(
		NULL,
		TRUE,
		FALSE,
		NULL
	);
	
	return _hWait != NULL;
}

void WebViewWaitEvent::Set()
{
	if (_hWait != NULL)
	{
		::SetEvent(_hWait);
	}
}

void WebViewWaitEvent::Wait()
{
	if (_hWait == NULL)
	{
		return;
	}

	const HANDLE handles[] = 
	{ 
		_hWait 
	};

	// 메시지 루프를 돌리면서 이벤트를 대기
	while (TRUE)
	{
		DWORD idx = ::MsgWaitForMultipleObjects(
			_countof(handles),
			handles,
			FALSE,
			INFINITE,
			QS_ALLINPUT
		);

		if (idx == WAIT_OBJECT_0)
		{
			// set이 발생함
			break;
		}
		else if (idx == WAIT_OBJECT_0 + _countof(handles))
		{
			MSG msg;
			while (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		}
		else
		{
			// 알 수 없는 오류가 발생
			break;
		}

	}
}