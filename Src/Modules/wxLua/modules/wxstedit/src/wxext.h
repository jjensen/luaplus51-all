///////////////////////////////////////////////////////////////////////////////
// File:        wxext.h
// Purpose:     wxWidgets extensions
// Author:      Troels K
// Created:     2009-11-11
// RCS-ID:
// Copyright:   (c) John Labenski, Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WXEXT_H__
#define __WXEXT_H__

#define WXK_HELP       WXK_F1
#define WXK_FULLSCREEN WXK_F11

#include "wx/stedit/stedefs.h" // WXDLLIMPEXP_STEDIT

WX_DECLARE_OBJARRAY_WITH_DECL(wxFileName, wxArrayFileName, class WXDLLIMPEXP_STEDIT);

#ifdef _WX_INTL_H_
class WXDLLIMPEXP_STEDIT wxLocaleHelper
{
public:

    static bool Init(wxLocale*, const wxString& exetitle, enum wxLanguage lang = wxLANGUAGE_DEFAULT);

    /// Get an array of enum wxLanguage directories found.
    /// If the localeDir string is empty, the current app_dir/locale is searched.
    static size_t GetSupportedLanguages(wxArrayInt& languages,
                                        const wxString& localeDir = wxEmptyString);

    /// Show a dialog to choose a language from an array of enum wxLanguage.
    /// Returns false if the dialog is canceled and modifies lang on user selection.
    static bool SingleChoice(const wxArrayInt& languages, wxLanguage* selected_language);

    /// Find the enum wxLanguage corresponding to the localeName (de, fr, ...)
    /// Returns false if not found and lang is unmodified.
    static bool Find(const wxString& localeName, wxLanguage* found_language);
};
#endif

WXDLLIMPEXP_STEDIT void wxFrame_SetInitialPosition(wxFrame*,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, int margin_pct = 5);
WXDLLIMPEXP_STEDIT void wxFrame_ClonePosition(wxFrame* wnd, wxWindow* other = NULL);

WXDLLIMPEXP_STEDIT void wxCommandLineUsage(wxWindow* parent);

#if wxUSE_ACCEL
class WXDLLIMPEXP_FWD_CORE wxMenuBar;
WX_DECLARE_OBJARRAY_WITH_DECL(wxAcceleratorEntry, wxArrayAcceleratorEntry, class WXDLLIMPEXP_STEDIT);
class WXDLLIMPEXP_STEDIT wxAcceleratorHelper
{
public:

    static wxAcceleratorEntry GetStockAccelerator(wxWindowID);

    static void SetAcceleratorTable(wxWindow*, const wxArrayAcceleratorEntry&);

    static void SetAccelText(wxMenuBar*, const wxArrayAcceleratorEntry&);

    static void SetAccelText(wxMenu*, const wxArrayAcceleratorEntry&);
};
WXDLLIMPEXP_STEDIT wxString wxToolBarTool_MakeShortHelp(const wxString&, const wxArrayAcceleratorEntry&, int id);
#endif

#if (wxVERSION_NUMBER >= 2900)
#define wxMessageBoxCaption wxTheApp->GetAppDisplayName()
#else
#define wxMessageBoxCaption wxTheApp->GetAppName()
#endif

#ifdef _WX_STOCKITEM_H_
#if (wxVERSION_NUMBER < 2901)
#define wxSTOCK_WITHOUT_ELLIPSIS 4
#define wxSTOCK_FOR_BUTTON (wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITHOUT_ELLIPSIS)
#endif
#define wxSTOCK_PLAINTEXT wxSTOCK_WITHOUT_ELLIPSIS
WXDLLIMPEXP_STEDIT wxString wxGetStockLabelEx(wxWindowID, long flags = wxSTOCK_WITH_MNEMONIC);
#endif

/////////////////////////////////////////////////////////////////////////////
// wxPreviewFrameEx

#ifdef _WX_PRNTBASEH__
class WXDLLIMPEXP_STEDIT wxPreviewFrameEx : public wxPreviewFrame
{
   typedef wxPreviewFrame base;
public:
   wxPreviewFrameEx(wxPrintPreviewBase* preview,
                   wxWindow *parent,
                   const wxString& title,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxDEFAULT_FRAME_STYLE,
                   const wxString& name = wxT("frame"));
   virtual bool Destroy();
protected:
#if (wxVERSION_NUMBER < 2900)
   void OnKeyDown(wxKeyEvent&);
#endif
   DECLARE_EVENT_TABLE()
};
#endif

class WXDLLIMPEXP_STEDIT wxClipboardHelper
{
public:
    enum Clipboard_Type
    {
        CLIPBOARD_DEFAULT = 1, // use the normal clipboard
        CLIPBOARD_PRIMARY = 2, // use the primary clipboard
        CLIPBOARD_BOTH    = 3  // use both clipboards (only valid for set functions)
    };

