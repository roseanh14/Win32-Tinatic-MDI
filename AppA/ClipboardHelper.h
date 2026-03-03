#pragma once
#include <windows.h>
#include <string>

// Custom binary header prepended to all clipboard data
// This ensures ONLY our app can read the data (not plain Ctrl+V)
struct TitanicClipHeader {
    DWORD magic;    // must be 0x54495441 ("TITA") to be valid
    DWORD version;  // format version = 1
    DWORD rows;     // number of passenger rows
    DWORD reserved;
};

void         InitClipboardFormat();
bool         CopyToClipboard(const std::wstring& data, DWORD rowCount);
std::wstring PasteFromClipboard();  // returns empty if format not valid