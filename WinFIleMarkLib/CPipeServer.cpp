#include "stdafx.h"
#include "CPipeServer.h"

#define _PIPE_RECV_BUFFER_SIZE 4096

CPipeServer::CPipeServer()
  : _hIocp(nullptr)
{
}

CPipeServer::~CPipeServer()
{
}

DWORD CPipeServer::Start(LPCWSTR pipeServerName)
{
  _pipeServerName = L"\\\\.\\pipe\\" + std::wstring(pipeServerName);

  // IOCP 생성
  _hIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
  if (_hIocp == nullptr)
  {
    return ::GetLastError();
  }

  for (int i = 0; i < 4; ++i)
  {
    HANDLE hWorker = ::CreateThread(nullptr, 0, CPipeServer::WorkerThreads, this, 0, nullptr);
    if (hWorker != nullptr)
    {
      _workerThreads.push_back(hWorker);
    }
  }

  for (int i = 0; i < 8; ++i)
  {
    CPipeContext* ctx = reinterpret_cast<CPipeContext*>(::malloc(sizeof(CPipeContext)));
    if (ctx == nullptr)
    {
      return ERROR_NOT_ENOUGH_MEMORY;
    }

    ::ZeroMemory(ctx, sizeof(CPipeContext));

    // 연결 대기 상태로 설정
    ctx->state = CPipeState::WAIT;

    DWORD err = CreateHandleAndWaitConnection(ctx);
    if (err != NO_ERROR)
    {
      return err;
    }

    _ctxs.push_back(ctx);
  }

  return NO_ERROR;
}

DWORD CPipeServer::CreateHandleAndWaitConnection(CPipeContext* ctx)
{
  if (ctx == nullptr)
  {
    return ERROR_INVALID_PARAMETER;
  }

  // 파이프 서버 생성
  //ctx->hPipe = ::CreateNamedPipeW(_pipeServerName.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 64 * 1024, 64 * 1024, 0, nullptr);

  ctx->hPipe = ::CreateNamedPipeW(
    _pipeServerName.c_str(),
    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
    PIPE_TYPE_BYTE | PIPE_WAIT,
    PIPE_UNLIMITED_INSTANCES,
    64 * 1024,
    64 * 1024,
    0,
    nullptr
  );

  if (ctx->hPipe == INVALID_HANDLE_VALUE)
  {
    return ::GetLastError();
  }

  // IOCP에 파이프 서버 핸들 등록
  if (::CreateIoCompletionPort(ctx->hPipe, _hIocp, reinterpret_cast<ULONG_PTR>(ctx), 0) == nullptr)
  {
    return ::GetLastError();
  }

  // 파이프 클라이언트의 연결 대기 (비동기)
  if (::ConnectNamedPipe(ctx->hPipe, &ctx->ov) == FALSE)
  {
    DWORD err = ::GetLastError();
    if (err == ERROR_PIPE_CONNECTED)
    {
      ::PostQueuedCompletionStatus(_hIocp, 0, reinterpret_cast<ULONG_PTR>(ctx), &ctx->ov);
    }
  }

  return NO_ERROR;
}

DWORD WINAPI CPipeServer::WorkerThreads(PVOID args)
{
  CPipeServer* server = reinterpret_cast<CPipeServer*>(args);
  DWORD bytes = 0;
  ULONG_PTR key = 0;
  LPOVERLAPPED pov = nullptr;

  while (true)
  {
    if (::GetQueuedCompletionStatus(server->_hIocp, &bytes, &key, &pov, INFINITE) == FALSE)
    {
      // 파이프가 예상치 못하게 연결 종료된 경우
      if (key != 0)
      {
        CPipeContext* ctx = reinterpret_cast<CPipeContext*>(key);
        server->DisconnectAndReset(ctx);
        continue;
      }
    }

    if (key == 0)
    {
      break;
    }

    CPipeContext* ctx = reinterpret_cast<CPipeContext*>(key);
    switch (ctx->state)
    {
    case CPipeState::WAIT:
    {
      // header (데이터 크기) 를 read
      if (server->PrepareReadData(ctx) != NO_ERROR)
      {
        server->DisconnectAndReset(ctx);
      }

      break;
    }
    case CPipeState::READ_HEADER:
    {
      // 데이터를 read
      if (server->ReadData(ctx) != NO_ERROR)
      {
        server->DisconnectAndReset(ctx);
      }

      break;
    }
    case CPipeState::READ:
    {
      // 나머지 데이터를 다 읽은 경우
      if (ctx->dataLen < sizeof(DWORD))
      {
        // 최소한의 데이터(패킷 타입) 크기 이하를 수신한 경우
        server->DisconnectAndReset(ctx);
        
        break;
      }

      CMemoryStream* response = nullptr;
      CMemoryReader request(ctx->buffer, ctx->dataLen);

      // 핸들러 호출
      server->OnDataReceived(&request, &response);

      // 수신 버퍼 해제
      server->FreeRecvBuffer(ctx, FALSE);

      if (response == nullptr)
      {
        // 수신 대기 (응답할 데이터가 없는 경우)
        if (server->PrepareReadData(ctx) != NO_ERROR)
        {
          server->DisconnectAndReset(ctx);
        }
      }
      else
      {
        // 응답 전송
        if (server->PrepareWriteData(ctx, response->GetRawBuffer(), response->GetDataLength()) != NO_ERROR)
        {
          server->DisconnectAndReset(ctx);
        }

        // 응답을 전송하고 메모리를 해제하도록 조치
        response->Detech();
      }

      break;
    }
    case CPipeState::WRITE_HEADER:
    {
      // 데이터를 write
      if (server->WriteData(ctx) != NO_ERROR)
      {
        server->DisconnectAndReset(ctx);
      }

      break;
    }
    case CPipeState::WRITE:
    {
      // 전송한 응답 버퍼를 해제
      server->FreeSendBuffer(ctx);

      // 응답값을 다 전송한 경우 다시 읽기 대기 상태로 전환
      if (server->PrepareReadData(ctx) != NO_ERROR)
      {
        server->DisconnectAndReset(ctx);
      }

      break;
    }
    }
  }

  return 0;
}

