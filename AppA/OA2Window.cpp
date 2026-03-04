#include "OA2Window.h"
#include "ClipboardHelper.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

static HWND s_hOA2 = NULL;
static HWND s_hList = NULL;

HWND GetOA2Window() { return s_hOA2; }

#define WM_RECEIVE_DATA  (WM_USER + 1)
#define BTN_PASTE        401

static void SetupColumns(HWND hLV) {
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    const wchar_t* headers[] = { L"ID", L"Survived", L"Class", L"Name", L"Sex", L"Age", L"Fare" };
    int widths[] = { 40, 70, 55, 200, 60, 45, 65 };
    for (int i = 0; i < 7; i++) {
        col.pszText = (LPWSTR)headers[i];
        col.cx = widths[i];
        ListView_InsertColumn(hLV, i, &col);
    }
}

static void DisplayData(HWND hLV, const std::wstring& data) {
    ListView_DeleteAllItems(hLV);
    int row = 0;
    size_t pos = 0;
    bool firstLine = true;
    while (pos < data.size()) {
        size_t end = data.find(L'\n', pos);
        if (end == std::wstring::npos) end = data.size();
        std::wstring line = data.substr(pos, end - pos);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        pos = end + 1;
        if (line.empty()) continue;
        if (firstLine) { firstLine = false; continue; } 

        
        std::wstring cols[7];
        int col = 0;
        size_t p2 = 0;
        while (col < 7 && p2 <= line.size()) {
            size_t t = line.find(L'\t', p2);
            if (t == std::wstring::npos) t = line.size();
            cols[col++] = line.substr(p2, t - p2);
            p2 = t + 1;
        }

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.pszText = (LPWSTR)cols[0].c_str();
        ListView_InsertItem(hLV, &item);
        for (int c = 1; c < 7; c++)
            ListView_SetItemText(hLV, row, c, (LPWSTR)cols[c].c_str());
        row++;
    }
}

void SendDataToOA2(const std::wstring& data) {
    if (!s_hOA2) return;
    std::wstring* p = new std::wstring(data);
    PostMessage(s_hOA2, WM_RECEIVE_DATA, 0, (LPARAM)p);
}

void PasteFromClipboardToOA2() {
    std::wstring d = PasteFromClipboard();
    if (!d.empty()) SendDataToOA2(d);
    else MessageBoxW(s_hOA2,
        L"Clipboard does not contain TitanicPassengerTable format!\n"
        L"Only data copied via 'Copy to Clipboard' in OA1 is accepted.",
        L"Paste Failed", MB_ICONWARNING);
}

static LRESULT CALLBACK OA2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"Paste from Clipboard",
            WS_CHILD | WS_VISIBLE, 4, 4, 160, 24,
            hwnd, (HMENU)BTN_PASTE, GetModuleHandleW(NULL), NULL);
        s_hList = CreateWindowExW(0, WC_LISTVIEWW, NULL,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 32, 0, 0,
            hwnd, (HMENU)IDC_OA2_LISTBOX, GetModuleHandleW(NULL), NULL);
        ListView_SetExtendedListViewStyle(s_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        SetupColumns(s_hList);
        return 0;

    case WM_SIZE:
        MoveWindow(s_hList, 0, 32,
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
        DisplayData(s_hList, *p);
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
        s_hOA2 = s_hList = NULL;
        return 0;
    }
    return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

bool RegisterOA2Class(HINSTANCE hInst) {
    INITCOMMONCONTROLSEX icx = { sizeof(icx), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icx);
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
    mcs.szTitle = L"OA2  --  Data Display";
    mcs.hOwner = GetModuleHandleW(NULL);
    mcs.x = 420; mcs.y = 10; mcs.cx = 600; mcs.cy = 460;
    s_hOA2 = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    return s_hOA2;
}