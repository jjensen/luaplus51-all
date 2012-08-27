///////////////////////////////////////////////////////////////////////////////
// File:        wxtrunk.h
// Purpose:     wx28/wx29 compatibility
// Author:      Troels K
// Created:     2011-10-28
// RCS-ID:
// Copyright:   (c) John Labenski, Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WXTRUNK_H__
#define __WXTRUNK_H__

#if (wxVERSION_NUMBER < 2812)
    #define wxT_2 wxT
#endif

#ifndef wxAPPLY
    #define wxAPPLY 0x00000020
#endif

#ifdef _WX_CONVAUTO_H_
#if (wxVERSION_NUMBER >= 2903)
inline wxBOM wxConvAuto_DetectBOM(const char *src, size_t srcLen)
{
    return wxConvAuto::DetectBOM(src, srcLen);
}
inline const char* wxConvAuto_GetBOMChars(wxBOM bomType, size_t* count)
{
    return wxConvAuto::GetBOMChars(bomType, count);
}
#else
enum wxBOM
{
    wxBOM_Unknown = -1,
    wxBOM_None,
    wxBOM_UTF32BE,
    wxBOM_UTF32LE,
    wxBOM_UTF16BE,
    wxBOM_UTF16LE,
    wxBOM_UTF8
};
WXDLLIMPEXP_STEDIT wxBOM wxConvAuto_DetectBOM(const char *src, size_t srcLen);
WXDLLIMPEXP_STEDIT const char* wxConvAuto_GetBOMChars(wxBOM, size_t* count);
#endif
#endif

#if defined(_WX_ABOUTDLG_H_) && (wxVERSION_NUMBER < 2900)
inline void wxAboutBox(const wxAboutDialogInfo& info, wxWindow* WXUNUSED(parent))
{
   wxAboutBox(info);
}
#endif

inline size_t wxBuffer_length(const wxCharBuffer& buf)
{
#if (wxVERSION_NUMBER >= 2900)
    return buf.length(); // wxCharBuffer.length() not available in wx28
#else
    return buf.data() ? strlen(buf.data()) : 0;
#endif
}

inline size_t wxBuffer_length(const wxWCharBuffer& buf)
{
#if (wxVERSION_NUMBER >= 2900)
    return buf.length(); // wxWCharBuffer.length() not available in wx28
#else
    return buf.data() ? wcslen(buf.data()) : 0;
#endif
}

#endif // __WXTRUNK_H__
