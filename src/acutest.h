#pragma once

#ifdef __GNUC__
#  ifndef __has_warning
#    define __has_warning(x) 0
#  endif
#  pragma GCC diagnostic push
#  if __has_warning("-Wswitch-enum")
#    pragma GCC diagnostic ignored "-Wswitch-enum"
#  endif
#  if __has_warning("-Wmissing-prototypes")
#    pragma GCC diagnostic ignored "-Wmissing-prototypes"
#  endif
#  if __has_warning("-Wsign-conversion")
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#  endif
#endif // __GNUC__
#include "3rd/acutest/include/acutest.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif // __GNUC__
