// link.cpp
// Copyright (c) 2009-2010 by Troels K. All rights reserved.
// License: wxWindows

#ifdef _MSC_VER

#include <wx/wx.h>

#if (wxVERSION_NUMBER >= 2900)
   #if wxUSE_UNICODE
      #ifdef __WXDEBUG__
         #pragma comment(lib, "wxbase29ud.lib")
         #pragma comment(lib, "wxmsw29ud_html.lib")
         #pragma comment(lib, "wxmsw29ud_core.lib")
         #pragma comment(lib, "wxmsw29ud_stc.lib")
         #pragma comment(lib, "wxmsw29ud_adv.lib")
         #pragma comment(lib, "wxmsw29ud_adv.lib")
      #else
         #pragma comment(lib, "wxbase29u.lib")
         #pragma comment(lib, "wxmsw29u_html.lib")
         #pragma comment(lib, "wxmsw29u_core.lib")
         #pragma comment(lib, "wxmsw29u_stc.lib")
         #pragma comment(lib, "wxmsw29u_adv.lib")
      #endif
   #else
      #ifdef __WXDEBUG__
         #pragma comment(lib, "wxbase29d.lib")
         #pragma comment(lib, "wxmsw29d_html.lib")
         #pragma comment(lib, "wxmsw29d_core.lib")
         #pragma comment(lib, "wxmsw29d_stc.lib")
         #pragma comment(lib, "wxmsw29d_adv.lib")
      #else
         #pragma comment(lib, "wxbase29.lib")
         #pragma comment(lib, "wxmsw29_html.lib")
         #pragma comment(lib, "wxmsw29_core.lib")
         #pragma comment(lib, "wxmsw29_stc.lib")
         #pragma comment(lib, "wxmsw29_adv.lib")
      #endif
   #endif
#elif (wxVERSION_NUMBER >= 2800)
   #if wxUSE_UNICODE
      #ifdef __WXDEBUG__
         #pragma comment(lib, "wxbase28ud.lib")
         #pragma comment(lib, "wxmsw28ud_html.lib")
         #pragma comment(lib, "wxmsw28ud_core.lib")
         #pragma comment(lib, "wxmsw28ud_stc.lib")
         #pragma comment(lib, "wxmsw28ud_adv.lib")
      #else
         #pragma comment(lib, "wxbase28u.lib")
         #pragma comment(lib, "wxmsw28u_html.lib")
         #pragma comment(lib, "wxmsw28u_core.lib")
         #pragma comment(lib, "wxmsw28u_stc.lib")
         #pragma comment(lib, "wxmsw28u_adv.lib")
      #endif
   #else
      #ifdef __WXDEBUG__
         #pragma comment(lib, "wxbase28d.lib")
         #pragma comment(lib, "wxmsw28d_html.lib")
         #pragma comment(lib, "wxmsw28d_core.lib")
         #pragma comment(lib, "wxmsw28d_stc.lib")
         #pragma comment(lib, "wxmsw28d_adv.lib")
      #else
         #pragma comment(lib, "wxbase28.lib")
         #pragma comment(lib, "wxmsw28_html.lib")
         #pragma comment(lib, "wxmsw28_core.lib")
         #pragma comment(lib, "wxmsw28_stc.lib")
         #pragma comment(lib, "wxmsw28_adv.lib")
      #endif
   #endif
#endif

#endif
