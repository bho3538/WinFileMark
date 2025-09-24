#include "stdafx.h"
#include "CClassFactory.h"
#include "CNotificationEditContextMenu.h"

#include <new>


CClassFactory::CClassFactory(IUnknown* target) :
	_refCount(0),
	_serverLocks(0),
	_target(target)
{
	AddRef();
}

CClassFactory::~CClassFactory() {
	if (_target != nullptr) {
		_target->Release();
	}
}


HRESULT CClassFactory::QueryInterface(REFIID riid, void** ppv) {
	if (ppv == nullptr) {
		return E_POINTER;
	}

	*ppv = nullptr;

	if (riid == IID_IUnknown) {
		*ppv = static_cast<IUnknown*>(this);
	}
	else if (riid == IID_IClassFactory) {
		*ppv = static_cast<IClassFactory*>(this);
	}

	if (*ppv != nullptr) {
		this->AddRef();
		return S_OK;
	}
	else {
		return E_NOINTERFACE;
	}
}

ULONG CClassFactory::AddRef() {
	return InterlockedIncrement(&_refCount);
}

ULONG CClassFactory::Release() {
	ULONG refCount = InterlockedDecrement(&_refCount);
	if (refCount == 0) {
		delete this;
	}

	return refCount;
}

HRESULT CClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) {
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	if (pUnkOuter != nullptr) {
		return CLASS_E_NOAGGREGATION;
	}

	HRESULT hr = _target->QueryInterface(riid, ppvObject);
	if (FAILED(hr)) {
		*ppvObject = nullptr;
	}

	return hr;
}

HRESULT CClassFactory::LockServer(BOOL fLock) {
	if (fLock) {
		InterlockedIncrement(&_serverLocks);
	}
	else {
		InterlockedDecrement(&_serverLocks);
	}

	return S_OK;
}