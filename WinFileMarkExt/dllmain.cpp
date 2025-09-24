// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "stdafx.h"
#include <string>

#include "CClassFactory.h"
#include "CHookShellFolder.h"

#include "CNotificationEditContextMenu.h"
#include "CWinFileMarkContextMenu.h"

// {FEB923B9-686D-4A77-80B6-C9C634FC6CC7}
const GUID CLSID_CNotificationEditContextMenu = { 0xfeb923b9, 0x686d, 0x4a77, { 0x80, 0xb6, 0xc9, 0xc6, 0x34, 0xfc, 0x6c, 0xc7 } };
// {FEB923B9-686D-4A77-80B6-C9C634FC6CC8}
const GUID CLSID_CFileMarkContextMenu = { 0xfeb923b9, 0x686d, 0x4a77, { 0x80, 0xb6, 0xc9, 0xc6, 0x34, 0xfc, 0x6c, 0xc8 } };


HMODULE g_Module = 0;
CHookShellFolder g_HookShellFolder;

HRESULT InstallAtRegistry();
HRESULT RegisterClsId(LPCWSTR clsid, LPCWSTR modulePath);
HRESULT RegisterOverlay(LPCWSTR clsid, DWORD clsidSize);
HRESULT RegisterContextMenu(LPCWSTR clsid, DWORD clsidSize, LPCWSTR location);

HRESULT UnRegisterClsId(LPCWSTR clsid);
HRESULT UnRegisterOverlay();
HRESULT UnRegisterContextMenu(LPCWSTR location);

HRESULT UninstallAtRegistry();


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        g_Module = hModule;

        g_HookShellFolder.Hook();

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:

        g_HookShellFolder.Unhook();
        break;
    }
    return TRUE;
}

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {

    void* target = nullptr;

    if (IsEqualCLSID(rclsid, CLSID_CNotificationEditContextMenu)) {
        target = new CNotificationEditContextMenu();
    }
    else if (IsEqualCLSID(rclsid, CLSID_CFileMarkContextMenu)) {
        target = new CWinFileMarkContextMenu();
    }
    else {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    CClassFactory* factory = new CClassFactory(reinterpret_cast<IUnknown*>(target));
    HRESULT hr = factory->QueryInterface(riid, ppv);

    factory->Release();

    return hr;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return S_FALSE;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
  return InstallAtRegistry();
}

// 레지스트리에서 Shell Extension 해제
extern "C" HRESULT __stdcall DllUnregisterServer()
{
  return UninstallAtRegistry();
}

HRESULT InstallAtRegistry() {
  wchar_t szModule[MAX_PATH];
  if (!GetModuleFileNameW(g_Module, szModule, MAX_PATH)) {
    return E_FAIL;
  }

  HRESULT hr = RegisterClsId(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}", szModule);
  if (hr != S_OK) {
    return hr;
  }

  hr = RegisterClsId(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC8}", szModule);
  if (hr != S_OK) {
    return hr;
  }

  // 아이콘 오버레이로 등록 (무조건 dll이 올라오도록)
  hr = RegisterOverlay(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}", sizeof(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}"));
  if (hr != S_OK) {
    return hr;
  }

  // 컨텍스트 매뉴 등록
  // * (모든 파일)
  hr = RegisterContextMenu(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC8}", sizeof(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC8}"), L"*");
  if (hr != S_OK) {
    return hr;
  }
  // Dictionary
  hr = RegisterContextMenu(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC8}", sizeof(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC8}"), L"Directory");
  if (hr != S_OK) {
    return hr;
  }

  // Dictionary (빈 공간)
  hr = RegisterContextMenu(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}", sizeof(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}"), L"Directory\\Background");
  if (hr != S_OK) {
    return hr;
  }

  return S_OK;
}

