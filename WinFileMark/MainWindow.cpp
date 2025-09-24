#include "pch.h"
#include "MainWindow.h"

#include "InstallHelper.h"

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{

}

LPCWSTR MainWindow::GetTitle()
{
	return L"WinFileMark";
}

LPCWSTR MainWindow::GetDefaultUrl()
{
	if (InstallHelper::IsExtensionInstalled() == TRUE)
	{
		return L"http://localhost:3000/#/main?installed=1";
	}
	else
	{
		return L"http://localhost:3000/#/main?installed=0";
	}
}

int MainWindow::GetWindowDefaultWidth()
{
	return 500;
}

int MainWindow::GetWindowDefaultHeight()
{
	return 250;
}

void MainWindow::OnMessageReceived(std::wstring message)
{
	StartBackgroundTask(message);
}

void MainWindow::OnBackgroundTask(std::wstring message)
{
	if (message == L"REQ_INSTALL")
	{
		DWORD status = InstallHelper::InstallModule();

		::Sleep(1500);
		
		ReportBackgroundTaskEnd(L"BKG_INSTALL_END", status);
	}
	else if (message == L"REQ_UNINSTALL")
	{
		DWORD status = InstallHelper::UnInstallModule();

		::Sleep(1500);

		ReportBackgroundTaskEnd(L"BKG_UNINSTALL_END", status);
	}
}

void MainWindow::OnBackgroundTaskEnd(const std::wstring& result, DWORD status)
{
	// send command to web

	if (result == L"BKG_INSTALL_END")
	{
		switch (status)
		{
			case NO_ERROR:
			{
				SendCommand(L"REP_INSTALL_SUCCESS");

				break;
			}
			default:
			{
				SendCommand(L"REP_INSTALL_FAILED");

				break;
			}
		}
	}
	else if (result == L"BKG_UNINSTALL_END")
	{
		switch (status)
		{
			case NO_ERROR:
			{
				SendCommand(L"REP_UNINSTALL_SUCCESS");

				break;
			}
			default:
			{
				SendCommand(L"REP_UNINSTALL_FAILED");

				break;
			}
		}
	}
}