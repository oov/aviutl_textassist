#pragma once

#include <stdbool.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void ods(PCWSTR fmt, ...);
bool odshr(HRESULT hr, PCWSTR fmt, ...);
