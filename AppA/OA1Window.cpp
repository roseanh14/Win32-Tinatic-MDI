#include "OA1Window.h"
#include "OA2Window.h"
#include "TitanicData.h"
#include "ClipboardHelper.h"
#include "PipeLauncher.h"
#include "DDEServer.h"
#include "resource.h"

static HWND s_hOA1 = NULL;
static HWND s_hList = NULL;

HWND GetOA1Window() { return s_hOA1; }

// Build the dropdown menu for OA1 programmatically (no .rc file needed)
static HMENU CreateOA1Menu() {
    HMENU hBar = CreateMenu();
    HMENU hData = CreatePopupMenu();

    AppendMenuW(hData, MF_STRING, IDM_DATA_DISPLAY, L"1.  Display Data Here");
    AppendMenuW(hData, MF_STRING, IDM_DATA_SEND_OA2, L"2.  Send to OA2  (direct)");
    AppendMenuW(hData, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hData, MF_STRING, IDM_DATA_SEND_CLIPBOARD, L"3.  Copy to Clipboard");
    AppendMenuW(hData, MF_STRING, IDM_DATA_PASTE_OA2, L"4.  Paste Clipboard -> OA2");
    AppendMenuW(hData, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hData, MF_STRING, IDM_DATA_SEND_PIPE, L"5.  Launch AppB via Pipe");
    AppendMenuW(hData, MF_STRING, IDM_DATA_SEND_DDE, L"6.  Send to OA2  (DDE)");

    AppendMenuW(hBar, MF_POPUP, (UINT_PTR)hData, L"Data");
    return hBar;
}

static void SetFrameMenuFromChild(HWND hwndChild) {
    HWND hFrame = GetParent(GetParent(hwndChild)); // child -> MDI client -> frame
    if (!hFrame) return;
    SetMenu(hFrame, CreateOA1Menu());
    DrawMenuBar(hFrame);
}

// Fill the listbox with one line per passenger
static void PopulateList(HWND hwndOwner) {
    const size_t n = TitanicData::GetPassengers().size();
    if (n == 0) {
        MessageBoxW(
            hwndOwner,
            L"No passengers loaded (0).\n\nThis usually means titanic.csv was not loaded.\nCheck working directory / data folder.",
            L"OA1",
            MB_OK | MB_ICONWARNING
        );
        SendMessage(s_hList, LB_RESETCONTENT, 0, 0);
        return;
    }

    SendMessage(s_hList, LB_RESETCONTENT, 0, 0);

    for (const auto& p : TitanicData::GetPassengers()) {
        std::wstring line = TitanicData::GetSummaryLine(p);
        SendMessageW(s_hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    }
}

// The window procedure for OA1
static LRESULT CALLBACK OA1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        s_hList = CreateWindowW(
            L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
            0, 0, 0, 0,
            hwnd,
            (HMENU)IDC_OA1_LISTBOX,
            GetModuleHandleW(NULL),
            NULL
        );

        SendMessageW(s_hList, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), TRUE);

        // Ensure the menu is visible immediately (not only after activation)
        SetFrameMenuFromChild(hwnd);
        return 0;

    case WM_SIZE:
        if (s_hList) {
            MoveWindow(s_hList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        }
        return 0;

    case WM_MDIACTIVATE:
        // When OA1 gains focus, show its menu on the frame window
        if ((HWND)lParam == hwnd) {
            SetFrameMenuFromChild(hwnd);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_DATA_DISPLAY:
            PopulateList(hwnd);
            break;

        case IDM_DATA_SEND_OA2:
            SendDataToOA2(TitanicData::ToTabDelimited());
            break;

        case IDM_DATA_SEND_CLIPBOARD:
            if (CopyToClipboard(TitanicData::ToTabDelimited())) {
                MessageBoxW(
                    hwnd,
                    L"Data copied.\nUse menu item 4 to paste into OA2.",
                    L"Clipboard",
                    MB_OK | MB_ICONINFORMATION
                );
            }
            break;

        case IDM_DATA_PASTE_OA2:
            PasteFromClipboardToOA2();
            break;

        case IDM_DATA_SEND_PIPE:
            LaunchAppBWithPipe(TitanicData::ToTabDelimited());
            break;

        case IDM_DATA_SEND_DDE:
            DDESendToOA2(TitanicData::ToTabDelimited());
            break;
        }
        return 0;
    }

    return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

bool RegisterOA1Class(HINSTANCE hInst) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = OA1Proc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = OA1_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    return RegisterClassW(&wc) != 0;
}

HWND CreateOA1Window(HWND hMDIClient) {
    MDICREATESTRUCT mcs = {};
    mcs.szClass = OA1_CLASS;
    mcs.szTitle = L"OA1  -  Data Source  (use Data menu)";
    mcs.hOwner = GetModuleHandleW(NULL);
    mcs.x = CW_USEDEFAULT; mcs.y = CW_USEDEFAULT;
    mcs.cx = 750; mcs.cy = 420;

    s_hOA1 = (HWND)SendMessageW(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    return s_hOA1;
}