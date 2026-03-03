#pragma once
#include <windows.h>
#include <string>

// OA2 is the "receiver" window - it just displays data sent to it
// It can receive data three ways: direct call, clipboard paste, or DDE

bool RegisterOA2Class(HINSTANCE hInst);    // Tell Windows about this window type
HWND CreateOA2Window(HWND hMDIClient);     // Actually create the window inside MDI
HWND GetOA2Window();                       // Get the window handle from anywhere

void SendDataToOA2(const std::wstring& data);  // Send data directly to OA2
void PasteFromClipboardToOA2();                // Read clipboard and display in OA2