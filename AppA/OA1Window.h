#pragma once
#include <windows.h>

// OA1 is the "data source" window
// It shows the Titanic data and has a menu to send it different ways

bool RegisterOA1Class(HINSTANCE hInst);
HWND CreateOA1Window(HWND hMDIClient);
HWND GetOA1Window();   // Other files need this to forward messages