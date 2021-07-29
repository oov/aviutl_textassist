#include "ods.h"

#include <stdarg.h>

void ods(PCWSTR fmt, ...)
{
  WCHAR s[1024], f[1024];
  va_list p;
  va_start(p, fmt);
  f[0] = L'\0';
  lstrcatW(f, L"textassist: ");
  lstrcatW(f, fmt);
  wvsprintfW(s, f, p);
  OutputDebugStringW(s);
  va_end(p);
}
