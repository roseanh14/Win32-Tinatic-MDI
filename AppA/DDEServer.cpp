#include "DDEServer.h"
#include "OA2Window.h"
#include "resource.h"
#include <ddeml.h>
#include <string>

// Tell linker: DDE functions live in user32 in modern Windows SDK
#pragma comment(lib, "user32.lib")

static DWORD        s_inst = 0;
static HSZ          s_hSvc = NULL;
static HSZ          s_hTopic = NULL;
static HSZ          s_hItem = NULL;
static std::wstring s_data;

static HDDEDATA CALLBACK DdeCb(UINT type, UINT fmt,
    HCONV, HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG_PTR, ULONG_PTR)
{
    if (type == XTYP_CONNECT)
        return (HDDEDATA)TRUE;

    if (type == XTYP_REQUEST && fmt == CF_UNICODETEXT) {
        size_t bytes = (s_data.size() + 1) * sizeof(wchar_t);
        return DdeCreateDataHandle(s_inst,
            (LPBYTE)s_data.c_str(), (DWORD)bytes,
            0, s_hItem, CF_UNICODETEXT, 0);
    }
    (void)hsz1; (void)hsz2; (void)hData;
    return NULL;
}

bool DDEServerInit(HINSTANCE) {
    if (DdeInitializeW(&s_inst, DdeCb, APPCLASS_STANDARD, 0) != DMLERR_NO_ERROR)
        return false;
    s_hSvc = DdeCreateStringHandleW(s_inst, DDE_SERVICE, CP_WINUNICODE);
    s_hTopic = DdeCreateStringHandleW(s_inst, DDE_TOPIC, CP_WINUNICODE);
    s_hItem = DdeCreateStringHandleW(s_inst, L"Data", CP_WINUNICODE);
    DdeNameService(s_inst, s_hSvc, NULL, DNS_REGISTER);
    return true;
}

void DDEServerShutdown() {
    if (!s_inst) return;
    DdeNameService(s_inst, s_hSvc, NULL, DNS_UNREGISTER);
    if (s_hSvc)   DdeFreeStringHandle(s_inst, s_hSvc);
    if (s_hTopic) DdeFreeStringHandle(s_inst, s_hTopic);
    if (s_hItem)  DdeFreeStringHandle(s_inst, s_hItem);
    DdeUninitialize(s_inst);
    s_inst = 0;
}

void DDESendToOA2(const std::wstring& data) {
    s_data = data;

    // Connect as client to our own server and request the data
    DWORD clientInst = 0;
    if (DdeInitializeW(&clientInst, DdeCb, APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR) {
        SendDataToOA2(data); return;
    }

    HSZ hSvc = DdeCreateStringHandleW(clientInst, DDE_SERVICE, CP_WINUNICODE);
    HSZ hTopic = DdeCreateStringHandleW(clientInst, DDE_TOPIC, CP_WINUNICODE);
    HSZ hItem = DdeCreateStringHandleW(clientInst, L"Data", CP_WINUNICODE);

    HCONV hConv = DdeConnect(clientInst, hSvc, hTopic, NULL);
    if (hConv) {
        HDDEDATA hResult = DdeClientTransaction(NULL, 0, hConv,
            hItem, CF_UNICODETEXT, XTYP_REQUEST, 3000, NULL);
        if (hResult) {
            DWORD len = 0;
            const wchar_t* ptr = (const wchar_t*)DdeAccessData(hResult, &len);
            if (ptr) SendDataToOA2(std::wstring(ptr));
            DdeUnaccessData(hResult);
            DdeFreeDataHandle(hResult);
        }
        else {
            SendDataToOA2(data); // fallback
        }
        DdeDisconnect(hConv);
    }
    else {
        SendDataToOA2(data); // fallback
    }

    DdeFreeStringHandle(clientInst, hSvc);
    DdeFreeStringHandle(clientInst, hTopic);
    DdeFreeStringHandle(clientInst, hItem);
    DdeUninitialize(clientInst);
}