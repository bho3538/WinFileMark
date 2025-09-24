#ifndef PCH_H
#define PCH_H

#include <Windows.h>

#define APP_NAME L"WinFileMark"

#ifdef _WIN64
#ifdef _M_ARM64

#else
#pragma comment(lib, "../x64/Debug/WinFileMarkLib.lib")
#endif
#else
#pragma comment(lib, "../Debug/WinFileMarkLib.lib")
#endif

#define _WINFILEMARK_TRAY_MESSAGE (WM_APP + 2)
#define _WINFILEMARK_TRAY_OPEN_COMMAND 0x1001
#define _WINFILEMARK_TRAY_EXIT_COMMAND 0x1002

#endif //PCH_H
