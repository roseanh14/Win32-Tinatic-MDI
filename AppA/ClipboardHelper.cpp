#include "ClipboardHelper.h"
#include "resource.h"

static UINT s_clipFormat = 0;

static const DWORD TITA_MAGIC = 0x54495441; // "TITA"
static const DWORD TITA_VERSION = 1;

void InitClipboardFormat() {
    // Register our own named format — Windows assigns a unique ID
    // No other app knows this format, so random Ctrl+V won't work in OA2
    s_clipFormat = RegisterClipboardFormatW(CLIPBOARD_FORMAT_NAME);
}

bool CopyToClipboard(const std::wstring& data, DWORD rowCount) {
    if (s_clipFormat == 0) InitClipboardFormat();

    // Layout in memory: [TitanicClipHeader][wchar_t data...]
    size_t dataBytes = (data.size() + 1) * sizeof(wchar_t);
    size_t totalBytes = sizeof(TitanicClipHeader) + dataBytes;

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, totalBytes);
    if (!hMem) return false;

    BYTE* ptr = static_cast<BYTE*>(GlobalLock(hMem));
    if (!ptr) { GlobalFree(hMem); return false; }

    // Write header
    TitanicClipHeader hdr = { TITA_MAGIC, TITA_VERSION, rowCount, 0 };
    memcpy(ptr, &hdr, sizeof(hdr));

    // Write wide string after header
    memcpy(ptr + sizeof(hdr), data.c_str(), dataBytes);
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
        BYTE* ptr = static_cast<BYTE*>(GlobalLock(hData));
        if (ptr) {
            // Validate magic number — reject anything that isn't our format
            TitanicClipHeader* hdr = reinterpret_cast<TitanicClipHeader*>(ptr);
            if (hdr->magic == TITA_MAGIC && hdr->version == TITA_VERSION) {
                // Data starts right after the header
                const wchar_t* text = reinterpret_cast<const wchar_t*>(ptr + sizeof(TitanicClipHeader));
                result = text;
            }
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
    return result; // empty = wrong format
}