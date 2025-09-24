#include "pch.h"
#include "WindowManager.h"

#include "WebViewBase.h"
#include "resource.h"

static LPCWSTR g_WindowClassName = L"WindowManagerWindow";
static LPCWSTR g_WinManagerBaseInstanceKey = L"WindowManagerindowInst";

static ATOM g_WindowClass = NULL;

WindowManager::WindowManager() : 
	_hwnd(NULL)
{
	::ZeroMemory(&_trayMenu, sizeof(NOTIFYICONDATAW));
}

WindowManager::~WindowManager()
{}

BOOL WindowManager::Initialize()
{
	WNDCLASS wc = { 0, };
	wc.lpfnWndProc = WindowManager::WndProc;
	wc.lpszClassName = g_WindowClassName;

	g_WindowClass = ::RegisterClassW(&wc);
	if (g_WindowClass == NULL)
	{
		return FALSE;
	}

	// 프로그램을 제어하기 위한 윈도우 생성 (숨겨진 창)
	_hwnd = CreateWindowW(
		g_WindowClassName,
		L"Winfilemark",
		WS_OVERLAPPED,
		0,
		0,
		0,
		0,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (_hwnd == NULL)
	{
		return FALSE;
	}

	::ShowWindow(
		_hwnd,
		SW_HIDE
	);

	// Tray 초기화
	_trayMenu.cbSize = sizeof(_trayMenu);
	_trayMenu.hWnd = _hwnd;
	_trayMenu.uID = 1001;
	_trayMenu.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	_trayMenu.uCallbackMessage = _WINFILEMARK_TRAY_MESSAGE;
	_trayMenu.hIcon = ::LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1));
	::wcscpy_s(_trayMenu.szTip, ARRAYSIZE(_trayMenu.szTip), L"WinFileMark");

	// Tray menu 생성
	if (Shell_NotifyIconW(
		NIM_ADD,
		&_trayMenu
	) == FALSE)
	{
		
	}

	// 메시지 프로시저에서 class instance 접근을 위해 사용
	::SetPropW(
		_hwnd,
		g_WinManagerBaseInstanceKey,
		this
	);

	// WebView 초기화
	if (WebViewBase::InitializeWebViewWindow() == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

void WindowManager::Release()
{
	// main window 정리
	_mainWindow.Release();

	// webview 정리
	WebViewBase::ReleaseWebViewWindow();

	// 창 등록 해제
	if (g_WindowClass != NULL)
	{
		::UnregisterClassW(g_WindowClassName, NULL);
	}
}

void WindowManager::Run()
{
	// 메인 메시지 루프
	MSG msg;
	while (::GetMessageW(
		&msg,
		NULL,
		0,
		0
	))
	{
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}
}

LRESULT WINAPI WindowManager::WndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	static UINT s_uTaskbarRestart = UINT_MAX;

	switch (msg)
	{
		case WM_CREATE:
		{
			s_uTaskbarRestart = ::RegisterWindowMessageW(L"TaskbarCreated");
			break;
		}
		case _WINFILEMARK_TRAY_MESSAGE:
		{
			if (lParam == WM_LBUTTONDBLCLK)
			{
				WindowManager* _this = (WindowManager*)::GetPropW(
					hwnd, 
					g_WinManagerBaseInstanceKey
				);

				if (_this != NULL)
				{
					_this->_mainWindow.Show();
				}

				return 0;
			}
			else if (lParam == WM_RBUTTONUP)
			{
				HMENU menu = ::CreatePopupMenu();
				if (!menu)
				{
					return 0;
				}

				::AppendMenuW(menu, MF_STRING, _WINFILEMARK_TRAY_OPEN_COMMAND, L"Open");
				::AppendMenuW(menu, MF_STRING, _WINFILEMARK_TRAY_EXIT_COMMAND, L"Exit");

				::SetForegroundWindow(hwnd);

				// 메뉴를 띄울 위치를 찾기
				POINT pt;
				::GetCursorPos(&pt);

				// 해당 위치에 메뉴를 띄우기
				::TrackPopupMenuEx(
					menu,
					TPM_RIGHTBUTTON | TPM_LEFTALIGN,
					pt.x,
					pt.y,
					hwnd,
					NULL
				);

				::DestroyMenu(menu);

				return 0;
			}
			break;
		}
		case WM_COMMAND:
		{
			WORD command = LOWORD(wParam);
			if (command == _WINFILEMARK_TRAY_OPEN_COMMAND)
			{
				WindowManager* _this = (WindowManager*)::GetPropW(
					hwnd,
					g_WinManagerBaseInstanceKey
				);

				if (_this != NULL)
				{
					_this->_mainWindow.Show();
				}
			}
			else if (command == _WINFILEMARK_TRAY_EXIT_COMMAND)
			{
				::PostMessageW(
					hwnd,
					WM_CLOSE,
					0,
					0
				);
			}

			break;
		}
		case WM_CLOSE:
		{
			WindowManager* _this = (WindowManager*)::GetPropW(
				hwnd,
				g_WinManagerBaseInstanceKey
			);

			if (_this != NULL)
			{
				_this->Release();
			}

			::DestroyWindow(hwnd);

			break;
		}
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			break;
		}
		default:
		{
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