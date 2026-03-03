#include "OA2Window.h"
#include "ClipboardHelper.h"
#include "resource.h"
#include <windows.h>

static HWND s_hOA2 = NULL;
static HWND s_hEdit = NULL;

HWND GetOA2Window() { return s_hOA2; }

#define WM_RECEIVE_DATA  (WM_USER + 1)
#define BTN_PASTE        401

void SendDataToOA2(const std::wstring& data) {
    if (!s_hOA2) return;
    std::wstring* p = new std::wstring(data);
    PostMessage(s_hOA2, WM_RECEIVE_DATA, 0, (LPARAM)p);
}

void PasteFromClipboardToOA2() {
    std::wstring d = PasteFromClipboard();
    if (!d.empty()) SendDataToOA2(d);
    else MessageBoxW(s_hOA2,
        L"Clipboard does not contain data in TitanicPassengerTable format!\n"
        L"Only data copied via 'Copy to Clipboard' in OA1 can be pasted here.",
        L"Paste Failed", MB_ICONWARNING);
}

static LRESULT CALLBACK OA2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"Paste from Clipboard",
            WS_CHILD | WS_VISIBLE, 4, 4, 160, 24,
            hwnd, (HMENU)BTN_PASTE, GetModuleHandleW(NULL), NULL);
        s_hEdit = CreateWindowW(L"EDIT",
            L"OA2: waiting for data from OA1...\r\n\r\n"
            L"  -- Direct:    click 'Send to OA2' in OA1\r\n"
            L"  -- Clipboard: click 'Copy to Clipboard' in OA1,\r\n"
            L"                then click 'Paste from Clipboard' here or press Ctrl+V\r\n"
            L"  -- Pipe:      click 'Launch AppB' in OA1 to open AppB window\r\n"
            L"  -- DDE:       click 'Send via DDE' in OA1\r\n\r\n"
            L"  NOTE: only custom TitanicPassengerTable clipboard format is accepted.",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 32, 0, 0,
            hwnd, (HMENU)IDC_OA2_LISTBOX, GetModuleHandleW(NULL), NULL);
        SendMessage(s_hEdit, WM_SETFONT,
            (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), TRUE);
        return 0;

    case WM_SIZE:
        MoveWindow(s_hEdit, 0, 32,
            LOWORD(lParam), HIWORD(lParam) - 32, TRUE);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == BTN_PASTE) PasteFromClipboardToOA2();
        return 0;

    case WM_KEYDOWN:
        if (wParam == 'V' && (GetKeyState(VK_CONTROL) & 0x8000))
            PasteFromClipboardToOA2();
        return 0;

    case WM_RECEIVE_DATA: {
        std::wstring* p = reinterpret_cast<std::wstring*>(lParam);
        SetWindowTextW(s_hEdit, p->c_str());
        delete p;
        return 0;
    }
    case WM_COPYDATA: {
        auto* cds = reinterpret_cast<const COPYDATASTRUCT*>(lParam);
        if (cds && cds->lpData) {
            std::wstring d(static_cast<const wchar_t*>(cds->lpData));
            SendDataToOA2(d);
        }
        return TRUE;
    }
    case WM_DESTROY:
        s_hOA2 = s_hEdit = NULL;
        return 0;
    }
    return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

bool RegisterOA2Class(HINSTANCE hInst) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = OA2Proc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = OA2_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    return RegisterClassW(&wc) != 0;
}

HWND CreateOA2Window(HWND hMDIClient) {
    MDICREATESTRUCT mcs = {};
    mcs.szClass = OA2_CLASS;
    mcs.szTitle = L"OA2  --  THIS WINDOW CAN PASTE FROM CLIPBOARD";
    mcs.hOwner = GetModuleHandleW(NULL);
    mcs.x = 10; mcs.y = 490; mcs.cx = 780; mcs.cy = 300;
    s_hOA2 = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    return s_hOA2;
}