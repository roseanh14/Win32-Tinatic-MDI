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

static void SetupColumns(HWND hLV) {
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    const wchar_t* headers[] = { L"ID", L"Survived", L"Class", L"Name", L"Sex", L"Age", L"Fare" };
    int widths[] = { 40, 70, 55, 220, 60, 45, 70 };
    for (int i = 0; i < 7; i++) {
        col.pszText = (LPWSTR)headers[i];
        col.cx = widths[i];
        ListView_InsertColumn(hLV, i, &col);
    }
}

void RefreshOA1List() {
    if (!s_hList) return;
    ListView_DeleteAllItems(s_hList);
    int row = 0;
    for (auto& p : TitanicData::GetPassengers()) {
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = row;
        wchar_t buf[64];
        swprintf(buf, 64, L"%d", p.passengerId);
        item.pszText = buf;
        ListView_InsertItem(s_hList, &item);
        swprintf(buf, 64, L"%d", p.survived);
        ListView_SetItemText(s_hList, row, 1, buf);
        swprintf(buf, 64, L"%d", p.pclass);
        ListView_SetItemText(s_hList, row, 2, buf);
        ListView_SetItemText(s_hList, row, 3, (LPWSTR)p.name.c_str());
        ListView_SetItemText(s_hList, row, 4, (LPWSTR)p.sex.c_str());
        if (p.age >= 0) swprintf(buf, 64, L"%.0f", p.age);
        else wcscpy_s(buf, L"-");
        ListView_SetItemText(s_hList, row, 5, buf);
        swprintf(buf, 64, L"%.2f", p.fare);
        ListView_SetItemText(s_hList, row, 6, buf);
        row++;
    }
}

void OA1CopyToClipboard() {
    DWORD rows = (DWORD)TitanicData::GetPassengers().size();
    CopyToClipboard(TitanicData::ToTabDelimited(), rows);
    MessageBoxW(s_hOA1,
        L"Copied using custom TitanicPassengerTable format.\n"
        L"Use OA2 menu -> Paste from Clipboard to display.",
        L"Clipboard", MB_ICONINFORMATION);
}

void OA1LaunchPipe() {
    LaunchAppBWithPipe(TitanicData::ToTabDelimited());
}

void OA1SendDDE() {
    DDESendToOA2(TitanicData::ToTabDelimited());
}

static LRESULT CALLBACK OA1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        s_hList = CreateWindowExW(0, WC_LISTVIEWW, NULL,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_OA1_LISTBOX, GetModuleHandleW(NULL), NULL);
        ListView_SetExtendedListViewStyle(s_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        {
            HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Calibri");
            SendMessage(s_hList, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        SetupColumns(s_hList);
        return 0;

    case WM_SIZE:
        MoveWindow(s_hList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    }
    return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

bool RegisterOA1Class(HINSTANCE hInst) {
    INITCOMMONCONTROLSEX icx = { sizeof(icx), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icx);
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
    mcs.szTitle = L"OA1  --  Data Source";
    mcs.hOwner = GetModuleHandleW(NULL);
    mcs.x = 10; mcs.y = 10; mcs.cx = 600; mcs.cy = 460;
    s_hOA1 = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    return s_hOA1;
}