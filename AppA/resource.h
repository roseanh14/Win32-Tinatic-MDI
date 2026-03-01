#pragma once

// Menu Commands
#define IDM_DATA_DISPLAY         1001
#define IDM_DATA_SEND_OA2        1002
#define IDM_DATA_SEND_CLIPBOARD  1003
#define IDM_DATA_PASTE_OA2       1004
#define IDM_DATA_SEND_PIPE       1005
#define IDM_DATA_SEND_DDE        1006

// Control IDs
#define IDC_OA1_LISTBOX          2001
#define IDC_OA2_LISTBOX          2002

// Window class names
#define FRAME_CLASS              L"TitanicMDIFrame"
#define OA1_CLASS                L"TitanicOA1"
#define OA2_CLASS                L"TitanicOA2"

// App strings
#define APP_TITLE                L"Titanic MDI - App A"
#define DDE_SERVICE              L"TitanicDDE"
#define DDE_TOPIC                L"Passengers"
#define CLIPBOARD_FORMAT_NAME    L"TitanicPassengerTable"
#define APPB_EXE                 L"AppB.exe"