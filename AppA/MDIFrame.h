#pragma once
#include <windows.h>

bool RegisterMDIFrameClass(HINSTANCE hInst);
HWND CreateMDIFrameWindow(HINSTANCE hInst, int nCmdShow);
HWND GetMDIClient();  