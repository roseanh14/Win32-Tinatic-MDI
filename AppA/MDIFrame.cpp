#include "MDIFrame.h"
#include "OA1Window.h"
#include "OA2Window.h"
#include "resource.h"

static HWND s_hMDIClient = NULL;

HWND GetMDIClient() { return s_hMDIClient; }

static LRESULT CALLBACK FrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CLIENTCREATESTRUCT ccs = {};
        ccs.hWindowMenu = NULL;
        ccs.idFirstChild = 9001;
        s_hMDIClient = CreateWindowW(L"MDICLIENT", NULL,
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
            0, 0, 0, 0,
            hwnd, NULL, GetModuleHandleW(NULL), &ccs);
        CreateOA1Window(s_hMDIClient);
        CreateOA2Window(s_hMDIClient);
        return 0;
    }
    case WM_SIZE:
        MoveWindow(s_hMDIClient, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefFrameProcW(hwnd, s_hMDIClient, msg, wParam, lParam);
}

bool RegisterFrameClass(HINSTANCE hInst) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = FrameProc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.lpszClassName = FRAME_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassW(&wc) != 0;
}

HWND CreateFrameWindow(HINSTANCE hInst) {
    return CreateWindowW(FRAME_CLASS, APP_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 700,
        NULL, NULL, hInst, NULL);
}