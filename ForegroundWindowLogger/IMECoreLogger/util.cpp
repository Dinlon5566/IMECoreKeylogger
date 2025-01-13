
#include "pch.h"
#include "util.h"
#include <Windows.h>
#include "logger.h"

bool isUserAdmin() {	// from MDMZ_Book.pdf
	bool isElevated = false;
	HANDLE token;
	TOKEN_ELEVATION elev;
	DWORD size;
	if (OpenProcessToken(GetCurrentProcess(),
		TOKEN_QUERY, &token)) {
		if (GetTokenInformation(token, TokenElevation,
			&elev, sizeof(elev), &size)) {
			isElevated = elev.TokenIsElevated;
		}
	}
	if (token) {
		CloseHandle(token);
		token = NULL;
	}
	return isElevated;
}


bool getDLLPath(wchar_t* DLLPath) {
	HMODULE hModule = NULL;
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&getDLLPath, &hModule))
	{
		if (GetModuleFileName(hModule, DLLPath, MAX_PATH) > 0)
		{
			return true;
		}
	}
	return false;
}

bool GetProcessNameByPID(DWORD pid, std::wstring& processName)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (!hProcess) {
		return false;
	}
	wchar_t pathBuffer[MAX_PATH] = { 0 };
	DWORD bufSize = MAX_PATH;
	if (!QueryFullProcessImageNameW(hProcess, 0, pathBuffer, &bufSize)) {
		CloseHandle(hProcess);
		return false;
	}
	CloseHandle(hProcess);

	std::wstring fullPath(pathBuffer);
	size_t pos = fullPath.find_last_of(L"\\/");
	if (pos != std::wstring::npos) {
		processName = fullPath.substr(pos + 1);
	}
	else {
		processName = fullPath;
	}

	return true;
}

// Do noting
void debug_hold() {
	while (true) {
		Sleep(1000);
	}
}
