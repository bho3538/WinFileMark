#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Unknwn.h>
#include <shobjidl_core.h>

#include <string>

#define WINFILEMARK_REDICON 1
#define WINFILEMARK_GREENICON 2
#define WINFILEMARK_BLUEICON 4

#define WinFileMark_EXT __declspec(dllimport)

#include "../WinFIleMarkLib/CShellUtils.h"
#include "../WinFIleMarkLib/CStringUtils.h"
#include "../WinFIleMarkLib/DB.h"


// link 
#ifdef _WIN64
#ifdef _M_ARM64
#pragma comment(lib, "../extern_lib/Detours/lib.ARM64/detours.lib")
#else
#pragma comment(lib, "../extern_lib/Detours/lib.X64/detours.lib")
#pragma comment(lib, "../x64/Debug/WinFileMarkLib.lib")
#endif
#else
#pragma comment(lib, "../extern_lib/Detours/lib.X86/detours.lib")
#pragma comment(lib, "../Debug/WinFileMarkLib.lib")
#endif

#define _PIPE_SERVER_NAME L"TEST"

#pragma comment(lib, "Propsys.lib")


