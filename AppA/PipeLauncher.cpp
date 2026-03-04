#include "PipeLauncher.h"
#include "resource.h"
#include <string>

void LaunchAppBWithPipe(const std::wstring& data) {
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    HANDLE hReadEnd  = NULL;   
    HANDLE hWriteEnd = NULL;   

    if (!CreatePipe(&hReadEnd, &hWriteEnd, &sa, 0)) {
        MessageBoxW(NULL, L"Failed to create pipe!", L"Error", MB_ICONERROR);
        return;
    }

    SetHandleInformation(hWriteEnd, HANDLE_FLAG_INHERIT, 0);

    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) wcscpy_s(lastSlash + 1, MAX_PATH - (lastSlash - exePath + 1), APPB_EXE);

    STARTUPINFOW si        = { sizeof(si) };
    si.dwFlags             = STARTF_USESTDHANDLES;
    si.hStdInput           = hReadEnd;
    si.hStdOutput          = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError           = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessW(
        exePath,        
        NULL,           
        NULL, NULL,     
        TRUE,           
        0,             
        NULL, NULL,     
        &si, &pi);

    if (!ok) {
        MessageBoxW(NULL,
            L"Could not launch AppB.exe!\nMake sure it is in the same folder as AppA.exe.",
            L"Error", MB_ICONERROR);
        CloseHandle(hReadEnd);
        CloseHandle(hWriteEnd);
        return;
    }

    int len = WideCharToMultiByte(CP_UTF8, 0, data.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, data.c_str(), -1, &utf8[0], len, NULL, NULL);

    DWORD written = 0;
    WriteFile(hWriteEnd, utf8.c_str(), (DWORD)utf8.size(), &written, NULL);

    CloseHandle(hWriteEnd);
    CloseHandle(hReadEnd);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}