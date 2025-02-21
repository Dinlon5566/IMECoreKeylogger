#include <windows.h>
#include "framework.h"
#include <imm.h>
#pragma comment(lib, "imm32.lib")
#define MAX_LOADSTRING 100
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    static TCHAR szAppName[] = TEXT("IMECharDemo");
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    // Register window class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szAppName;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, TEXT("Failed to register window class!"), szAppName, MB_ICONERROR);
        return 0;
    }

    // Create window
    hwnd = CreateWindow(
        szAppName,
        TEXT("WM_IME_CHAR Example"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        500,
        400,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hwnd)
        return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void GetCurrentInputMethodDescription(TCHAR *descBuffer, int bufferSize)
{
    HKL hKL = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL));
    wsprintf(descBuffer, TEXT("Input Method (HKL: 0x%p)"), hKL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR ch = static_cast<TCHAR>(wParam);
    TCHAR msgBuffer[512];
    TCHAR imeDescription[256];

    switch (message)
    {
    case WM_IME_CHAR:
    {
        GetCurrentInputMethodDescription(imeDescription, sizeof(imeDescription) / sizeof(TCHAR));
        wsprintf(msgBuffer,
                 TEXT("Received WM_IME_CHAR:\nUnicode: 0x%X\nCharacter: %c\n%s"),
                 ch, ch, imeDescription);
        MessageBox(hwnd, msgBuffer, TEXT("WM_IME_CHAR Message"), MB_OK);
    }
        return 0;

    case WM_CHAR:
    {
        GetCurrentInputMethodDescription(imeDescription, sizeof(imeDescription) / sizeof(TCHAR));
        wsprintf(msgBuffer,
                 TEXT("Received WM_CHAR:\nUnicode: 0x%X\nCharacter: %c\n%s"),
                 ch, ch, imeDescription);
        MessageBox(hwnd, msgBuffer, TEXT("WM_CHAR Message"), MB_OK);
    }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}