HRESULT RegisterClsId(LPCWSTR clsid, LPCWSTR modulePath) {
  if (clsid == nullptr || modulePath == nullptr) {
    return E_INVALIDARG;
  }

  std::wstring regPath = L"CLSID\\";
  regPath += clsid;

  // HKEY_CLASS_ROOT\CLSID 에 기록
  HKEY hKey = nullptr;
  LONG lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, regPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
  if (lRes != ERROR_SUCCESS) {
    return HRESULT_FROM_WIN32(lRes);
  }

  DWORD modulePathSize = static_cast<DWORD>(wcslen(modulePath) + 1) * sizeof(WCHAR);

  lRes = RegSetValueExW(hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"WinFileMark"), sizeof(L"WinFileMark"));
  if (lRes != ERROR_SUCCESS) {
    RegCloseKey(hKey);

    return HRESULT_FROM_WIN32(lRes);
  }

  RegCloseKey(hKey);

  regPath += L"\\InprocServer32";
  lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, regPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
  if (lRes != ERROR_SUCCESS) {
    return HRESULT_FROM_WIN32(lRes);
  }

  lRes = RegSetValueExW(hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(modulePath), modulePathSize);
  if (lRes != ERROR_SUCCESS) {
    RegCloseKey(hKey);

    return HRESULT_FROM_WIN32(lRes);
  }

  lRes = RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"Apartment"), sizeof(L"Apartment"));
  if (lRes != ERROR_SUCCESS) {
    RegCloseKey(hKey);

    return HRESULT_FROM_WIN32(lRes);
  }

  return S_OK;
}

HRESULT UnRegisterClsId(LPCWSTR clsid) {
    if (clsid == nullptr) {
        return E_INVALIDARG;
    }

    std::wstring regPath = L"CLSID\\";
    regPath += clsid;

    LONG lRes = RegDeleteTreeW(HKEY_CLASSES_ROOT, regPath.c_str());
    return HRESULT_FROM_WIN32(lRes);
}

HRESULT RegisterOverlay(LPCWSTR clsid, DWORD clsidSize) {
  if (clsid == nullptr) {
    return E_INVALIDARG;
  }

  // 아이콘 오버레이에 등록
  HKEY hKey = nullptr;
  LONG lRes = RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\explorer\\ShellIconOverlayIdentifiers\\ winfilemark", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
  if (lRes != ERROR_SUCCESS) {
    return HRESULT_FROM_WIN32(lRes);
  }

  lRes = RegSetValueExW(hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(clsid), clsidSize);
  if (lRes != ERROR_SUCCESS) {
    RegCloseKey(hKey);

    return HRESULT_FROM_WIN32(lRes);
  }

  RegCloseKey(hKey);

  return S_OK;
}

HRESULT UnRegisterOverlay() {
    LONG lRes = RegDeleteTreeW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\explorer\\ShellIconOverlayIdentifiers\\ winfilemark");
    return HRESULT_FROM_WIN32(lRes);
}

HRESULT RegisterContextMenu(LPCWSTR clsid, DWORD clsidSize, LPCWSTR location) {
  if (clsid == nullptr || location == nullptr) {
    return E_INVALIDARG;
  }

  std::wstring regPath = location;
  regPath += L"\\shell\\winfilemark";

  HKEY hKey = nullptr;
  LONG lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, regPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
  if (lRes != ERROR_SUCCESS) {
    return HRESULT_FROM_WIN32(lRes);
  }

  lRes = RegSetValueExW(hKey, L"ExplorerCommandHandler", 0, REG_SZ, reinterpret_cast<const BYTE*>(clsid), clsidSize);
  if (lRes != ERROR_SUCCESS) {
    RegCloseKey(hKey);

    return HRESULT_FROM_WIN32(lRes);
  }

  RegCloseKey(hKey);

  return S_OK;
}

HRESULT UnRegisterContextMenu(LPCWSTR location) {
    std::wstring regPath = location;
    regPath += L"\\shell\\winfilemark";

    LONG lRes = RegDeleteTreeW(HKEY_CLASSES_ROOT, regPath.c_str());

    return HRESULT_FROM_WIN32(lRes);
}



HRESULT UninstallAtRegistry() {
    UnRegisterClsId(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC7}");
    UnRegisterClsId(L"{FEB923B9-686D-4A77-80B6-C9C634FC6CC8}");
    UnRegisterOverlay();

    // 컨텍스트 매뉴 등록 해제
    // * (모든 파일)
    UnRegisterContextMenu(L"*");

    // Dictionary
    UnRegisterContextMenu(L"Directory");

    // Dictionary (빈 공간)
    UnRegisterContextMenu(L"Directory\\Background");

    return S_OK;
}

