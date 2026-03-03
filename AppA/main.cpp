// =============================================================
//  AppA — Entry Point
//  WinMain is Windows' version of main()
// =============================================================
#include <windows.h>
#include "resource.h"
#include "MDIFrame.h"
#include "OA1Window.h"
#include "OA2Window.h"
#include "TitanicData.h"
#include "ClipboardHelper.h"
#include "DDEServer.h"

// HINSTANCE  = a handle (ID) representing this running program
// nCmdShow   = how to display the window (normal, maximized, etc.)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    // ---- 1. Load CSV data ----
    // Try next to .exe first, then in a data subfolder
    if (!TitanicData::LoadCSV(L"data\\titanic.csv")) {
        MessageBoxW(NULL,
            L"Cannot find  data\\titanic.csv\n\n"
            L"Put titanic.csv inside a 'data' folder next to AppA.exe",
            L"Startup Error", MB_ICONERROR);
        return 1;
    }

    // ---- 2. Register our custom clipboard format ----
    InitClipboardFormat();

    // ---- 3. Start the DDE server ----
    DDEServerInit(hInstance);

    // ---- 4. Register all window classes ----
    // Windows needs to know about each window type before we create them
    if (!RegisterMDIFrameClass(hInstance) ||
        !RegisterOA1Class(hInstance)      ||
        !RegisterOA2Class(hInstance)) {
        MessageBoxW(NULL, L"Failed to register window classes!", L"Error", MB_ICONERROR);
        return 1;
    }

    // ---- 5. Create the main window ----
    HWND hFrame = CreateMDIFrameWindow(hInstance, nCmdShow);
    if (!hFrame) {
        MessageBoxW(NULL, L"Failed to create main window!", L"Error", MB_ICONERROR);
        return 1;
    }

    // ---- 6. The Message Loop ----
    // Windows is event-driven: it tells us things via "messages"
    // GetMessage waits for the next message, DispatchMessage sends it
    // to the right window's procedure function.  Loop until app closes.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        // TranslateMDISysAccel handles MDI keyboard shortcuts (Ctrl+F4 etc.)
        if (!TranslateMDISysAccel(GetMDIClient(), &msg)) {
            TranslateMessage(&msg);   // Converts key codes → characters
            DispatchMessage(&msg);    // Sends message to the window procedure
        }
    }

    // ---- 7. Cleanup ----
    DDEServerShutdown();
    return (int)msg.wParam;
}