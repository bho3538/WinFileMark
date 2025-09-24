#pragma once
class CMemoryReader
{
public:
	CMemoryReader(BYTE* data, DWORD dataLen);
	~CMemoryReader();

	const BYTE* Read(DWORD len);
	DWORD ReadDWORD();
	LPCWSTR ReadWSTR();

	void ResetOffset();

private:

	bool CanRead(DWORD bytes);

	BYTE* _data;
	size_t _dataLen;
	size_t _offset;
};

