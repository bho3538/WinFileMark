#include "pch.h"
#include "WebViewBase.h"
#include "WebViewWaitEvent.h"

#include <wrl.h>
using namespace Microsoft::WRL;

static ATOM g_WindowClass = NULL;
static ICoreWebView2Environment* g_WebViewEnv = NULL;

static LPCWSTR g_WindowClassName = L"WebViewWindow";
static LPCWSTR g_WebViewBaseInstanceKey = L"WebViewWindowInst";

#define WM_WEBVIEW_BACKGROUND_END WM_APP + 2

WebViewBase::WebViewBase() :
	_hwnd(NULL),
	_webViewController(NULL),
	_isBackgroundTaskRunning(FALSE),
	_backgroundThread(NULL),
	_backgroundThreadWake(NULL),
	_backgroundTaskCommand(L""),
	_WillWindowClosed(FALSE)
{
	_messageReceivedToken.value = 0;

	::InitializeCriticalSection(&_section);

	_backgroundThreadWake = ::CreateEventW(
		NULL,
		FALSE,
		FALSE,
		NULL
	);

	_backgroundThread = ::CreateThread(
		NULL,
		0,
		WebViewBase::BackgroundTask,
		this,
		0,
		NULL
	);
}

WebViewBase::~WebViewBase()
{
	Release();
}

