#include "ods.h"

#include <stdarg.h>
#include <stdbool.h>

void ods(wchar_t *fmt, ...) {
  wchar_t s[1024], f[1024];
  va_list p;
  va_start(p, fmt);
  wcscpy(f, L"textassist: ");
  wcscat(f, fmt);
  wvsprintfW(s, f, p);
  OutputDebugStringW(s);
  va_end(p);
}

bool odshr(HRESULT hr, wchar_t *fmt, ...) {
  if (SUCCEEDED(hr)) {
    return false;
  }
  wchar_t s[1024];
  size_t slen = 0;
  {
    wchar_t f[1024];
    va_list p;
    va_start(p, fmt);
    wcscpy(f, L"textassist: ");
    wcscat(f, fmt);
    slen = (size_t)wvsprintfW(s, f, p);
    va_end(p);
  }
  if (!slen) {
    OutputDebugStringW(L"textassist: wvsprintfW failed");
    return true;
  }

  wchar_t *msg = NULL;
  DWORD msglen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                (DWORD)hr,
                                LANG_USER_DEFAULT,
                                (LPWSTR)&msg,
                                0,
                                NULL);
  if (!msglen) {
    OutputDebugStringW(L"textassist: FormatMessageW failed");
    return true;
  }

  LPWSTR buf = realloc(NULL, sizeof(wchar_t) * (slen + msglen + 3));
  if (!buf) {
    OutputDebugStringW(L"textassist: failed to allocate buffer for error message");
    LocalFree(msg);
    return true;
  }
  memcpy(buf, s, slen * sizeof(wchar_t));
  buf[slen] = L':';
  buf[slen + 1] = L' ';
  memcpy(buf + slen + 2, msg, msglen * sizeof(wchar_t));
  buf[slen + 2 + msglen] = L'\0';
  LocalFree(msg);
  OutputDebugStringW(buf);
  free(buf);
  return true;
}
