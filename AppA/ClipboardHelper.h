#pragma once
#include <windows.h>
#include <string>

// --- Clipboard functions ---
// The clipboard is Windows' built-in copy/paste system
// We register a CUSTOM format so only our app can read our special table data

void         InitClipboardFormat();                     // Register our format (call once)
bool         CopyToClipboard(const std::wstring& data); // Put data into clipboard
std::wstring PasteFromClipboard();                      // Get our data back from clipboard