#pragma once

#include "../WinFIleMarkLib/CPipeServer.h"
#include "../WinFIleMarkLib/CRequestIconStatus.h"
#include "../WinFIleMarkLib/CResponseIconStatus.h"
#include "../WinFIleMarkLib/CRequestSetIconStatus.h"
#include "../WinFIleMarkLib/CRequestRenameFile.h"
#include "../WinFIleMarkLib/CRequestDeleteFile.h"
#include "../WinFIleMarkLib/CRequestMoveFile.h"
#include "../WinFIleMarkLib/CResponseMoveFile.h"

class WinFIleMarkServer : public CPipeServer
{
public:
	WinFIleMarkServer() = default;
	~WinFIleMarkServer() = default;

protected:
	void OnDataReceived(CMemoryReader* reader, CMemoryStream** ppResponse);

private:
	void HandleRequestIconStatus(CRequestIconStatus* req, CResponseIconStatus* res);
	void HandleRequestSetIconStatus(CRequestSetIconStatus* req);
	void HandleRequestRenameFile(CRequestRenameFile* req);
	void HandleRequestDeleteFile(CRequestDeleteFile* req);
	void HandleRequestMoveFile(CRequestMoveFile* req, CResponseMoveFile* res);
};