BOOL WebViewBase::InitializeWebViewWindow()
{
	WNDCLASS wc = { 0, };
	wc.lpfnWndProc = WebViewBase::WndProc;
	wc.lpszClassName = g_WindowClassName;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	g_WindowClass = ::RegisterClassW(&wc);
	if (g_WindowClass == NULL)
	{
		return FALSE;
	}

	WebViewWaitEvent waitEvent;
	if (waitEvent.Initialize() == FALSE)
	{
		return FALSE;
	}

	::CreateCoreWebView2EnvironmentWithOptions(
		NULL,
		NULL,
		NULL,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[&waitEvent](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

				if (result == S_OK)
				{
					if (env == NULL)
					{
						return E_FAIL;
					}

					g_WebViewEnv = env;
					g_WebViewEnv->AddRef();
				}

				waitEvent.Set();

				return result;
			}
		).Get()
	);

	// webview 초기화를 대기
	waitEvent.Wait();

	// webview 초기화에 실패한 경우
	if (g_WebViewEnv == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

void WebViewBase::ReleaseWebViewWindow()
{
	if (g_WebViewEnv != NULL)
	{
		g_WebViewEnv->Release();
		g_WebViewEnv = NULL;
	}

	if (g_WindowClass != NULL)
	{
		::UnregisterClassW(
			g_WindowClassName,
			NULL
		);
		g_WindowClass = NULL;
	}
}

HWND WebViewBase::GetHwnd()
{
	return _hwnd;
}

void WebViewBase::Navigate(LPCWSTR url)
{
	if (_webViewController == NULL)
	{
		return;
	}

	 // url 검사
	if (url == NULL || ::wcslen(url) == 0)
	{
		return;
	}

	ICoreWebView2* webView = NULL;
	if (_webViewController->get_CoreWebView2(&webView) == S_OK && webView != NULL)
	{
		webView->add_NavigationCompleted(
			Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[this](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT {

					// 창을 다시 보여주기
					::SetLayeredWindowAttributes(
						_hwnd,
						0,
						255,
						LWA_ALPHA
					);

					return S_OK;
				}).Get(),
			NULL
		);

		webView->Navigate(url);

		webView->Release();
	}
}

void WebViewBase::Show()
{
	// 전역 initialize 를 안한 경우
	if (g_WebViewEnv == NULL)
	{
		return;
	}

	// 이후 호출인 경우 (창을 숨겼다가 다시 보여주는 경우)
	if (_hwnd != NULL)
	{
		::ShowWindow(
			_hwnd,
			SW_SHOW
		);

		return;
	}

	// 최초 호출인 경우
	_hwnd = ::CreateWindowExW(
		WS_EX_LAYERED,
		g_WindowClassName,
		GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		GetWindowDefaultWidth(),
		GetWindowDefaultHeight(),
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (_hwnd == NULL)
	{
		return;
	}

	// 메시지 프로시저에서 class instance 접근을 위해 사용
	::SetPropW(
		_hwnd,
		g_WebViewBaseInstanceKey,
		this
	);

	// 창을 투명하게 설정 (깜밖임 완화)
	::SetLayeredWindowAttributes(
		_hwnd,
		0,
		0,
		LWA_ALPHA
	);

	// 창 표시 (창을 반드시 먼저 표시해야 함)
	::ShowWindow(
		_hwnd,
		SW_SHOW
	);

	// 창이 뒤로 숨는 현상을 방지하기 위해 topmost 후 topmost 해제
	::SetWindowPos(
		_hwnd,
		HWND_TOPMOST,
		0,
		0,
		0,
		0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
	);

	WebViewWaitEvent waitEvent;
	if (waitEvent.Initialize() == FALSE)
	{
		return;
	}

	HRESULT hr = g_WebViewEnv->CreateCoreWebView2Controller(
		_hwnd,
		Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
			[this, &waitEvent](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {

				if (result == S_OK)
				{
					if (controller == NULL)
					{
						return E_FAIL;
					}

					_webViewController = controller;
					_webViewController->AddRef();

					// 기본 설정 적용
					SetSettings();

					// 크기 조절
					ResizeWebView();

					// 메시지 수신기 준비
					PrepareMessageReceivedHandler();

					// 대기 해제
					waitEvent.Set();
				}

				return result;
			})
		.Get()
	);

	// 웹 뷰가 다 만들어질때까지 대기
	waitEvent.Wait();

	Navigate(GetDefaultUrl());

	::SetWindowPos(
		_hwnd,
		HWND_NOTOPMOST,
		0,
		0,
		0,
		0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
	);
}

void WebViewBase::Hide()
{
	::ShowWindow(
		_hwnd,
		SW_HIDE
	);
}

void WebViewBase::Release()
{
	_WillWindowClosed = TRUE;

	ReleaseMessageReceivedHandler();

	if (_webViewController != NULL)
	{
		_webViewController->Release();
		_webViewController = NULL;
	}

	if (_hwnd != NULL)
	{
		::CloseWindow(_hwnd);
		::DestroyWindow(_hwnd);

		_hwnd = NULL;
	}

	if (_backgroundThreadWake != NULL)
	{
		// 깨우기
		::SetEvent(_backgroundThreadWake);
		::CloseHandle(_backgroundThreadWake);
		_backgroundThreadWake = NULL;
	}

	if (_backgroundThread != NULL)
	{
		// 스레드가 완전히 종료될때까지 대기
		::WaitForSingleObject(_backgroundThread, INFINITE);
		::CloseHandle(_backgroundThread);
		_backgroundThread = NULL;
	}

	::DeleteCriticalSection(&_section);
}

LRESULT WINAPI WebViewBase::WndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (msg)
	{
		case WM_CLOSE:
		{
			WebViewBase* _this = GetInstance(hwnd);
			if (_this != NULL)
			{
				_this->Hide();
			}

			return 0;
		}
		case WM_DESTROY:
		{
			::RemovePropW(
				hwnd,
				g_WebViewBaseInstanceKey
			);

			break;
		}
		case WM_SIZE:
		{
			WebViewBase* _this = GetInstance(hwnd);
			if (_this != NULL)
			{
				_this->ResizeWebView();
			}

			break;
		}
		case WM_WEBVIEW_BACKGROUND_END:
		{
			LPWSTR data = (LPWSTR)lParam;
			if (data != NULL)
			{
				WebViewBase* _this = GetInstance(hwnd);
				if (_this != NULL)
				{
					_this->OnBackgroundTaskEnd(data, (DWORD)wParam);

					::free(data);

					_this->ResetBackgroundTask();
				}
			}
			break;
		}
	}

	return ::DefWindowProcW(
		hwnd,
		msg,
		wParam,
		lParam
	);
}

DWORD WINAPI WebViewBase::BackgroundTask(PVOID args)
{
	if (!args)
	{
		return 0;
	}

	WebViewBase* _this = (WebViewBase*)args;
	while (TRUE)
	{
		// 대기
		if (::WaitForSingleObject(
			_this->_backgroundThreadWake, 
			INFINITE
		) != WAIT_OBJECT_0)
		{
			break;
		}

		// 창 종료 요청이 온 경우 스레드 정지
		if (_this->_WillWindowClosed == TRUE)
		{
			break;
		}

		_this->OnBackgroundTask(_this->_backgroundTaskCommand);
	}

	return 0;
}

void WebViewBase::ResizeWebView()
{
	if (_hwnd == NULL || _webViewController == NULL)
	{
		return;
	}

	RECT bounds = { 0, };
	// 부모 창의 크기를 조회
	if (::GetClientRect(
		_hwnd,
		&bounds
	) != FALSE)
	{
		// webview 를 부모 창의 크기와 동일하게 설정
		_webViewController->put_Bounds(bounds);
	}
}

void WebViewBase::ResizeWebView(RECT* rect)
{
	if (_webViewController == NULL)
	{
		return;
	}

	_webViewController->put_Bounds(*rect);
}

void WebViewBase::SetSettings()
{
	if (_webViewController == NULL)
	{
		return;
	}

	_webViewController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);

	// 외부 파일 D&D 방지
	ICoreWebView2Controller4* controller4 = NULL;
	if (_webViewController->QueryInterface(&controller4) == S_OK)
	{
		controller4->put_AllowExternalDrop(FALSE);

		controller4->Release();
	}

	ICoreWebView2* webView = NULL;
	if (_webViewController->get_CoreWebView2(&webView) != S_OK || webView == NULL)
	{
		return;
	}

	ICoreWebView2Settings* settings = NULL;
	if (webView->get_Settings(&settings) != S_OK || settings == NULL)
	{
		webView->Release();

		return;
	}

	// 브라우저 기본 우클릭 방지
	//settings->put_AreDefaultContextMenusEnabled(FALSE);

	// 브라우저 하단 상태 바 비활성화
	settings->put_IsStatusBarEnabled(FALSE);

	ICoreWebView2Settings4* settings4 = NULL;
	if (settings->QueryInterface(&settings4) == S_OK)
	{
		// 자동 완성 비활성화
		settings4->put_IsGeneralAutofillEnabled(FALSE);
		settings4->Release();
	}
	
	settings->Release();
	webView->Release();
}


