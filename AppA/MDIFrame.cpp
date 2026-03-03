#include "MDIFrame.h"
#include "OA1Window.h"
#include "OA2Window.h"
#include "resource.h"

static HWND s_hMDIClient = NULL;

HWND GetMDIClient() { return s_hMDIClient; }

static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CLIENTCREATESTRUCT ccs = {};
        ccs.hWindowMenu = NULL;
        ccs.idFirstChild = 100;

        s_hMDIClient = CreateWindowW(
            L"MDICLIENT",
            NULL,
            WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
            0, 0, 0, 0,
            hwnd, (HMENU)1, GetModuleHandleW(NULL), &ccs);

        CreateOA2Window(s_hMDIClient);
        CreateOA1Window(s_hMDIClient);
        return 0;
    }

    case WM_SIZE:
        MoveWindow(s_hMDIClient, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_COMMAND:
    {
        // Forward menu commands to the currently active MDI child window
        HWND hActive = (HWND)SendMessageW(s_hMDIClient, WM_MDIGETACTIVE, 0, 0);
        if (hActive) {
            SendMessageW(hActive, WM_COMMAND, wParam, lParam);
            return 0;
        }

        // Fallback: if nothing active, send to OA1 if it exists
        if (GetOA1Window()) {
            SendMessageW(GetOA1Window(), WM_COMMAND, wParam, lParam);
            return 0;
        }

        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefFrameProc(hwnd, s_hMDIClient, msg, wParam, lParam);
}

bool RegisterMDIFrameClass(HINSTANCE hInst) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = FrameWndProc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.lpszClassName = FRAME_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassW(&wc) != 0;
}

HWND CreateMDIFrameWindow(HINSTANCE hInst, int nCmdShow) {
    HWND hwnd = CreateWindowW(
        FRAME_CLASS, APP_TITLE,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 720,
        NULL, NULL, hInst, NULL);
    if (!hwnd) return NULL;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return hwnd;
}