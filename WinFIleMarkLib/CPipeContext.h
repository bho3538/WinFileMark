#pragma once

#include "stdafx.h"

enum class CPipeState
{
	WAIT,
	READ_HEADER,
	READ,
	WRITE_HEADER,
	WRITE
};

struct CPipeContext
{
	OVERLAPPED ov;
	HANDLE hPipe;

	// recv buffer
	BYTE* buffer;
	DWORD dataLen;

	// send buffer
	BYTE* sendBuffer;
	DWORD sendDataLen;

	// pipe state
	CPipeState state;
};
