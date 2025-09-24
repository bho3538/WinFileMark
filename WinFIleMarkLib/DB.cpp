#include "stdafx.h"
#include "DB.h"
#include "CStringUtils.h"

#include <SQLiteCpp/SQLiteCpp.h>

#define DB_FILENAME					L".winfilemark.db"
#define DB_FILENAME_WITH_SEPERATOR	L"\\.winfilemark.db"

DB::DB(LPCWSTR folderPath) :
	DB(std::wstring(folderPath))
{
}

DB::DB(const std::wstring& folderPath) {
	_dbPath.reserve(MAX_PATH);
	_dbPath.append(folderPath);
	_dbPath.append(DB_FILENAME_WITH_SEPERATOR);

	_dbPathUtf8 = CStringUtils::Utf16ToUtf8(_dbPath);
}

bool DB::IsCurrentFolderHasDbFile() {

	return GetFileAttributesW(_dbPath.c_str()) != INVALID_FILE_ATTRIBUTES ? true : false;
}


bool DB::GetInfobarMessage(PTBINFOBARDATA pInfobarData) {
	if (pInfobarData == nullptr) {
		return false;
	}

	if (IsCurrentFolderHasDbFile() == false) {
		return false;
	}

	// db 에서 해당 폴더의 infobar 정보를 읽기
	try {
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READONLY);

		if (db.tableExists("tbInfoBar") == false) {
			return false;
		}

		SQLite::Statement query(db, "SELECT infobarmsg, weburl, flags FROM tbInfoBar");

		if (query.executeStep() == false) {
			return false;
		}

		pInfobarData->infobarMessage = CStringUtils::Utf8ToUtf16(query.getColumn(0));
		if (pInfobarData->infobarMessage.empty() == true) {
			return false;
		}

		pInfobarData->url = CStringUtils::Utf8ToUtf16(query.getColumn(1));
		pInfobarData->flags = query.getColumn(2);

	}
	catch (SQLite::Exception&) {

		return false;
	}

	return true;
}

int DB::GetFileMarkStatus(const std::wstring& fileName) {

	int status = 0;

	if (IsCurrentFolderHasDbFile() == false) {
		return status;
	}

	// db 에서 해당 폴더의 infobar 정보를 읽기
	try {
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READONLY);

		if (db.tableExists("tbFileMarkInfo") == false) {
			return status;
		}

		SQLite::Statement query(db, "SELECT iconstatus FROM tbFileMarkInfo WHERE filename = ?");

		query.bind(1, CStringUtils::Utf16ToUtf8(fileName));

		if (query.executeStep() == false) {
			return status;
		}

		status = query.getColumn(0);
	}
	catch (SQLite::Exception&) {

		return status;
	}

	return status;
}

void DB::SetFileMarkStatus(const std::wstring& fileName, int iconStatus, int currentIconStatus) {

	if (IsCurrentFolderHasDbFile() == false) {
		// create db file

		if (CreateDbFileAndInitializeIt() == false) {
			return;
		}
	}

	try {
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READWRITE);

		if (db.tableExists("tbFileMarkInfo") == false) {
			return;
		}

		if (currentIconStatus == 0) {
			// set
			SQLite::Statement query(db, "INSERT INTO tbFileMarkInfo VALUES(?,?,?)");
			query.bind(1, CStringUtils::Utf16ToUtf8(fileName));
			query.bind(2, iconStatus);
			query.bind(3, 0);
			query.exec();
		}
		else {
			currentIconStatus |= iconStatus;

			SQLite::Statement query(db, "UPDATE tbFileMarkInfo SET iconstatus = ? WHERE filename = ?");
			query.bind(1, currentIconStatus);
			query.bind(2, CStringUtils::Utf16ToUtf8(fileName));
			query.exec();
		}
	}
	catch (SQLite::Exception&) {

	}
	
}

void DB::UnsetFileMarkStatus(const std::wstring& fileName, int iconStatus, int currentIconStatus) {

	try {
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READWRITE);

		if (db.tableExists("tbFileMarkInfo") == false) {
			return;
		}

		currentIconStatus &= ~iconStatus;

		if (currentIconStatus == 0) {
			// set
			SQLite::Statement query(db, "DELETE FROM tbFileMarkInfo WHERE filename = ?");
			query.bind(1, CStringUtils::Utf16ToUtf8(fileName));
			query.exec();
		}
		else {
			SQLite::Statement query(db, "UPDATE tbFileMarkInfo SET iconstatus = ? WHERE filename = ?");
			query.bind(1, currentIconStatus);
			query.bind(2, CStringUtils::Utf16ToUtf8(fileName));
			query.exec();
		}
	}
	catch (SQLite::Exception&) {

	}
}

