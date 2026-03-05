#pragma once

#define IDM_OA1_LOAD_CSV         1001
#define IDM_OA1_DISPLAY          1002
#define IDM_OA1_SEND_OA2         1003
#define IDM_OA1_CLIPBOARD        1004
#define IDM_OA1_PIPE             1005
#define IDM_OA1_DDE              1006

#define IDM_OA2_PASTE_CLIP       2001
#define IDM_OA2_LOAD_DDE         2002

#define IDC_OA1_LISTBOX          3001
#define IDC_OA2_LISTBOX          3002
#define IDC_DATA_EDIT            3003

#define FRAME_CLASS              L"TitanicMDIFrame"
#define OA1_CLASS                L"TitanicOA1"
#define OA2_CLASS                L"TitanicOA2"

#define APP_TITLE                L"Titanic MDI - App A"
#define DDE_SERVICE              L"TitanicDDE"
#define DDE_TOPIC                L"Passengers"
#define CLIPBOARD_FORMAT_NAME    L"TitanicPassengerTable"
#define APPB_EXE                 L"AppB.exe"
#define APPB_TITLE               L"Titanic AppB - Pipe Receiver"