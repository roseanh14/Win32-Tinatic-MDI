#pragma once
#include <windows.h>
#include <string>

bool DDEServerInit(HINSTANCE hInst);
void DDEServerShutdown();
void DDESendToOA2(const std::wstring& data);