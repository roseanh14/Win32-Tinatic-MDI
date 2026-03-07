#include "DDEServer.h"
#include "resource.h"
#include <ddeml.h>
#include <commctrl.h>
#include <string>
#include <vector>
#pragma comment(lib, "comctl32.lib")

static HWND s_hList = NULL;

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
    size_t pos = 0; int row = 0; bool first = true;
    while (pos <= data.size()) {
        size_t end = data.find(L'\n', pos);
        if (end == std::wstring::npos) end = data.size();
        std::wstring line = data.substr(pos, end - pos);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        pos = end + 1;
        if (line.empty()) continue;
        if (first) { first = false; continue; }
        auto cols = SplitTab(line);
        LVITEMW item = {}; item.mask = LVIF_TEXT; item.iItem = row;
        item.pszText = cols.size() > 0 ? (LPWSTR)cols[0].c_str() : (LPWSTR)L"";
        ListView_InsertItem(hLV, &item);
        for (int c = 1; c < 7 && c < (int)cols.size(); c++)
            ListView_SetItemText(hLV, row, c, (LPWSTR)cols[c].c_str());
        row++;
    }
}

static LRESULT CALLBACK ClientWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        s_hList = CreateWindowExW(0, WC_LISTVIEWW, NULL,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 0, 0, 0, hwnd, NULL, GetModuleHandleW(NULL), NULL);
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
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int RunAsDDEClient() {
    INITCOMMONCONTROLSEX icx = { sizeof(icx), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icx);

    // Register window
    WNDCLASSW wc = {};
    wc.lpfnWndProc = ClientWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"DDEClientFrame";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"DDEClientFrame",
        L"DDE Client -- Data received from AppA",
        WS_OVERLAPPEDWINDOW,
        200, 200, 900, 600,
        NULL, NULL, GetModuleHandleW(NULL), NULL);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Connect to DDE server (AppA server instance)
    DWORD inst = 0;
    HDDEDATA CALLBACK cb(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR);

    auto simpleCb = [](UINT type, UINT, HCONV, HSZ, HSZ, HDDEDATA hData, ULONG_PTR, ULONG_PTR) -> HDDEDATA {
        return NULL;
        };

    // Use simple callback for client
    DdeInitializeW(&inst, [](UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR) -> HDDEDATA {
        return NULL;
        }, APPCMD_CLIENTONLY, 0);

    HSZ hSvc = DdeCreateStringHandleW(inst, DDE_SERVICE, CP_WINUNICODE);
    HSZ hTopic = DdeCreateStringHandleW(inst, DDE_TOPIC, CP_WINUNICODE);
    HSZ hItem = DdeCreateStringHandleW(inst, L"Data", CP_WINUNICODE);

    // Retry connect a few times (server needs a moment)
    HCONV hConv = NULL;
    for (int i = 0; i < 10 && !hConv; i++) {
        Sleep(200);
        hConv = DdeConnect(inst, hSvc, hTopic, NULL);
    }

    if (hConv) {
        HDDEDATA hResult = DdeClientTransaction(NULL, 0, hConv,
            hItem, CF_UNICODETEXT, XTYP_REQUEST, 5000, NULL);
        if (hResult) {
            DWORD len = 0;
            const wchar_t* ptr = (const wchar_t*)DdeAccessData(hResult, &len);
            if (ptr && s_hList) PopulateList(s_hList, std::wstring(ptr));
            DdeUnaccessData(hResult);
            DdeFreeDataHandle(hResult);
        }
        DdeDisconnect(hConv);
    }
    else {
        MessageBoxW(hwnd, L"Could not connect to DDE server!", L"DDE Error", MB_ICONERROR);
    }

    DdeFreeStringHandle(inst, hSvc);
    DdeFreeStringHandle(inst, hTopic);
    DdeFreeStringHandle(inst, hItem);
    DdeUninitialize(inst);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}