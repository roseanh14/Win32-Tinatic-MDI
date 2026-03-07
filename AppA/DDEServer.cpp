#include "DDEServer.h"
#include "OA2Window.h"
#include "resource.h"
#include <ddeml.h>
#include <string>

static DWORD        s_inst = 0;
static HSZ          s_hSvc = NULL;
static HSZ          s_hTopic = NULL;
static HSZ          s_hItem = NULL;
static std::wstring s_data;

static HDDEDATA CALLBACK DdeCb(UINT type, UINT fmt,
    HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR)
{
    if (type == XTYP_CONNECT) return (HDDEDATA)TRUE;
    if (type == XTYP_REQUEST && fmt == CF_UNICODETEXT) {
        size_t bytes = (s_data.size() + 1) * sizeof(wchar_t);
        return DdeCreateDataHandle(s_inst,
            (LPBYTE)s_data.c_str(), (DWORD)bytes,
            0, s_hItem, CF_UNICODETEXT, 0);
    }
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

void DDESetData(const std::wstring& data) {
    s_data = data;
}

// Spawn AppA.exe /ddeclient — fork of process A as DDE client
void DDESendToOA2(const std::wstring& data) {
    s_data = data; // server has the data ready

    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    std::wstring cmd = std::wstring(L"\"") + exePath + L"\" /ddeclient";

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    CreateProcessW(NULL, (LPWSTR)cmd.c_str(),
        NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread)  CloseHandle(pi.hThread);
}