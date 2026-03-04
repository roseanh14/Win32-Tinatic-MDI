#include "OA1Window.h"
#include "OA2Window.h"
#include "TitanicData.h"
#include "ClipboardHelper.h"
#include "PipeLauncher.h"
#include "DDEServer.h"
#include "resource.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

static HWND s_hOA1 = NULL;
static HWND s_hList = NULL;

HWND GetOA1Window() { return s_hOA1; }

#define BTN_SEND_OA2   301
#define BTN_CLIPBOARD  302
#define BTN_PIPE       303
#define BTN_DDE        304

static void SetupColumns(HWND hLV) {
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    const wchar_t* headers[] = { L"ID", L"Survived", L"Class", L"Name", L"Sex", L"Age", L"Fare" };
    int widths[] = { 40, 70, 55, 200, 60, 45, 65 };
    for (int i = 0; i < 7; i++) {
        col.pszText = (LPWSTR)headers[i];
        col.cx      = widths[i];
        ListView_InsertColumn(hLV, i, &col);
    }
}

static void PopulateList(HWND hLV) {
    ListView_DeleteAllItems(hLV);
    int row = 0;
    for (auto& p : TitanicData::GetPassengers()) {
        LVITEMW item = {};
        item.mask    = LVIF_TEXT;
        item.iItem   = row;

        wchar_t buf[64];
        swprintf(buf, 64, L"%d", p.passengerId);
        item.pszText = buf;
        ListView_InsertItem(hLV, &item);

        swprintf(buf, 64, L"%d", p.survived);
        ListView_SetItemText(hLV, row, 1, buf);
        swprintf(buf, 64, L"%d", p.pclass);
        ListView_SetItemText(hLV, row, 2, buf);
        ListView_SetItemText(hLV, row, 3, (LPWSTR)p.name.c_str());
        ListView_SetItemText(hLV, row, 4, (LPWSTR)p.sex.c_str());
        if (p.age >= 0) swprintf(buf, 64, L"%.0f", p.age);
        else wcscpy_s(buf, L"-");
        ListView_SetItemText(hLV, row, 5, buf);
        swprintf(buf, 64, L"%.2f", p.fare);
        ListView_SetItemText(hLV, row, 6, buf);
        row++;
    }
}

static LRESULT CALLBACK OA1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"Send to OA2",
            WS_CHILD|WS_VISIBLE, 4, 4, 110, 24,
            hwnd, (HMENU)BTN_SEND_OA2, GetModuleHandleW(NULL), NULL);
        CreateWindowW(L"BUTTON", L"Copy to Clipboard",
            WS_CHILD|WS_VISIBLE, 120, 4, 140, 24,
            hwnd, (HMENU)BTN_CLIPBOARD, GetModuleHandleW(NULL), NULL);
        CreateWindowW(L"BUTTON", L"Launch AppB (Pipe)",
            WS_CHILD|WS_VISIBLE, 266, 4, 140, 24,
            hwnd, (HMENU)BTN_PIPE, GetModuleHandleW(NULL), NULL);
        CreateWindowW(L"BUTTON", L"Send via DDE",
            WS_CHILD|WS_VISIBLE, 412, 4, 120, 24,
            hwnd, (HMENU)BTN_DDE, GetModuleHandleW(NULL), NULL);
        s_hList = CreateWindowExW(0, WC_LISTVIEWW, NULL,
            WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS,
            0, 32, 0, 0,
            hwnd, (HMENU)IDC_OA1_LISTBOX, GetModuleHandleW(NULL), NULL);
        ListView_SetExtendedListViewStyle(s_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        SetupColumns(s_hList);
        PopulateList(s_hList);
        return 0;

    case WM_SIZE:
        MoveWindow(s_hList, 0, 32,
            LOWORD(lParam), HIWORD(lParam) - 32, TRUE);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case BTN_SEND_OA2:
            SendDataToOA2(TitanicData::ToTabDelimited()); break;
        case BTN_CLIPBOARD: {
            DWORD rows = (DWORD)TitanicData::GetPassengers().size();
            CopyToClipboard(TitanicData::ToTabDelimited(), rows);
            MessageBoxW(hwnd,
                L"Copied! Click 'Paste from Clipboard' in OA2 or press Ctrl+V.",
                L"Clipboard", MB_ICONINFORMATION);
            break;
        }
        case BTN_PIPE:
            LaunchAppBWithPipe(TitanicData::ToTabDelimited()); break;
        case BTN_DDE:
            DDESendToOA2(TitanicData::ToTabDelimited()); break;
        }
        return 0;
    }
    return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

bool RegisterOA1Class(HINSTANCE hInst) {
    INITCOMMONCONTROLSEX icx = { sizeof(icx), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icx);
    WNDCLASSW wc     = {};
    wc.lpfnWndProc   = OA1Proc;
    wc.hInstance     = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = OA1_CLASS;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    return RegisterClassW(&wc) != 0;
}

HWND CreateOA1Window(HWND hMDIClient) {
    MDICREATESTRUCT mcs = {};
    mcs.szClass = OA1_CLASS;
    mcs.szTitle = L"OA1  --  Data Source";
    mcs.hOwner  = GetModuleHandleW(NULL);
    mcs.x = 10; mcs.y = 10; mcs.cx = 600; mcs.cy = 460;
    s_hOA1 = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    return s_hOA1;
}