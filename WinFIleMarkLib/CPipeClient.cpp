#include "stdafx.h"
#include "CDataModelBase.h"
#include "CPipeClient.h"

#include <string>

CPipeClient::CPipeClient()
  : _hPipe(INVALID_HANDLE_VALUE)
{}

CPipeClient::~CPipeClient()
{
  Close();
}

void CPipeClient::Close()
{
  if (_hPipe != INVALID_HANDLE_VALUE)
  {
    ::CloseHandle(_hPipe);
    _hPipe = INVALID_HANDLE_VALUE;
  }
}

DWORD CPipeClient::Connect(LPCWSTR pipeServerName)
{
  DWORD errCode = 0;
  if (_hPipe != INVALID_HANDLE_VALUE)
  {
    return 0;
  }

  // 파이프 연결 문자열
  std::wstring connName = L"\\\\.\\pipe\\" + std::wstring(pipeServerName);

  // 파이프 연결
  _hPipe = ::CreateFileW(connName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
  if (_hPipe == INVALID_HANDLE_VALUE)
  {
    errCode = ::GetLastError();
  }

  return errCode;
}

BOOL CPipeClient::IsConnected()
{
  if (_hPipe != INVALID_HANDLE_VALUE)
  {
    return TRUE;
  }
  return FALSE;
}

DWORD CPipeClient::Send(const BYTE* data, DWORD dataLen)
{
  DWORD sendLen = 0;

  // 전송하려는 데이터의 길이를 먼저 전송
  if (!::WriteFile(_hPipe, reinterpret_cast<const BYTE*>(&dataLen), sizeof(DWORD), &sendLen, nullptr) || sendLen != sizeof(DWORD))
  {
    return 0;
  }

  // 데이터 전송
  if (!::WriteFile(_hPipe, data, dataLen, &sendLen, nullptr) || sendLen != dataLen)
  {
    return 0;
  }

  return sendLen;
}

DWORD CPipeClient::Receive(BYTE** data)
{
  DWORD receiveLen = 0;
  DWORD dataLen = 0;

  // 수신 데이터의 길이를 먼저 받기
  if (!::ReadFile(_hPipe, reinterpret_cast<BYTE*>(&dataLen), sizeof(DWORD), &receiveLen, nullptr) || receiveLen != sizeof(DWORD))
  {
    return 0;
  }

  // 데이터를 수신받을 버퍼를 할당
  *data = reinterpret_cast<BYTE*>(::CoTaskMemAlloc(dataLen));
  if (*data == nullptr)
  {
    return 0;
  }

  // 데이터의 길이만큼 데이터 읽기
  if (!::ReadFile(_hPipe, *data, dataLen, &receiveLen, nullptr) || receiveLen != dataLen)
  {
    ::CoTaskMemFree(*data);
    *data = nullptr;

    return 0;
  }

  return dataLen;
}

DWORD CPipeClient::Send(CDataModelBase* data)
{
  if (data == nullptr)
  {
    return 0;
  }

  CMemoryStream* ms = nullptr;
  data->Serialize(&ms);

  DWORD sendLen = Send(ms->GetRawBuffer(), ms->GetDataLength());

  delete ms;

  return sendLen;
}

DWORD CPipeClient::Receive(CDataModelBase* data)
{
  if (data == nullptr)
  {
    return 0;
  }

  DWORD recvDataLen = 0;
  BYTE* recvData = nullptr;
  recvDataLen = Receive(&recvData);

  if (recvDataLen == 0)
  {
    return 0;
  }

  CMemoryReader reader(recvData, recvDataLen);
  data->Deserialize(&reader);

  ::CoTaskMemFree(recvData);

  return recvDataLen;
}
