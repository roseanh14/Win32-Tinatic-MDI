#pragma once
#include <windows.h>

// The MDI Frame is the OUTER window - the container
// Inside it is the "MDI Client" which holds OA1 and OA2 as child windows
// MDI = Multiple Document Interface (windows inside a window)

bool RegisterMDIFrameClass(HINSTANCE hInst);
HWND CreateMDIFrameWindow(HINSTANCE hInst, int nCmdShow);
HWND GetMDIClient();   // Other files need this for the message loop