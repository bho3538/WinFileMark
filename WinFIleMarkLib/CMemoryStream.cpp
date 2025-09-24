#include "stdafx.h"
#include "CMemoryStream.h"

CMemoryStream::CMemoryStream() :
	_data(nullptr),
	_offset(0),
	_size(0),
	_maxsize(0)
{}

CMemoryStream::~CMemoryStream()
{
	if (_data)
	{
		::CoTaskMemFree(_data);
	}
}

BOOL CMemoryStream::Initialize(DWORD initialSize)
{
	// 버퍼 할당
	_data = reinterpret_cast<BYTE*>(::CoTaskMemAlloc(initialSize));
	if (_data == nullptr)
	{
		_maxsize = 0;

		return FALSE;
	}
	else
	{
		_maxsize = initialSize;

		return TRUE;
	}
}

BOOL CMemoryStream::WriteDWORD(DWORD data)
{
	return Write(reinterpret_cast<BYTE*>(&data), sizeof(DWORD));
}

BOOL CMemoryStream::WriteWSTR(LPCWSTR data)
{
	if (data == nullptr)
	{
		return FALSE;
	}

	DWORD len = ::wcslen(data);

	// 문자열의 길이를 write
	if (Write(reinterpret_cast<BYTE*>(&len), sizeof(DWORD)) == FALSE)
	{
		return FALSE;
	}

	// 문자열을 write (null 포함)
	return Write(reinterpret_cast<BYTE*>(const_cast<LPWSTR>(data)), (len + 1) * sizeof(WCHAR));
}

BOOL CMemoryStream::Write(BYTE* data, DWORD len)
{
	if (_size + len > _maxsize)
	{
		// 버퍼에 충분한 공간이 없는 경우 재할당
		if (_Realloc(len) == FALSE)
		{
			return FALSE;
		}
	}

	::memcpy((_data + _offset), data, len);

	_size += len;
	_offset += len;
	return TRUE;
}

BOOL CMemoryStream::_Realloc(DWORD addSize)
{
	DWORD maxBufferLen = _maxsize + addSize;

	LPVOID buffer = ::CoTaskMemRealloc(_data, maxBufferLen);
	if (buffer != nullptr)
	{
		_data = reinterpret_cast<BYTE*>(buffer);
		_maxsize = maxBufferLen;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BYTE* CMemoryStream::GetRawBuffer()
{
	return this->_data;
}

DWORD CMemoryStream::GetDataLength()
{
	return this->_size;
}

void CMemoryStream::Detech()
{
	_data = nullptr;
	_size = 0;
	_offset = 0;
	_maxsize = 0;
}
