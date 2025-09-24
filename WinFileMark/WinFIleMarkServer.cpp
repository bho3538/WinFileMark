#include "pch.h"
#include "WinFIleMarkServer.h"
#include "../WinFIleMarkLib/DB.h"

void WinFIleMarkServer::OnDataReceived(CMemoryReader* reader, CMemoryStream** ppResponse)
{
	if (reader == nullptr || ppResponse == nullptr)
	{
		return;
	}

	// 데이터 타입을 먼저 읽기
	CDataType dataType = static_cast<CDataType>(reader->ReadDWORD());
	reader->ResetOffset();

	switch (dataType)
	{
		case CDataType::REQUEST_ICONSTATUS:
		{
			// request 해석
			CRequestIconStatus req;
			req.Deserialize(reader);

			// response
			CResponseIconStatus res;

			// request 를 처리 후 response 를 구하기
			HandleRequestIconStatus(&req, &res);

			res.Serialize(ppResponse);

			break;
		}
		case CDataType::REQUEST_SETICONSTATUS:
		{
			// request 해석
			CRequestSetIconStatus req;
			req.Deserialize(reader);
			
			HandleRequestSetIconStatus(&req);

			break;
		}
		case CDataType::REQUEST_RENAMEFILE:
		{
			CRequestRenameFile req;
			req.Deserialize(reader);

			HandleRequestRenameFile(&req);

			break;
		}
		case CDataType::REQUEST_DELETEFILE:
		{
			CRequestDeleteFile req;
			req.Deserialize(reader);

			HandleRequestDeleteFile(&req);

			break;
		}
		case CDataType::REQUEST_MOVEFILE:
		{
			CRequestMoveFile req;
			req.Deserialize(reader);

			CResponseMoveFile res;

			HandleRequestMoveFile(&req, &res);

			res.Serialize(ppResponse);

			break;
		}
	}
}

void WinFIleMarkServer::HandleRequestIconStatus(CRequestIconStatus* req, CResponseIconStatus* res)
{
	// path 가 유효한지 (접근 가능한지) 확인
	if (::GetFileAttributesW(req->Path.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	// 해당 폴더의 DB 파일을 open
	DB db(req->Path);

	// db 에서 값 조회
	db.FindFilesMarkStatus(res, req->Offset, req->Length);
}

void WinFIleMarkServer::HandleRequestSetIconStatus(CRequestSetIconStatus* req)
{
	// path 가 유효한지 (접근 가능한지) 확인
	if (::GetFileAttributesW(req->Path.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	// 해당 폴더의 DB 파일을 open
	DB db(req->Path);

	if (req->UnsetMode == false)
	{
		db.SetFilesMarkStatus(req);
	}
	else
	{
		db.UnSetFilesMarkStatus(req);
	}
}

void WinFIleMarkServer::HandleRequestRenameFile(CRequestRenameFile* req)
{
	// path 가 유효한지 (접근 가능한지) 확인
	if (::GetFileAttributesW(req->Path.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	// 해당 폴더의 DB 파일을 open
	DB db(req->Path);

	db.RenameFileInfo(req->OldFileName, req->NewFileName);
}

void WinFIleMarkServer::HandleRequestDeleteFile(CRequestDeleteFile* req)
{
	// path 가 유효한지 (접근 가능한지) 확인
	if (::GetFileAttributesW(req->Path.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	// 해당 폴더의 DB 파일을 open
	DB db(req->Path);

	db.DeleteFileInfo(req->FileName);
}

void WinFIleMarkServer::HandleRequestMoveFile(CRequestMoveFile* req, CResponseMoveFile* res)
{
	// path 가 유효한지 (접근 가능한지) 확인
	if (::GetFileAttributesW(req->OldPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		// 이전 폴더에 접근할 수 없으면 파일이 가지고 있던 status mark 를 가져올 방법이 없음
		return;
	}

	// 해당 폴더의 DB 파일을 open
	DB db(req->OldPath);
	int status = db.GetFileMarkStatus(req->OldFileName);

	if (status > 0)
	{
		DB destDB(req->NewPath);

		destDB.SetFileMarkStatus(req->NewFileName, status, 0);
	}

	db.DeleteFileInfo(req->OldFileName);
}