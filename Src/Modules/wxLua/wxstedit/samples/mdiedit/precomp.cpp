// precomp.cpp

#include "precomp.h"

#ifdef _MSC_VER
#if wxUSE_UNICODE
#define MODE "Unicode"
#else
#define MODE "Ansi"
#endif

#pragma message("Compiling using wx "wxVERSION_NUM_DOT_STRING" "MODE"...")
#endif
