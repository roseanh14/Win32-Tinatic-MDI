#include "OA2Window.h"
#include "ClipboardHelper.h"
#include "resource.h"
#include <windows.h>

static HWND s_hOA2  = NULL;   // Handle to the OA2 window itself
static HWND s_hList = NULL;   // Handle to the listbox inside OA2

HWND GetOA2Window() { return s_hOA2; }

// WM_USER is a safe starting point for custom window messages
// We define our own message for sending data
#define WM_RECEIVE_DATA (WM_USER + 1)

// Split the tab-delimited text into lines and show each in the listbox
static void DisplayData(const std::wstring& data) {
    SendMessage(s_hList, LB_RESETCONTENT, 0, 0);  // Clear old content
    size_t pos = 0;
    while (pos < data.size()) {
        size_t end = data.find(L'\n', pos);
        if (end == std::wstring::npos) end = data.size();
        std::wstring line = data.substr(pos, end - pos);
        if (!line.empty() && line.back() == L'\r') line.pop_back(); // Remove \r
        if (!line.empty())
            SendMessage(s_hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        pos = end + 1;
    }
}

// Called from OA1 or DDE - allocates a copy on the heap and posts a message
// Why heap? PostMessage is async - the string must survive until OA2 reads it
void SendDataToOA2(const std::wstring& data) {
    if (!s_hOA2) return;
    std::wstring* pCopy = new std::wstring(data);   // Allocate on heap
    PostMessage(s_hOA2, WM_RECEIVE_DATA, 0, (LPARAM)pCopy); // Send pointer as lParam
}

void PasteFromClipboardToOA2() {
    std::wstring data = PasteFromClipboard();
    if (!data.empty()) SendDataToOA2(data);
    else MessageBoxW(s_hOA2, L"No Titanic data found in clipboard!", L"Paste", MB_ICONINFORMATION);
}

// The window procedure - handles all messages for OA2
// Windows calls this automatically whenever something happens to OA2
static LRESULT CALLBACK OA2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        // Create a listbox that fills the whole window
        s_hList = CreateWindowW(L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
            0, 0, 0, 0, hwnd, (HMENU)IDC_OA2_LISTBOX, GetModuleHandleW(NULL), NULL);
        // Fixed-width font so tab-separated columns look aligned
        SendMessage(s_hList, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), TRUE);
        return 0;

    case WM_SIZE:
        // Keep the listbox filling the whole OA2 window when it's resized
        MoveWindow(s_hList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_RECEIVE_DATA: {
        // lParam holds a pointer to the std::wstring we allocated in SendDataToOA2
        std::wstring* pData = reinterpret_cast<std::wstring*>(lParam);
        DisplayData(*pData);
        delete pData;  // Free the heap memory we allocated
        return 0;
    }
    case WM_DESTROY:
        s_hOA2 = s_hList = NULL;
        return 0;
    }
    // DefMDIChildProc handles standard MDI behavior (close, maximize, etc.)
    return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

bool RegisterOA2Class(HINSTANCE hInst) {
    WNDCLASSW wc     = {};
    wc.lpfnWndProc   = OA2Proc;
    wc.hInstance     = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = OA2_CLASS;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    return RegisterClassW(&wc) != 0;
}

HWND CreateOA2Window(HWND hMDIClient) {
    // MDICREATESTRUCT tells the MDI client how to create a child window
    MDICREATESTRUCT mcs = {};
    mcs.szClass = OA2_CLASS;
    mcs.szTitle = L"OA2  —  Data Receiver";
    mcs.hOwner  = GetModuleHandleW(NULL);
    mcs.x = 50; mcs.y = 50; mcs.cx = 700; mcs.cy = 400;
    s_hOA2 = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    return s_hOA2;
}