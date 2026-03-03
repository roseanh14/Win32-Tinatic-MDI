#include "ClipboardHelper.h"
#include "resource.h"

// Windows assigns a unique number to our custom format name
// We store it here so we can use it later
static UINT s_clipFormat = 0;

void InitClipboardFormat() {
    // RegisterClipboardFormat = "hey Windows, I have a custom data format called X"
    // Windows gives us back an ID number we use for all future clipboard operations
    s_clipFormat = RegisterClipboardFormatW(CLIPBOARD_FORMAT_NAME);
}

bool CopyToClipboard(const std::wstring& data) {
    if (s_clipFormat == 0) InitClipboardFormat();

    // Calculate bytes needed: (chars + 1 null terminator) × 2 bytes per wide char
    size_t bytes = (data.size() + 1) * sizeof(wchar_t);

    // GlobalAlloc = allocate memory that can be shared between programs
    // GMEM_MOVEABLE = Windows can move it in memory if needed (required for clipboard)
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) return false;

    // Lock to get a real pointer, copy data, then unlock
    void* ptr = GlobalLock(hMem);
    memcpy(ptr, data.c_str(), bytes);
    GlobalUnlock(hMem);

    // Open clipboard, wipe it, put our data in, close it
    if (!OpenClipboard(NULL)) { GlobalFree(hMem); return false; }
    EmptyClipboard();
    SetClipboardData(s_clipFormat, hMem); // Use OUR custom format ID
    CloseClipboard();
    return true;
}

std::wstring PasteFromClipboard() {
    if (s_clipFormat == 0) InitClipboardFormat();
    if (!OpenClipboard(NULL)) return L"";

    // GetClipboardData returns NULL if the clipboard doesn't have our format
    HANDLE hData = GetClipboardData(s_clipFormat);
    std::wstring result;

    if (hData) {
        wchar_t* ptr = static_cast<wchar_t*>(GlobalLock(hData));
        if (ptr) {
            result = ptr; // Copy the string out
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
    return result; // Empty string if our format wasn't there
}