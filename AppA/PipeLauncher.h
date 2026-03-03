#pragma once
#include <windows.h>
#include <string>

// A pipe is like a one-way tube between two programs
// We create the pipe, write Titanic data into it,
// and AppB reads from the other end

void LaunchAppBWithPipe(const std::wstring& data);