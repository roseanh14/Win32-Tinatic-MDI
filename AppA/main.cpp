
#include <windows.h>
#include "resource.h"
#include "MDIFrame.h"
#include "OA1Window.h"
#include "OA2Window.h"
#include "TitanicData.h"
#include "ClipboardHelper.h"
#include "DDEServer.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    if (!TitanicData::LoadCSV(L"data\\titanic.csv")) {
        MessageBoxW(NULL,
            L"Cannot find  data\\titanic.csv\n\n"
            L"Put titanic.csv inside a 'data' folder next to AppA.exe",
            L"Startup Error", MB_ICONERROR);
        return 1;
    }

    InitClipboardFormat();

    DDEServerInit(hInstance);

    if (!RegisterMDIFrameClass(hInstance) ||
        !RegisterOA1Class(hInstance)      ||
        !RegisterOA2Class(hInstance)) {
        MessageBoxW(NULL, L"Failed to register window classes!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hFrame = CreateMDIFrameWindow(hInstance, nCmdShow);
    if (!hFrame) {
        MessageBoxW(NULL, L"Failed to create main window!", L"Error", MB_ICONERROR);
        return 1;
    }

  
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