void WebViewBase::SendCommand(std::wstring message)
{
	if (_webViewController == NULL)
	{
		return;
	}

	ICoreWebView2* webView = NULL;
	if (_webViewController->get_CoreWebView2(&webView) != S_OK || webView == NULL)
	{
		return;
	}

	webView->PostWebMessageAsString(message.c_str());

	webView->Release();
}

void WebViewBase::PrepareMessageReceivedHandler()
{
	if (_webViewController == NULL)
	{
		return;
	}

	ICoreWebView2* webView = NULL;
	if (_webViewController->get_CoreWebView2(&webView) != S_OK || webView == NULL)
	{
		return;
	}

	webView->add_WebMessageReceived(
		Callback<ICoreWebView2WebMessageReceivedEventHandler>(
			[this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
				PWSTR message;
				args->TryGetWebMessageAsString(&message);

				if (message != NULL)
				{
					OnMessageReceived(message);

					::CoTaskMemFree(message);
				}

				return S_OK;
			}
		).Get(),
		&_messageReceivedToken
	);

	webView->Release();
}

void WebViewBase::ReleaseMessageReceivedHandler()
{
	if (_webViewController == NULL || _messageReceivedToken.value == 0)
	{
		return;
	}

	ICoreWebView2* webView = NULL;
	if (_webViewController->get_CoreWebView2(&webView) != S_OK || webView == NULL)
	{
		return;
	}

	webView->remove_WebMessageReceived(_messageReceivedToken);

	_messageReceivedToken.value = 0;
}

WebViewBase* WebViewBase::GetInstance(HWND hwnd)
{
	WebViewBase* inst = (WebViewBase*)::GetPropW(
		hwnd,
		g_WebViewBaseInstanceKey
	);

	return inst;
}

bool WebViewBase::StartBackgroundTask(std::wstring message)
{
	bool canRun = false;

	::EnterCriticalSection(&_section);

	canRun = !_isBackgroundTaskRunning;

	::LeaveCriticalSection(&_section);

	if (canRun == true)
	{
		_backgroundTaskCommand = message;

		::SetEvent(_backgroundThreadWake);
	}

	return canRun;
}

void WebViewBase::ResetBackgroundTask()
{
	::EnterCriticalSection(&_section);

	_isBackgroundTaskRunning = FALSE;

	::LeaveCriticalSection(&_section);
}

void WebViewBase::ReportBackgroundTaskEnd(const std::wstring& result, DWORD status)
{
	if (result.empty() == true)
	{
		return;
	}

	LPWSTR data = (LPWSTR)::malloc((result.length() + 1) * sizeof(WCHAR));
	if (data == NULL)
	{
		return;
	}

	::wcscpy_s(
		data,
		result.length() + 1,
		result.c_str()
	);

	::PostMessageW(
		_hwnd,
		WM_WEBVIEW_BACKGROUND_END,
		status,
		(LPARAM)data
	);
}
