#pragma once

#include <ShlObj_core.h>

class CHookShellFolder
{
public:
	void Hook();
	void Unhook();

	static HRESULT SetIconColumnInfo(PROPVARIANT* pv, int flags);

private:


};

