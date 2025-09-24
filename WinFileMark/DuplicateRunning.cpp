#include "pch.h"
#include "DuplicateRunning.h"

DuplicateRunning::DuplicateRunning() :
	_mutex(nullptr)
{}

DuplicateRunning::~DuplicateRunning()
{
	if (_mutex != nullptr)
	{
		::ReleaseMutex(_mutex);

		_mutex = nullptr;
	}
}

DuplicateRunning* DuplicateRunning::CreateInstance()
{
	return new DuplicateRunning();
}

bool DuplicateRunning::Check()
{
	if (_mutex != nullptr)
	{
		return false;
	}

	_mutex = ::CreateMutexW(nullptr, TRUE, L"WinFileMark_Running");
	if (_mutex == nullptr)
	{
		return false;
	}

	if (::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::ReleaseMutex(_mutex);
		_mutex = nullptr;

		return false;
	}

	return true;
}

void DuplicateRunning::ShowPreviousRunning()
{
	HWND hwnd = ::FindWindowW(L"WindowManagerWindow", L"Winfilemark");
	if (hwnd != nullptr)
	{
		::PostMessageW(hwnd, _WINFILEMARK_TRAY_MESSAGE, 0, WM_LBUTTONDBLCLK);
	}
}

void DuplicateRunning::Release()
{
	delete this;
}