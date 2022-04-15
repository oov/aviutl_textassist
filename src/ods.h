#pragma once

#include <stdbool.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void ods(wchar_t *fmt, ...);
bool odshr(HRESULT hr, wchar_t *fmt, ...);
