#include "stdafx.h"
#include "CMemoryReader.h"

CMemoryReader::CMemoryReader(BYTE* data, DWORD dataLen) :
	_data(data),
	_dataLen(dataLen),
	_offset(0)
{}

CMemoryReader::~CMemoryReader()
{}

DWORD CMemoryReader::ReadDWORD()
{
	if (CanRead(sizeof(DWORD)) == false)
	{
		return 0;
	}

	DWORD data = *reinterpret_cast<DWORD*>(_data + _offset);
	_offset += sizeof(DWORD);

	return data;
}

LPCWSTR CMemoryReader::ReadWSTR()
{
	// 문자열의 길이를 구하기
	DWORD cch = ReadDWORD();
	if (cch >= MAXINT - 1)
	{
		return nullptr;
	}

	DWORD size = (cch + 1) * sizeof(WCHAR);
	if (CanRead(size) == false)
	{
		return nullptr;
	}

	LPCWSTR data = reinterpret_cast<LPCWSTR>(_data + _offset);

	// 문자열의 길이 + NULL 만큼 offset 을 이동
	_offset += ((cch + sizeof(CHAR)) * sizeof(WCHAR));

	return data;
}

const BYTE* CMemoryReader::Read(DWORD len)
{
	if (CanRead(len) == false)
	{
		return nullptr;
	}

	_offset += len;
	return _data + _offset;
}

void CMemoryReader::ResetOffset()
{
	_offset = 0;
}

bool CMemoryReader::CanRead(DWORD bytes)
{
	// offset이 dataLen을 넘었거나, 요청 바이트가 남은 공간보다 크면 읽기 실패 처리
	if (_offset > _dataLen || bytes > (_dataLen - _offset))
	{
		return false;
	}

	return true;
}