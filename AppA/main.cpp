#include <windows.h>
#include "MDIFrame.h"
#include "OA1Window.h"
#include "OA2Window.h"
#include "TitanicData.h"
#include "DDEServer.h"
#include "resource.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    // Load Titanic data from CSV next to the exe
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring path(exePath);
    path = path.substr(0, path.rfind(L'\\') + 1) + L"data\\titanic.csv";
    TitanicData::LoadCSV(path);

    // Init DDE server
    DDEServerInit(hInst);

    // Register window classes
    RegisterFrameClass(hInst);
    RegisterOA1Class(hInst);
    RegisterOA2Class(hInst);

    // Create and show main window
    HWND hwnd = CreateFrameWindow(hInst);
    if (!hwnd) return 1;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
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