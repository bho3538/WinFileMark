#pragma once

class CClassFactory :
	public IClassFactory
{
public:
	CClassFactory(IUnknown* target);
	~CClassFactory();

	// IUnknown
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	// IClassFactory
	HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
	HRESULT __stdcall LockServer(BOOL fLock);

private:
	IUnknown* _target;

	LONG _refCount;
	LONG _serverLocks;
};

