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

#define BTN_SEND_OA2   301
#define BTN_CLIPBOARD  302
#define BTN_PIPE       303
#define BTN_DDE        304

static void PopulateList() {
    SendMessage(s_hList, LB_RESETCONTENT, 0, 0);
    for (auto& p : TitanicData::GetPassengers())
        SendMessage(s_hList, LB_ADDSTRING, 0,
            (LPARAM)TitanicData::GetSummaryLine(p).c_str());
}

static LRESULT CALLBACK OA1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"Send to OA2",
            WS_CHILD | WS_VISIBLE, 4, 4, 110, 24,
            hwnd, (HMENU)BTN_SEND_OA2, GetModuleHandleW(NULL), NULL);
        CreateWindowW(L"BUTTON", L"Copy to Clipboard",
            WS_CHILD | WS_VISIBLE, 120, 4, 140, 24,
            hwnd, (HMENU)BTN_CLIPBOARD, GetModuleHandleW(NULL), NULL);
        CreateWindowW(L"BUTTON", L"Launch AppB (Pipe)",
            WS_CHILD | WS_VISIBLE, 266, 4, 140, 24,
            hwnd, (HMENU)BTN_PIPE, GetModuleHandleW(NULL), NULL);
        CreateWindowW(L"BUTTON", L"Send via DDE",
            WS_CHILD | WS_VISIBLE, 412, 4, 120, 24,
            hwnd, (HMENU)BTN_DDE, GetModuleHandleW(NULL), NULL);
        s_hList = CreateWindowW(L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
            0, 32, 0, 0, hwnd,
            (HMENU)IDC_OA1_LISTBOX, GetModuleHandleW(NULL), NULL);
        SendMessage(s_hList, WM_SETFONT,
            (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), TRUE);
        PopulateList();
        return 0;

    case WM_SIZE:
        MoveWindow(s_hList, 0, 32,
            LOWORD(lParam), HIWORD(lParam) - 32, TRUE);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case BTN_SEND_OA2:
            SendDataToOA2(TitanicData::ToTabDelimited());
            break;
        case BTN_CLIPBOARD: {
            // Pass row count so the header can store it
            DWORD rows = (DWORD)TitanicData::GetPassengers().size();
            CopyToClipboard(TitanicData::ToTabDelimited(), rows);
            MessageBoxW(hwnd,
                L"Copied to clipboard using custom TitanicPassengerTable format.\n"
                L"Click 'Paste from Clipboard' in OA2 or press Ctrl+V there.\n\n"
                L"Note: plain Ctrl+V in other apps will NOT work — custom format only.",
                L"Clipboard", MB_ICONINFORMATION);
            break;
        }
        case BTN_PIPE:
            LaunchAppBWithPipe(TitanicData::ToTabDelimited());
            break;
        case BTN_DDE:
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
    mcs.szTitle = L"OA1  --  CSV Data Source";
    mcs.hOwner = GetModuleHandleW(NULL);
    mcs.x = 10; mcs.y = 10; mcs.cx = 780; mcs.cy = 460;
    s_hOA1 = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    return s_hOA1;
}