#pragma once
#include <windows.h>
#include <string>

// Real DDE server using DDEML API
// Service: "TitanicDDE"  Topic: "Passengers"  Item: "Data"

bool DDEServerInit(HINSTANCE hInst);
void DDEServerShutdown();
void DDESendToOA2(const std::wstring& data);