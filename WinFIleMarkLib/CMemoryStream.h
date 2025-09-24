#pragma once
class CMemoryStream
{
public:
	CMemoryStream();
	~CMemoryStream();

	BOOL Initialize(DWORD initialSize);

	BOOL WriteDWORD(DWORD data);
	BOOL WriteWSTR(LPCWSTR data);
	BOOL Write(BYTE* data, DWORD len);

	BYTE* GetRawBuffer();
	DWORD GetDataLength();

	void Detech();

private:
	BOOL _Realloc(DWORD addSize);

	BYTE* _data;
	DWORD _offset;
	DWORD _size;
	DWORD _maxsize;
};

