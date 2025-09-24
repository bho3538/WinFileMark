
#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Unknwn.h>
#include <shobjidl_core.h>

#include <string>

// link 
#ifdef _WIN64
#ifdef _M_ARM64

#else
#pragma comment(lib, "../extern_lib/SQLiteCpp/debug/x64/sqlite3.lib")
#pragma comment(lib, "../extern_lib/SQLiteCpp/debug/x64/sqlitecpp.lib")
#endif
#else
#pragma comment(lib, "../extern_lib/SQLiteCpp/debug/x86/sqlite3.lib")
#pragma comment(lib, "../extern_lib/SQLiteCpp/debug/x86/sqlitecpp.lib")
#endif

#endif //PCH_H
