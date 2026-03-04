#pragma once
#include <windows.h>
#include <string>

struct TitanicClipHeader {
    DWORD magic;    
    DWORD version;  
    DWORD rows;     
    DWORD reserved;
};

void         InitClipboardFormat();
bool         CopyToClipboard(const std::wstring& data, DWORD rowCount);
std::wstring PasteFromClipboard();  