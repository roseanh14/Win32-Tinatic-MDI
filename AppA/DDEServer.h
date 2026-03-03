#pragma once
#include <windows.h>
#include <string>

// --- DDE = Dynamic Data Exchange ---
// An old but standard Windows IPC (inter-process communication) protocol
// We act as a SERVER that holds data, and connect as CLIENT to fetch it into OA2

bool DDEServerInit(HINSTANCE hInst); // Start the DDE server (call once)
void DDEServerShutdown();            // Clean up when app closes
void DDESendToOA2(const std::wstring& data); // Store data + send to OA2 via DDE