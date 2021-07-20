#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "aviutl.h"
#include "textassist.h"

FILTER_DLL *filter_list[] = {&textassist_filter, NULL};

EXTERN_C FILTER_DLL __declspec(dllexport) * *__stdcall GetFilterTableList(void)
{
  return (FILTER_DLL **)&filter_list;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  (void)hinstDLL;
  (void)lpvReserved;
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    break;

  case DLL_PROCESS_DETACH:
    break;

  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}
