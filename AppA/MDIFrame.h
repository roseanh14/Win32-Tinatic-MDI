#pragma once
#include <windows.h>

bool RegisterFrameClass(HINSTANCE hInst);
HWND CreateFrameWindow(HINSTANCE hInst);
HWND GetMDIClient();