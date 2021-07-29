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

bool odshr(HRESULT hr, PCWSTR fmt, ...)
{
  if (SUCCEEDED(hr))
  {
    return false;
  }
  WCHAR s[1024];
  int slen = 0;
  {
    WCHAR f[1024];
    va_list p;
    va_start(p, fmt);
    f[0] = L'\0';
    lstrcatW(f, L"textassist: ");
    lstrcatW(f, fmt);
    slen = wvsprintfW(s, f, p);
    va_end(p);
  }
  if (!slen)
  {
    OutputDebugStringW(L"textassist: wvsprintfW failed");
    return true;
  }

  LPWSTR msg = NULL;
  int msglen = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      hr,
      LANG_USER_DEFAULT,
      (LPWSTR)&msg,
      0,
      NULL);
  if (!msglen)
  {
    OutputDebugStringW(L"textassist: FormatMessageW failed");
    return true;
  }

  LPWSTR buf = realloc(NULL, sizeof(WCHAR) * (slen + msglen + 3));
  if (!buf)
  {
    OutputDebugStringW(L"textassist: failed to allocate buffer for error message");
    LocalFree(msg);
    return true;
  }
  memcpy(buf, s, slen * sizeof(WCHAR));
  buf[slen] = L':';
  buf[slen + 1] = L' ';
  memcpy(buf + slen + 2, msg, msglen * sizeof(WCHAR));
  buf[slen + 2 + msglen] = L'\0';
  LocalFree(msg);
  OutputDebugStringW(buf);
  free(buf);
  return true;
}
