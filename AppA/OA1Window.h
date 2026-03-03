#pragma once
#include <windows.h>

bool RegisterOA1Class(HINSTANCE hInst);
HWND CreateOA1Window(HWND hMDIClient);
HWND GetOA1Window();