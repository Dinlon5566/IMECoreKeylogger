#pragma once

#include <string>
#include <Windows.h>
bool isUserAdmin();
bool getDLLPath(wchar_t* DLLPath);
void debug_hold();
bool GetProcessNameByPID(DWORD pid, std::wstring& processName);