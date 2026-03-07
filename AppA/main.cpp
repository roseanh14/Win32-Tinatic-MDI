#include <windows.h>
#include "MDIFrame.h"
#include "OA1Window.h"
#include "OA2Window.h"
#include "DDEServer.h"
#include "resource.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, int nCmdShow) {
    if (strstr(lpCmdLine, "/ddeclient")) {
        return RunAsDDEClient();
    }

    DDEServerInit(hInst);
    RegisterFrameClass(hInst);
    RegisterOA1Class(hInst);
    RegisterOA2Class(hInst);
    HWND hwnd = CreateFrameWindow(hInst);
    if (!hwnd) return 1;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateMDISysAccel(GetMDIClient(), &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    DDEServerShutdown();
    return (int)msg.wParam;
}