#pragma once
#include <windows.h>
#include <string>

bool DDEServerInit(HINSTANCE hInst);
void DDEServerShutdown();
void DDESetData(const std::wstring& data);
void DDESendToOA2(const std::wstring& data);

int RunAsDDEClient();