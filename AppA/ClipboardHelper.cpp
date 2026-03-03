#include "ClipboardHelper.h"
#include "resource.h"

static UINT s_clipFormat = 0;

void InitClipboardFormat() {
    s_clipFormat = RegisterClipboardFormatW(CLIPBOARD_FORMAT_NAME);
}

bool CopyToClipboard(const std::wstring& data) {
    if (s_clipFormat == 0) InitClipboardFormat();

    size_t bytes = (data.size() + 1) * sizeof(wchar_t);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) return false;

    void* ptr = GlobalLock(hMem);
    if (!ptr) { GlobalFree(hMem); return false; }  // fixes C6387
    memcpy(ptr, data.c_str(), bytes);
    GlobalUnlock(hMem);

    if (!OpenClipboard(NULL)) { GlobalFree(hMem); return false; }
    EmptyClipboard();
    SetClipboardData(s_clipFormat, hMem);
    CloseClipboard();
    return true;
}

std::wstring PasteFromClipboard() {
    if (s_clipFormat == 0) InitClipboardFormat();
    if (!OpenClipboard(NULL)) return L"";

    HANDLE hData = GetClipboardData(s_clipFormat);
    std::wstring result;

    if (hData) {
        wchar_t* ptr = static_cast<wchar_t*>(GlobalLock(hData));
        if (ptr) {
            result = ptr;
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
    return result;
}