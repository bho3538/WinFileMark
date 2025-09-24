#pragma once
class DuplicateRunning
{
private:
	DuplicateRunning();
	~DuplicateRunning();

public:

	static DuplicateRunning* CreateInstance();
	bool Check();
	void ShowPreviousRunning();
	void Release();

private:
	HANDLE _mutex;
};

