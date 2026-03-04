#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")

static std::wstring g_receivedData;
static HWND s_hList = NULL;

static std::wstring ReadAllFromPipe() {
    std::string raw;
    char buf[4096];
    DWORD bytesRead = 0;
    while (ReadFile(GetStdHandle(STD_INPUT_HANDLE),
        buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buf[bytesRead] = '\0';
        raw += buf;
    }
    int wlen = MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), -1, NULL, 0);
    if (wlen <= 1) return L"";
    std::wstring result(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), -1, &result[0], wlen);
    return result;
}

static std::vector<std::wstring> SplitTab(const std::wstring& line) {
    std::vector<std::wstring> parts;
    std::wstring cur;
    for (wchar_t c : line) {
        if (c == L'\t') { parts.push_back(cur); cur.clear(); }
        else cur += c;
    }
    parts.push_back(cur);
    return parts;
}

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

static void PopulateList(HWND hLV, const std::wstring& data) {
    ListView_DeleteAllItems(hLV);
    size_t pos = 0;
    int row = 0;
    bool firstLine = true;
    while (pos <= data.size()) {
        size_t end = data.find(L'\n', pos);
        if (end == std::wstring::npos) end = data.size();
        std::wstring line = data.substr(pos, end - pos);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        pos = end + 1;
        if (line.empty()) continue;
        if (firstLine) { firstLine = false; continue; } // skip header row

        auto cols = SplitTab(line);

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.pszText = cols.size() > 0 ? (LPWSTR)cols[0].c_str() : (LPWSTR)L"";
        ListView_InsertItem(hLV, &item);
        for (int c = 1; c < 7 && c < (int)cols.size(); c++)
            ListView_SetItemText(hLV, row, c, (LPWSTR)cols[c].c_str());
        row++;
    }
}

static LRESULT CALLBACK AppBWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        s_hList = CreateWindowExW(0, WC_LISTVIEWW, NULL,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_DATA_EDIT, GetModuleHandleW(NULL), NULL);
        ListView_SetExtendedListViewStyle(s_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        {
            // Excel-like Calibri font
            HFONT hFont = CreateFontW(
                16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SWISS, L"Calibri");
            SendMessage(s_hList, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        SetupColumns(s_hList);
        PopulateList(s_hList, g_receivedData);
        return 0;

    case WM_SIZE:
        if (s_hList)
            MoveWindow(s_hList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX icx = { sizeof(icx), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icx);

    g_receivedData = ReadAllFromPipe();
    if (g_receivedData.empty()) {
        MessageBoxW(NULL, L"No data received from pipe!", APPB_TITLE, MB_ICONWARNING);
        return 1;
    }

    WNDCLASSW wc = {};
    wc.lpfnWndProc = AppBWndProc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"AppBWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"AppBWindow", APPB_TITLE,
        WS_OVERLAPPEDWINDOW,
        150, 150, 900, 620,
        NULL, NULL, hInst, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}