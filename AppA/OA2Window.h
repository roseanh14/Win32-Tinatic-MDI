#pragma once
#include <windows.h>
#include <string>

bool RegisterOA2Class(HINSTANCE hInst);
HWND CreateOA2Window(HWND hMDIClient);
HWND GetOA2Window();
void SendDataToOA2(const std::wstring& data);
void PasteFromClipboardToOA2();