    // Is text available in the single specified clipboard in any usable text format.
    // Formats tested are wxDF_TEXT and if avilable wxDF_UNICODETEXT and wxDF_HTML.
    static bool IsTextAvailable(Clipboard_Type clip_type = CLIPBOARD_DEFAULT);

    // Returns true if there is data in the single specified clipboard with the given formats.
    // This function takes an array since the clipboard has to be opened to test formats.
    static bool IsFormatAvailable(const enum wxDataFormatId* array, size_t array_count, Clipboard_Type clip_type = CLIPBOARD_DEFAULT);

    // Get the current text in the single specified clipboard into the buf.
    // Returns true if the clipboard was opened and the buf is not empty.
    static bool GetText(wxString* buf, Clipboard_Type clip_type = CLIPBOARD_DEFAULT);

    // Set the text to the specified clipboard(s).
    static bool SetText(const wxString& str, Clipboard_Type clip_type = CLIPBOARD_DEFAULT);

    // Set the HTML text to the clipboard. In MSW the clipboard will contain
    // a valid HTML data object and a text object, on other systems the
    // clipboard only contains a text object.
    static bool SetHtmlText(const wxString& htmldata);

    static bool Set(wxDataObject* def, wxDataObject* primary = NULL);
};

#ifdef __WXMSW__
// Strange that wxMBConv classes work with char and wchar_t only, not with wxChar;
// this surely makes for unnecessary extra conversions
class WXDLLIMPEXP_STEDIT wxMBConvOEM : public wxMBConv
{
public:
    wxMBConvOEM();
    virtual ~wxMBConvOEM();

    virtual size_t ToWChar  (wchar_t*    dst, size_t dstLen,
                             const char* src, size_t srcLen = wxNO_LEN) const;
    virtual size_t FromWChar(char*          dst, size_t dstLen,
                             const wchar_t* src, size_t srcLen = wxNO_LEN) const;

    virtual wxMBConv* Clone() const { return new wxMBConvOEM(); }
};
#endif

class WXDLLIMPEXP_STEDIT wxTextEncoding
{
public:
    enum TextEncoding_Type
    {
        None  = 0,
        Ascii = 0,
        UTF8,
        Unicode_LE,
        ISO8859_1,
    #ifdef __WXMSW__
        OEM,
    #endif
        TextEncoding__Count
    };

#if (defined(__WXTRUNK_H__) || (wxVERSION_NUMBER >= 2903) ) && defined(_WX_CONVAUTO_H_) // wxBOM enum is in wx/convauto.h
    // char -> wxString method. Specify encoding.
    static bool CharToString(wxString*, const wxCharBuffer& buf, size_t buf_len = wxNO_LEN,
                             TextEncoding_Type encoding = Ascii, wxBOM bom = wxBOM_None);

    // char -> wxString method. Utilizing wxConvAuto::DetectBOM.
    static bool CharToStringDetectBOM(wxString*, const wxCharBuffer& buf, size_t buf_len = wxNO_LEN,
                                      wxBOM* file_bom = NULL);
#endif

    // char -> wxString method. Specify wxMBConv conversion class
    static bool CharToString(wxString* dst_wxstr, const char* src_str,
                             const wxMBConv& conv, size_t len);

    // wxString -> char method. Specify wxMBConv conversion class
    static wxCharBuffer StringToChar(const wxString& src_wxstr, const wxMBConv& conv);

    // wxString -> char method. Specify TextEncoding_Type conversion;
    // Creates wxMBConv instance and calls StringToChar(wxMBConv) above.
    // The size of the returned buffer is optionally returned in buffer_size.
    static wxCharBuffer StringToChar(const wxString& src_wxstr, TextEncoding_Type encoding_type,
                                     size_t* buffer_size);

    // enum -> string representation (eg TextEncoding_Type::UTF8 -> "utf-8")
    static wxString TypeToString(TextEncoding_Type encoding_type);

    // String representation -> enum (eg "utf-8" -> TextEncoding_Type::UTF8)
    static TextEncoding_Type TypeFromString(const wxString& encoding_name);

    // Search the provided string for encoding specification (eg "utf-8")
    static bool TypeFromString(TextEncoding_Type* encoding_type,
                               const char* str,
                               const char* identifier, const char* strpbrk_ctrl);

#ifdef _WX_XML_H_
    inline static TextEncoding_Type TypeFromString(const wxXmlDocument& xml)
    {
        return TypeFromString(xml.GetFileEncoding());
    }
#endif

    static const char* GetBOMChars(TextEncoding_Type, size_t* count);

    static bool SaveFile(const wxString&, wxOutputStream&, TextEncoding_Type encoding = Ascii, bool file_bom = false);
};

#endif // __WXEXT_H__
