#include "PipeLauncher.h"
#include "resource.h"
#include <string>

void LaunchAppBWithPipe(const std::wstring& data) {
    // SECURITY_ATTRIBUTES with bInheritHandle = TRUE
    // means the child process (AppB) can use the pipe handles
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    HANDLE hReadEnd  = NULL;   // AppB reads from this end
    HANDLE hWriteEnd = NULL;   // We write to this end

    if (!CreatePipe(&hReadEnd, &hWriteEnd, &sa, 0)) {
        MessageBoxW(NULL, L"Failed to create pipe!", L"Error", MB_ICONERROR);
        return;
    }

    // Make the WRITE end non-inheritable (AppB only needs the read end)
    SetHandleInformation(hWriteEnd, HANDLE_FLAG_INHERIT, 0);

    // Find AppB.exe in the same folder as this executable
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) wcscpy_s(lastSlash + 1, MAX_PATH - (lastSlash - exePath + 1), APPB_EXE);

    // STARTUPINFO controls how the child process window appears
    // We redirect its stdin to our pipe's read end
    STARTUPINFOW si        = { sizeof(si) };
    si.dwFlags             = STARTF_USESTDHANDLES;
    si.hStdInput           = hReadEnd;
    si.hStdOutput          = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError           = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessW(
        exePath,        // Path to AppB.exe
        NULL,           // Command line arguments (none needed)
        NULL, NULL,     // Process/thread security (use defaults)
        TRUE,           // bInheritHandles = TRUE: child gets our pipe handle
        0,              // Creation flags
        NULL, NULL,     // Environment and working directory (inherit ours)
        &si, &pi);

    if (!ok) {
        MessageBoxW(NULL,
            L"Could not launch AppB.exe!\nMake sure it is in the same folder as AppA.exe.",
            L"Error", MB_ICONERROR);
        CloseHandle(hReadEnd);
        CloseHandle(hWriteEnd);
        return;
    }

    // Convert our wide string to UTF-8 bytes to write into the pipe
    int len = WideCharToMultiByte(CP_UTF8, 0, data.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, data.c_str(), -1, &utf8[0], len, NULL, NULL);

    DWORD written = 0;
    WriteFile(hWriteEnd, utf8.c_str(), (DWORD)utf8.size(), &written, NULL);

    // Close our write end - this sends EOF to AppB so it knows we're done
    CloseHandle(hWriteEnd);
    CloseHandle(hReadEnd);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}