bool DB::CreateDbFileAndInitializeIt() {
	try {
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

		if (db.tableExists("tbFileMarkInfo") == false) {
			db.exec("CREATE TABLE tbFileMarkInfo (filename TEXT primary key, iconstatus INTEGER DEFAULT 0, flags INTEGER DEFAULT 0)");
		}

		// DB 파일 숨기기
		SetFileAttributesW(_dbPath.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL);

		return true;
	}
	catch (SQLite::Exception&) {

		return false;
	}
}

void DB::RenameFileInfo(const std::wstring& oldFileName, const std::wstring& newFileName) {
	try {
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READWRITE);

		if (db.tableExists("tbFileMarkInfo") == false) {
			return;
		}

		SQLite::Statement query(db, "UPDATE tbFileMarkInfo SET filename = ? WHERE filename = ?");
		query.bind(1, CStringUtils::Utf16ToUtf8(newFileName));
		query.bind(2, CStringUtils::Utf16ToUtf8(oldFileName));
		query.exec();
	}
	catch (SQLite::Exception& e) {
		OutputDebugStringA(e.getErrorStr());
	}
}

void DB::DeleteFileInfo(const std::wstring& fileName) {
	try {
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READWRITE);

		if (db.tableExists("tbFileMarkInfo") == false) {
			return;
		}

		SQLite::Statement query(db, "DELETE FROM tbFileMarkInfo WHERE filename = ?");
		query.bind(1, CStringUtils::Utf16ToUtf8(fileName));
		query.exec();
	}
	catch (SQLite::Exception&) {

	}
}

void DB::FindFilesMarkStatus(CResponseIconStatus* storage, DWORD offset, DWORD length)
{
	if (storage == nullptr)
	{
		return;
	}

	if (IsCurrentFolderHasDbFile() == false)
	{
		return;
	}

	try
	{
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READONLY);

		if (db.tableExists("tbFileMarkInfo") == false) 
		{
			return;
		}

		SQLite::Statement query(db, "SELECT filename, iconstatus FROM tbFileMarkInfo");

		while (query.executeStep() == true) 
		{
			std::string fileName = query.getColumn(0);
			int status = query.getColumn(1);

			storage->AddItem(CStringUtils::Utf8ToUtf16(fileName), status);
		}
	}
	catch (SQLite::Exception&) 
	{

	}
}

void DB::SetFilesMarkStatus(CRequestSetIconStatus* req)
{
	if (req == nullptr)
	{
		return;
	}

	if (IsCurrentFolderHasDbFile() == false)
	{
		if (CreateDbFileAndInitializeIt() == false) 
		{
			return;
		}
	}

	const auto& items = req->GetItems();
	DWORD itemsCount = items.size();

	try
	{
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READWRITE);

		if (db.tableExists("tbFileMarkInfo") == false)
		{
			return;
		}

		const std::string sql = R"(
			INSERT INTO tbFileMarkInfo (filename, iconstatus) VALUES (?1, ?2)
				ON CONFLICT(filename) DO UPDATE SET
					iconstatus = CASE
						WHEN (tbFileMarkInfo.iconstatus & ?2) = 0 THEN tbFileMarkInfo.iconstatus | ?2
          END;
			)";

		SQLite::Transaction transaction(db);

		SQLite::Statement query(db, sql);

		for (DWORD i = 0; i < itemsCount; i++)
		{
			query.bind(1, CStringUtils::Utf16ToUtf8(items[i]));
			query.bind(2, req->StatusIconFlags);

			query.exec();
			query.reset();
		}

		transaction.commit();
	}
	catch (SQLite::Exception&)
	{
	}
}

void DB::UnSetFilesMarkStatus(CRequestSetIconStatus* req)
{
	if (req == nullptr)
	{
		return;
	}

	if (IsCurrentFolderHasDbFile() == false)
	{
		return;
	}

	const auto& items = req->GetItems();
	DWORD itemsCount = items.size();

	try
	{
		SQLite::Database db(_dbPathUtf8, SQLite::OPEN_READWRITE);

		if (db.tableExists("tbFileMarkInfo") == false)
		{
			return;
		}

		const std::string sql = "UPDATE tbFileMarkInfo SET iconstatus = iconstatus & ~?1 WHERE filename = ?2 AND (iconstatus & ?1) != 0";

		SQLite::Transaction transaction(db);

		SQLite::Statement query(db, sql);

		for (DWORD i = 0; i < itemsCount; i++)
		{
			query.bind(1, req->StatusIconFlags);
			query.bind(2, CStringUtils::Utf16ToUtf8(items[i]));

			query.exec();
			query.reset();
		}

		transaction.commit();
	}
	catch (SQLite::Exception&)
	{
	}
}