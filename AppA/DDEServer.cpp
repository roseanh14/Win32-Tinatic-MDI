#include "DDEServer.h"
#include "OA2Window.h"
#include "resource.h"
#include <ddeml.h>     // DDE Management Library
#include <string>

static DWORD        s_serverInst = 0;       // Our DDE server instance ID
static HSZ          s_hszService = NULL;    // "TitanicDDE" as a DDE atom (fast string ID)
static HSZ          s_hszTopic = NULL;    // "Passengers" atom
static HSZ          s_hszItem = NULL;    // "Data" atom
static std::wstring s_currentData;          // Data the server is currently serving

// Windows calls this function automatically for every DDE event
// Think of it like a WndProc but for DDE messages
static HDDEDATA CALLBACK DdeCallback(
    UINT type, UINT fmt,
    HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR)
{
    if (type == XTYP_CONNECT)
        return (HDDEDATA)TRUE;  // Accept all incoming connections

    if (type == XTYP_REQUEST && fmt == CF_UNICODETEXT) {
        // A client is asking for our data - package it up and return it
        size_t bytes = (s_currentData.size() + 1) * sizeof(wchar_t);
        return DdeCreateDataHandle(
            s_serverInst,
            (LPBYTE)s_currentData.c_str(),
            (DWORD)bytes,
            0,
            s_hszItem,
            CF_UNICODETEXT,
            0
        );
    }

    return NULL;
}

bool DDEServerInit(HINSTANCE) {
    // Initialize as a standard DDE application (server)
    if (DdeInitializeW(&s_serverInst, DdeCallback, APPCLASS_STANDARD, 0) != DMLERR_NO_ERROR)
        return false;

    // Create string handles (atoms) - faster than passing raw strings in DDE
    s_hszService = DdeCreateStringHandleW(s_serverInst, DDE_SERVICE, CP_WINUNICODE);
    s_hszTopic = DdeCreateStringHandleW(s_serverInst, DDE_TOPIC, CP_WINUNICODE);
    s_hszItem = DdeCreateStringHandleW(s_serverInst, L"Data", CP_WINUNICODE);

    // Announce to Windows: "I am a DDE server named TitanicDDE"
    DdeNameService(s_serverInst, s_hszService, NULL, DNS_REGISTER);
    return true;
}

void DDEServerShutdown() {
    if (!s_serverInst) return;

    DdeNameService(s_serverInst, s_hszService, NULL, DNS_UNREGISTER);

    if (s_hszService) DdeFreeStringHandle(s_serverInst, s_hszService);
    if (s_hszTopic)   DdeFreeStringHandle(s_serverInst, s_hszTopic);
    if (s_hszItem)    DdeFreeStringHandle(s_serverInst, s_hszItem);

    DdeUninitialize(s_serverInst);

    s_hszService = NULL;
    s_hszTopic = NULL;
    s_hszItem = NULL;
    s_serverInst = 0;
}

void DDESendToOA2(const std::wstring& data) {
    s_currentData = data;  // Store data so the server can serve it

    // Create a SEPARATE client-only DDE instance to connect to our server
    // (a server instance cannot connect to itself)
    DWORD clientInst = 0;
    if (DdeInitializeW(&clientInst, DdeCallback, APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR) {
        SendDataToOA2(data); // fallback if client init failed
        return;
    }

    HSZ hSvc = DdeCreateStringHandleW(clientInst, DDE_SERVICE, CP_WINUNICODE);
    HSZ hTopic = DdeCreateStringHandleW(clientInst, DDE_TOPIC, CP_WINUNICODE);
    HSZ hItem = DdeCreateStringHandleW(clientInst, L"Data", CP_WINUNICODE);

    HCONV hConv = DdeConnect(clientInst, hSvc, hTopic, NULL);
    if (hConv) {
        // Ask the server for the data item
        HDDEDATA hResult = DdeClientTransaction(
            NULL, 0, hConv,
            hItem, CF_UNICODETEXT,
            XTYP_REQUEST, 3000, NULL
        );

        if (hResult) {
            DWORD len = 0;
            const wchar_t* ptr = (const wchar_t*)DdeAccessData(hResult, &len);
            if (ptr) {
                SendDataToOA2(std::wstring(ptr));
                DdeUnaccessData(hResult);
            }
            DdeFreeDataHandle(hResult);
        }

        DdeDisconnect(hConv);
    }
    else {
        SendDataToOA2(data); // Fallback: send directly if DDE connect failed
    }

    if (hSvc)   DdeFreeStringHandle(clientInst, hSvc);
    if (hTopic) DdeFreeStringHandle(clientInst, hTopic);
    if (hItem)  DdeFreeStringHandle(clientInst, hItem);

    DdeUninitialize(clientInst);
}