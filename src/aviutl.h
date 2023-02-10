#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __GNUC__
#  ifndef __has_warning
#    define __has_warning(x) 0
#  endif
#  pragma GCC diagnostic push
#  if __has_warning("-Winvalid-utf8")
#    pragma GCC diagnostic ignored "-Winvalid-utf8"
#  endif
#endif // __GNUC__
#include "aviutl_sdk/filter.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif // __GNUC__
