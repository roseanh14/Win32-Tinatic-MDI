#include "MDIFrame.h"
#include "OA1Window.h"
#include "OA2Window.h"
#include "TitanicData.h"
#include "resource.h"

static HWND s_hMDIClient = NULL;

HWND GetMDIClient() { return s_hMDIClient; }

static HMENU CreateAppMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hOA1Menu = CreatePopupMenu();
    HMENU hOA2Menu = CreatePopupMenu();

    AppendMenuW(hOA1Menu, MF_STRING, IDM_OA1_LOAD_CSV, L"Load Titanic CSV...");
    AppendMenuW(hOA1Menu, MF_STRING, IDM_OA1_DISPLAY, L"Display data in OA1");
    AppendMenuW(hOA1Menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hOA1Menu, MF_STRING, IDM_OA1_SEND_OA2, L"Send to OA2 (direct)");
    AppendMenuW(hOA1Menu, MF_STRING, IDM_OA1_CLIPBOARD, L"Copy to Clipboard (custom format)");
    AppendMenuW(hOA1Menu, MF_STRING, IDM_OA1_PIPE, L"Launch AppB via PIPE");
    AppendMenuW(hOA1Menu, MF_STRING, IDM_OA1_DDE, L"Start DDE server");

    AppendMenuW(hOA2Menu, MF_STRING, IDM_OA2_PASTE_CLIP, L"Paste from Clipboard (custom format)");
    AppendMenuW(hOA2Menu, MF_STRING, IDM_OA2_LOAD_DDE, L"Load from DDE");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hOA1Menu, L"OA1");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hOA2Menu, L"OA2");

    return hMenu;
}

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

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_OA1_LOAD_CSV: {
            wchar_t path[MAX_PATH] = {};
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"CSV Files\0*.csv\0All Files\0*.*\0";
            ofn.lpstrFile = path;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileNameW(&ofn)) {
                TitanicData::LoadCSV(path);
                RefreshOA1List();
            }
            break;
        }
        case IDM_OA1_DISPLAY:    RefreshOA1List();       break;
        case IDM_OA1_SEND_OA2:   SendDataToOA2(TitanicData::ToTabDelimited()); break;
        case IDM_OA1_CLIPBOARD:  OA1CopyToClipboard();   break;
        case IDM_OA1_PIPE:       OA1LaunchPipe();         break;
        case IDM_OA1_DDE:        OA1SendDDE();            break;
        case IDM_OA2_PASTE_CLIP: PasteFromClipboardToOA2(); break;
        case IDM_OA2_LOAD_DDE:   OA2LoadDDE();            break;
        }
        return DefFrameProcW(hwnd, s_hMDIClient, msg, wParam, lParam);

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
    HMENU hMenu = CreateAppMenu();
    return CreateWindowW(FRAME_CLASS, APP_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 700,
        NULL, hMenu, hInst, NULL);
}