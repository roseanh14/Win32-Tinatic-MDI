// =============================================================
//  AppB — Pipe Receiver
//  Reads Titanic data from stdin (pipe) and shows it in a window
// =============================================================
#include <windows.h>
#include "resource.h"
#include <string>

static std::wstring g_receivedData;  // Data read from the pipe, shown in the window

// Read ALL bytes from stdin (which is our pipe) until the pipe closes
static std::wstring ReadAllFromPipe() {
    std::string raw;
    char buf[4096];
    DWORD bytesRead = 0;

    // ReadFile returns FALSE when the pipe is closed (EOF)
    while (ReadFile(GetStdHandle(STD_INPUT_HANDLE),
                    buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buf[bytesRead] = '\0';
        raw += buf;
    }

    // Convert UTF-8 bytes back to a wide string for display
    int wlen = MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), -1, NULL, 0);
    if (wlen <= 1) return L"";
    std::wstring result(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), -1, &result[0], wlen);
    return result;
}

// Window procedure for AppB's simple display window
static LRESULT CALLBACK AppBWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // Get the window size so we can fill it with the edit box
        RECT rc;
        GetClientRect(hwnd, &rc);

        // Create a read-only multi-line text box to display the data
        HWND hEdit = CreateWindowW(L"EDIT", g_receivedData.c_str(),
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 0, rc.right, rc.bottom,
            hwnd, (HMENU)IDC_DATA_EDIT, GetModuleHandleW(NULL), NULL);
        SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), TRUE);
        return 0;
    }
    case WM_SIZE: {
        // Keep the edit box filling the whole window when resized
        HWND hEdit = GetDlgItem(hwnd, IDC_DATA_EDIT);
        if (hEdit) MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {

    // Step 1: Read the piped data BEFORE showing any window
    g_receivedData = ReadAllFromPipe();

    if (g_receivedData.empty()) {
        MessageBoxW(NULL, L"No data received from pipe!", APPB_TITLE, MB_ICONWARNING);
        return 1;
    }

    // Step 2: Register a simple window class
    WNDCLASSW wc     = {};
    wc.lpfnWndProc   = AppBWndProc;
    wc.hInstance     = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"AppBWindow";
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassW(&wc);

    // Step 3: Create and show the window
    HWND hwnd = CreateWindowW(L"AppBWindow", APPB_TITLE,
        WS_OVERLAPPEDWINDOW,
        150, 150, 860, 620,
        NULL, NULL, hInst, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Step 4: Message loop (same pattern as AppA)
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}