DWORD CPipeServer::PrepareReadData(CPipeContext* ctx)
{
  // 데이터의 크기를 읽기
  if (::ReadFile(ctx->hPipe, &ctx->dataLen, sizeof(DWORD), nullptr, &ctx->ov) == FALSE)
  {
    DWORD err = ::GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      return err;
    }
  }

  // header 까지 읽었다고 상태 표기
  ctx->state = CPipeState::READ_HEADER;
  return NO_ERROR;
}

DWORD CPipeServer::ReadData(CPipeContext* ctx)
{
  if (ctx->dataLen == 0)
  {
    return ERROR_BROKEN_PIPE;
  }

  // 수신 버퍼를 할당 (데이터의 크기만큼)
  SetRecvBuffer(ctx, ctx->dataLen);
  if (ctx->buffer == nullptr)
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  ctx->state = CPipeState::READ;

  if (::ReadFile(ctx->hPipe, ctx->buffer, ctx->dataLen, nullptr, &ctx->ov) == FALSE)
  {
    DWORD err = ::GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      return err;
    }
  }

  return NO_ERROR;
}

DWORD CPipeServer::PrepareWriteData(CPipeContext* ctx, BYTE* data, DWORD dataLen)
{
  if (data == nullptr || dataLen == 0)
  {
    return ERROR_INVALID_PARAMETER;
  }

  ctx->sendBuffer = data;
  ctx->sendDataLen = dataLen;
  ctx->state = CPipeState::WRITE_HEADER;

  // 데이터의 길이를 먼저 전송
  if (::WriteFile(ctx->hPipe, &dataLen, sizeof(DWORD), nullptr, &ctx->ov) == FALSE)
  {
    DWORD err = ::GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      return err;
    }
  }

  return NO_ERROR;
}

DWORD CPipeServer::WriteData(CPipeContext* ctx)
{
  if (ctx->sendBuffer == nullptr || ctx->sendDataLen == 0)
  {
    return ERROR_INVALID_PARAMETER;
  }

  ctx->state = CPipeState::WRITE;

  if (::WriteFile(ctx->hPipe, ctx->sendBuffer, ctx->sendDataLen, nullptr, &ctx->ov) == FALSE)
  {
    DWORD err = ::GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      return err;
    }
  }

  return NO_ERROR;
}

DWORD CPipeServer::DisconnectAndReset(CPipeContext* ctx)
{
  // 진행중인 pipe io 취소
  ::CancelIo(ctx->hPipe);

  // 파이프 연결 해제
  ::DisconnectNamedPipe(ctx->hPipe);

  // 파이프 핸들 닫기
  ::CloseHandle(ctx->hPipe);

  // 내부 버퍼 초기화
  FreeRecvBuffer(ctx, FALSE);
  FreeSendBuffer(ctx);

  // 내부 상태 초기화 (overlapped 등)
  ::ZeroMemory(ctx, sizeof(CPipeContext));

  // 다시 대기 상태로 변경
  ctx->state = CPipeState::WAIT;

  // 다시 파이프 서버를 준비
  CreateHandleAndWaitConnection(ctx);

  return NO_ERROR;
}

void CPipeServer::FreeRecvBuffer(CPipeContext* ctx, BOOL /*cleanup*/)
{
  ::CoTaskMemFree(ctx->buffer);

  ctx->buffer = nullptr;
  ctx->dataLen = 0;
}

void CPipeServer::SetRecvBuffer(CPipeContext* ctx, DWORD dataLen)
{
  ctx->buffer = reinterpret_cast<BYTE*>(::CoTaskMemAlloc(dataLen));
  ctx->dataLen = dataLen;
}

void CPipeServer::FreeSendBuffer(CPipeContext* ctx)
{
  ::CoTaskMemFree(ctx->sendBuffer);

  ctx->sendBuffer = nullptr;
  ctx->sendDataLen = 0;
}
