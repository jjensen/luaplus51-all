///////////////////////////////////////////////////////////////////////////////
// File:        wxext.cpp
// Purpose:     wxWidgets extensions
// Author:      Troels K
// Created:     2009-11-11
// RCS-ID:
// Copyright:   (c) John Labenski, Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/cmdline.h>
#include <wx/clipbrd.h>
#include <wx/convauto.h>

#include "wx/stedit/stedefs.h"

#include "wxtrunk.h"
#include "wxext.h"

#ifndef WXPRECOMP
    #include <wx/settings.h>
    #include <wx/app.h>
    #include <wx/choicdlg.h>
#endif // WXPRECOMP

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxArrayAcceleratorEntry);
WX_DEFINE_OBJARRAY(wxArrayFileName);

bool wxGetExeFolder(wxFileName* filename)
{
    wxFileName temp;

    temp.Assign(wxStandardPaths::Get().GetExecutablePath());
    temp.SetFullName(wxEmptyString);

    bool ok = temp.IsOk();

    if (ok && filename)
    {
        *filename = temp;
    }
    return ok;
}

/*static*/
bool wxLocaleHelper::Init(wxLocale* locale, const wxString& exetitle, enum wxLanguage lang)
{
    wxFileName filename;
    wxGetExeFolder(&filename);
    filename.AppendDir(wxT("locale"));
    wxLocale::AddCatalogLookupPathPrefix(filename.GetFullPath());

    if (locale->Init(lang))
    {
        locale->AddCatalog(exetitle);
        return true;
    }

    return false;
}

/*static*/
bool wxLocaleHelper::Find(const wxString& localeName, wxLanguage* found_language)
{
    const size_t len = localeName.Length();

    for (int i = wxLANGUAGE_UNKNOWN + 1; i < wxLANGUAGE_USER_DEFINED; i++)
    {
        const wxLanguageInfo* info = wxLocale::GetLanguageInfo(i);

        if ( info && ((0 == localeName.CmpNoCase(info->CanonicalName)) ||
                      (0 == localeName.CmpNoCase(info->CanonicalName.Left(len)))))
        {
            if (found_language)
                *found_language = (wxLanguage)i;

            return true;
        }
    }

    return false;
}

/*static*/
size_t wxLocaleHelper::GetSupportedLanguages(wxArrayInt& languages,
                                             const wxString& localeDir)
{
    size_t init_count = languages.GetCount();

    wxFileName filename(localeDir);

    if (localeDir.IsEmpty())
    {
        wxGetExeFolder(&filename);
        filename.AppendDir(wxT("locale"));
    }

    const enum wxLanguage default_lang = wxLANGUAGE_ENGLISH;
    languages.Add(default_lang);

    wxDir    dir;
    wxString dirName;

    if (!dir.Open(filename.GetFullPath()))
        return 0;

    for (bool cont = dir.GetFirst(&dirName, wxEmptyString, wxDIR_DIRS);
           cont;
           cont = dir.GetNext(&dirName))
    {
        enum wxLanguage lang = default_lang;

        if (wxLocaleHelper::Find(dirName, &lang) &&
            (lang != default_lang) &&
            (languages.Index(lang) == wxNOT_FOUND))
            languages.Add(lang);
    }

   return languages.GetCount() - init_count;
}

/*static*/
bool wxLocaleHelper::SingleChoice(const wxArrayInt& languages, wxLanguage* selected_language)
{
    size_t n, count = languages.GetCount();

    if (count == 0u)
    {
        wxMessageBox(_("Unable to find language translations, defaulting to English."),
                     _("No Languages Found"), wxOK|wxICON_ERROR);
        return false;
    }

    wxArrayString languageNames;

    for (n = 0; n < count; ++n)
    {
        enum wxLanguage lang = (enum wxLanguage)languages.Item(n);

        wxString name = wxLocale::GetLanguageName(lang);

        if (!name.IsEmpty())
            languageNames.Add(name);
    }

    int index = wxGetSingleChoiceIndex(wxT("Language"), wxMessageBoxCaption, languageNames);

    if (index != wxNOT_FOUND)
    {
        if (selected_language)
            *selected_language = (wxLanguage)languages.Item(index);

        return true;
    }

    return wxLANGUAGE_UNKNOWN;
}

#if wxUSE_ACCEL

/*static*/
wxAcceleratorEntry wxAcceleratorHelper::GetStockAccelerator(wxWindowID id)
{
    wxAcceleratorEntry accelEntry;

    #define STOCKITEM(stockid, flags, keycode)      \
        case stockid: accelEntry.Set(flags, keycode, stockid); break;

    // subjective list of accelerators considered "stock" (standard)
    switch (id)
    {
    #if (wxVERSION_NUMBER < 2903)
        STOCKITEM(wxID_PRINT,         wxACCEL_CTRL,                 'P')
        STOCKITEM(wxID_UNDO,          wxACCEL_CTRL,                 'Z')
    #endif
        STOCKITEM(wxID_PREVIEW,       wxACCEL_CTRL | wxACCEL_SHIFT, 'P')
        STOCKITEM(wxID_SAVEAS,        wxACCEL_CTRL | wxACCEL_SHIFT, 'S')
        STOCKITEM(wxID_SELECTALL,     wxACCEL_CTRL,                 'A')
        STOCKITEM(wxID_REDO,          wxACCEL_CTRL,                 'Y')
        STOCKITEM(wxID_EXIT,          wxACCEL_CTRL,                 'Q')
        STOCKITEM(wxID_ABOUT,         wxACCEL_SHIFT,           WXK_HELP)
        default:
            accelEntry = wxGetStockAccelerator(id);
            break;
    }

    #undef STOCKITEM

#if (wxVERSION_NUMBER >= 2902)
    wxASSERT(accelEntry.IsOk());
#else
    // trac.wxwidgets.org/ticket/12444
    // trac.wxwidgets.org/ticket/12445
#endif
    return accelEntry;
}

/*static*/
void wxAcceleratorHelper::SetAcceleratorTable(wxWindow* wnd, const wxArrayAcceleratorEntry& array)
{
   const size_t count = array.GetCount();
   wxAcceleratorEntry* temp = new wxAcceleratorEntry[count];

   for (size_t i = 0; i < count; i++)
   {
      temp[i] = array.Item(i);
   }
   wxAcceleratorTable accel((int)count, temp);

   wnd->SetAcceleratorTable(accel);
   delete [] temp;
}

#endif // wxUSE_ACCEL

static wxString wxMenuItem_GetText(const wxMenuItem* item)
{
   wxString str = item->GetItemLabel();

#ifdef __WXGTK__
   str.Replace(wxString(wxT('_')), wxString(wxT('&')));
#endif
   return str;
}

#define ACCELSTR_SEP "   "

static bool wxMenuItem_SetAccelText(wxMenuItem* item, const wxString& accel, bool append = true)
{
   wxString str = wxMenuItem_GetText(item);
   wxString ch_sep = wxT("\t");
   const int sep = str.Find(ch_sep);

   if (wxNOT_FOUND == sep)
   {
   }
   else if (append)
   {
#ifdef __WXMSW__
    ch_sep = wxT(ACCELSTR_SEP);
#else
    // Having multiple accelerators per menu item in GTK yields these warnings in the console window,
    // "Unknown accel modifier: 'w   ctrl'...No accel key found, accel string ignored."
    // see also trac.wxwidgets.org/ticket/9363#comment:5
    return false;
#endif
   }
   else
   {
      str.Truncate(sep);
   }
   item->SetItemLabel(wxString::Format(wxT("%s%s%s"),
      str.wx_str(),
      ch_sep.wx_str(),
      accel.wx_str()));
   return true;
}

static wxString wxGetAccelText(int flags, int keyCode)
{
   // wxAcceleratorEntry.ToString() produces silly text
   wxString str;
#if (wxVERSION_NUMBER >= 2901)
   const wxChar sep = '+'; // the new wx default
#else
   const wxChar sep = '-'; // the old annoying wx default
#endif
#if 0 && (wxVERSION_NUMBER >= 2800)
   wxAcceleratorEntry entry(flags, keyCode);
   str = entry.ToString(); // doesn't work (wxIsalnum(WXK_F2)), silly text (WXK_NUMPAD_ADD)
#else

   if (flags & wxACCEL_CTRL)
   {
      if (!str.IsEmpty()) str+=sep;
      str+=_("Ctrl");
   }
   if (flags & wxACCEL_ALT)
   {
      if (!str.IsEmpty()) str+=sep;
      str+=_("Alt");
   }
   if (flags & wxACCEL_SHIFT)
   {
      if (!str.IsEmpty()) str+=sep;
      str+=_("Shift");
   }
   if (!str.IsEmpty()) str+=sep;
   switch (keyCode)
   {
      case WXK_INSERT          : str += _("Insert" ); break;
      case WXK_PAGEUP          : str += _("PgUp"   ); break;
      case WXK_PAGEDOWN        : str += _("PgDn"   ); break;
      case WXK_HOME            : str += _("Home"   ); break;
      case WXK_END             : str += _("End"    ); break;
      case WXK_RETURN          : str += _("Return" ); break;
      case WXK_DELETE          : str += _("Del"    ); break;
      case WXK_SPACE           : str += _("Space"  ); break;
      case WXK_NUMPAD_ADD      : str += _("Num+"   ); break;
      case WXK_NUMPAD_SUBTRACT : str += _("Num-"   ); break;
      default:
      {
         if ( (keyCode >= WXK_F1) && (keyCode <= WXK_F24) )
         {
            str += wxString::Format(wxT("F%d"), keyCode - WXK_F1 + 1);
         }
         else
         {
            wxASSERT(keyCode >= ' ');
            str += (wxChar)keyCode;
         }
         break;
      }
   }
#endif

   return str;
}

wxString wxGetStockLabelEx(wxWindowID id, long flags)
{
#define STOCKITEM(stockid, label) \
     case stockid:                 \
         stockLabel = label;       \
         break;

   wxString stockLabel;

   switch (id)
   {
      STOCKITEM(wxID_SAVEAS,     _("Save &As...")) // + ellipsis
      STOCKITEM(wxID_SELECTALL,  _("Select &All")) // + ampersand
      STOCKITEM(wxID_FIND,       _("&Find..."))    // + ellipsis
      STOCKITEM(wxID_REPLACE,    _("Rep&lace...")) // + ellipsis
      STOCKITEM(wxID_REVERT,     _("Re&vert..."))
      STOCKITEM(wxID_PREVIEW,    _("Print Previe&w")) // + ampersand
      STOCKITEM(wxID_PROPERTIES, _("Proper&ties...")) // + ellipsis
#if (wxVERSION_NUMBER >= 2900)
#else
      STOCKITEM(wxID_OPEN,       _("&Open..."))  // + ellipsis
      STOCKITEM(wxID_PRINT,      _("&Print...")) // + ellipsis
#endif
      default:
         break;
   }
#undef STOCKITEM

   if (!stockLabel.IsEmpty())
   {
       if ( !(flags & wxSTOCK_WITH_MNEMONIC) )
       {
           stockLabel = wxStripMenuCodes(stockLabel);
       }
#if (wxVERSION_NUMBER >= 2901)
       if (flags & wxSTOCK_WITHOUT_ELLIPSIS)
       {
           wxString baseLabel;
           if ( stockLabel.EndsWith(wxT("..."), &baseLabel) )
               stockLabel = baseLabel;
       }
#else
       // handled below
#endif
   }
   else
   {
      stockLabel = wxGetStockLabel(id, flags);
   }
#if (wxVERSION_NUMBER < 2901)
   if (flags & wxSTOCK_WITHOUT_ELLIPSIS)
   {
      wxString baseLabel;
      if ( stockLabel.EndsWith(wxT("..."), &baseLabel) )
         stockLabel = baseLabel;
   }
#endif

   return stockLabel;
}

static wxString wxGetAccelText(const wxAcceleratorEntry& accel)
{
   return wxGetAccelText(accel.GetFlags(), (enum wxKeyCode)accel.GetKeyCode());
}

/*static*/
void wxAcceleratorHelper::SetAccelText(wxMenuBar* menubar, const wxArrayAcceleratorEntry& accel)
{
   size_t count = menubar->GetMenuCount();

   for (size_t j = 0; j < count; j++)
   {
      wxMenu* menu = menubar->GetMenu(j);

      wxAcceleratorHelper::SetAccelText(menu, accel);
   }
}

wxString wxToolBarTool_MakeShortHelp(const wxString& rstr, const wxArrayAcceleratorEntry& accel, int id)
{
   wxString str = rstr;

   if (accel.GetCount() && !str.IsEmpty())
   {
      wxString strAccel;
      size_t i, count = accel.GetCount();

      for (i = 0; i < count; i++)
      {
         const wxAcceleratorEntry& element = accel.Item(i);

         if (element.GetCommand() == id)
         {
            if (!strAccel.IsEmpty()) strAccel+=wxT(ACCELSTR_SEP);
            strAccel+=wxGetAccelText(element);
         }
      }
      if (!strAccel.IsEmpty())
      {
         str += wxString::Format(wxT(" (%s)"), strAccel.wx_str());
      }
   }
   return str;
}

static bool wxMenuItem_SetAccelText(wxMenuItem* item, const wxAcceleratorEntry& entry)
{
   return wxMenuItem_SetAccelText(item, wxGetAccelText(entry));
}

static void wxMenu_SetAccelText(wxMenu* menu, const wxAcceleratorEntry& accel)
{
   wxMenuItemList& list = menu->GetMenuItems();

   for (wxMenuItemList::iterator it = list.begin();
        it != list.end();
        it++)
   {
      wxMenuItem* item = *it;

      if (item->IsSubMenu())
      {
         wxMenu_SetAccelText(item->GetSubMenu(), accel);
      }
      else if (item->GetId() == accel.GetCommand())
      {
         wxMenuItem_SetAccelText(item, accel);
      }
   }
}

/*static*/
void wxAcceleratorHelper::SetAccelText(wxMenu* menu, const wxArrayAcceleratorEntry& array)
{
    size_t i, count = array.GetCount();

    for (i = 0; i < count; i++)
    {
        const wxAcceleratorEntry& accel = array.Item(i);

        wxMenu_SetAccelText(menu, accel);
    }
}

/////////////////////////////////////////////////////////////////////////////
// wxPreviewFrameEx

wxPreviewFrameEx::wxPreviewFrameEx(wxPrintPreviewBase* preview,
                wxWindow *parent,
                const wxString& title,
                const wxPoint& pos,
                const wxSize& size,
                long style,
                const wxString& name)
   : wxPreviewFrame(preview, parent, title, pos, size, style, name)
{
}

BEGIN_EVENT_TABLE(wxPreviewFrameEx, wxPreviewFrame)
#if (wxVERSION_NUMBER < 2900)
    EVT_CHAR_HOOK(wxPreviewFrameEx::OnKeyDown)
#endif
END_EVENT_TABLE()

bool wxPreviewFrameEx::Destroy()
{
   bool ok = base::Destroy();

   if (ok && GetParent())
   {
      GetParent()->Raise();
   }
   return ok;
}

#if (wxVERSION_NUMBER < 2900)
// trac.wxwidgets.org/ticket/8570
void wxPreviewFrameEx::OnKeyDown(wxKeyEvent& event)
{
   switch (event.GetKeyCode())
   {
      case WXK_ESCAPE:
         Close();
         break;
      default:
         event.Skip();
         break;
   }
}
#endif

void wxCommandLineUsage(wxWindow* parent)
{
    wxCmdLineParser parser;

    wxTheApp->OnInitCmdLine(parser);
#if (wxVERSION_NUMBER >= 2900)
    // GetUsageString() is public
    wxMessageBox(parser.GetUsageString(), wxTheApp->GetAppDisplayName(), wxOK | wxICON_INFORMATION, parent);
#elif defined(__WXMSW__)
    // GetUsageString() is private, sigh
    parser.Usage();
    wxUnusedVar(parent);
#else
   // Usage() goes to console, sigh
    wxUnusedVar(parent);
#endif
}

void wxFrame_SetInitialPosition(wxFrame* wnd, const wxPoint& pos, const wxSize& size, int margin_pct)
{
   if (size == wxDefaultSize)
   {
      wxRect rect = wxGetClientDisplayRect();
      wxSize size(
         (rect.width  * (100 - margin_pct*2))/100,
         (rect.height * (100 - margin_pct*2))/100);

      wnd->SetSize(size);
   }
   if (pos == wxDefaultPosition)
   {
      wnd->Center();
   }
}

void wxFrame_ClonePosition(wxFrame* wnd, wxWindow* otherwindow /*= NULL*/)
{
   otherwindow = otherwindow ? wxGetTopLevelParent(otherwindow) : wxTheApp->GetTopWindow();
   wxFrame* topframe = wxStaticCast(otherwindow, wxFrame);

   if (topframe->IsMaximized())
   {
      wnd->Maximize();
   }
   else if (topframe->IsFullScreen())
   {
      wnd->Maximize();
   }
   else
   {
      wxRect rect = topframe->GetScreenRect();

      wnd->SetSize(rect);
   }
}

#define HASBIT(value, bit)      (((value) & (bit)) != 0)

/*static*/
bool wxClipboardHelper::IsTextAvailable(Clipboard_Type clip_type)
{
    wxCHECK_MSG(clip_type != CLIPBOARD_BOTH, false, wxT("Getting values from both clipboards is not supported"));

    bool ok = false;
#if wxUSE_CLIPBOARD
    const enum wxDataFormatId text[] =
    {
        wxDF_TEXT
      //,wxDF_OEMTEXT,     // This is wxDF_TEXT in MSW, not supported in GTK/OSX
#   if wxUSE_UNICODE
        ,wxDF_UNICODETEXT  // asserts in ANSI build
#   endif // wxUSE_UNICODE
#   ifdef __WXMSW__
        ,wxDF_HTML         // Only supported in MSW
#   endif // __WXMSW__
    };

    ok = IsFormatAvailable(text, WXSIZEOF(text), clip_type);
#endif // wxUSE_CLIPBOARD
    return ok;
}

/*static*/
bool wxClipboardHelper::IsFormatAvailable(const enum wxDataFormatId* array,
                                                     size_t array_count,
                                                     Clipboard_Type clip_type)
{
    wxCHECK_MSG(clip_type != CLIPBOARD_BOTH, false, wxT("Getting values from both clipboards is not supported"));

    bool ok = false;
#if wxUSE_CLIPBOARD
    wxClipboard* clipboard = wxTheClipboard;
    bool was_open = clipboard->IsOpened();
    ok = was_open || clipboard->Open();

    if (ok)
    {
        size_t i;

        clipboard->UsePrimarySelection(HASBIT(clip_type, CLIPBOARD_PRIMARY));
        for (i = 0; i < array_count; i++)
        {
        #ifdef __WXMSW__
            // wxClipboard::IsSupported(wxDF_HTML) returns false always; handle it here instead
            if (array[i] == wxDF_HTML)
            {
                static int CF_HTML = ::RegisterClipboardFormat(_T("HTML Format"));

                if (::IsClipboardFormatAvailable(CF_HTML))
                {
                    break;
                }
            }
            else
        #endif
            if (clipboard->IsSupported(wxDataFormat(array[i])))
            {
                break;
            }
        }
        ok = (i != array_count);

        if (!was_open)
            clipboard->Close();
    }
#endif // wxUSE_CLIPBOARD
    return ok;
}

/*static*/
bool wxClipboardHelper::GetText(wxString* str, Clipboard_Type clip_type)
{
    wxCHECK_MSG(clip_type != CLIPBOARD_BOTH, false, wxT("Getting values from both clipboards is not supported"));

    if (!str) return false;
    bool ok = false;

#if wxUSE_DATAOBJ && wxUSE_CLIPBOARD
    wxClipboard* clipboard = wxTheClipboard;
    bool was_open = clipboard->IsOpened();
    ok = was_open || clipboard->Open();

    if (ok)
    {
        wxTextDataObject temp;

        clipboard->UsePrimarySelection(HASBIT(clip_type, CLIPBOARD_PRIMARY));
        ok = clipboard->GetData(temp);

        if (ok)
            *str = temp.GetText();

        if (!was_open)
            clipboard->Close();
    }
#endif // wxUSE_DATAOBJ && wxUSE_CLIPBOARD
    return ok && !str->empty();
}

/*static*/
bool wxClipboardHelper::Set(wxDataObject* def, wxDataObject* primary)
{
#if wxUSE_DATAOBJ && wxUSE_CLIPBOARD
    wxClipboard* clipboard = wxTheClipboard;
    bool was_open = clipboard->IsOpened();
    bool ok = was_open || clipboard->Open();

    if (ok)
    {
        if (def)
        {
            clipboard->UsePrimarySelection(false);
            ok = clipboard->SetData(def);
            if (ok)
            {
                def = NULL;
            }
        }
    #ifndef __WXMSW__
        if (primary)
        {
            clipboard->UsePrimarySelection(true);
            ok = clipboard->SetData(primary);
            clipboard->UsePrimarySelection(false);
            if (ok)
            {
                primary = NULL;
            }
        }
    #endif // __WXMSW__
        if (!was_open)
        {
            clipboard->Close();
        }
        //clipboard->Flush(); // else emu and wxc is freezing
    }
    delete def;
    delete primary;
    return ok;
#else
    return false;
#endif
}

/*static*/
bool wxClipboardHelper::SetText(const wxString& str, Clipboard_Type clip_type)
{
#if wxUSE_DATAOBJ && wxUSE_CLIPBOARD
    return Set(HASBIT(clip_type, CLIPBOARD_DEFAULT) ? new wxTextDataObject(str) : NULL,
               HASBIT(clip_type, CLIPBOARD_PRIMARY) ? new wxTextDataObject(str) : NULL);
#else
    return false;
#endif
}

/*static*/
bool wxClipboardHelper::SetHtmlText(const wxString& htmldata)
{
    bool ok;
#ifdef __WXMSW__
    ok = wxOpenClipboard();
    if (ok)
    {
        EmptyClipboard();
        const wxCharBuffer buf(htmldata.mb_str());
        ok = wxSetClipboardData(wxDF_HTML, buf.data()); // save as html
        wxSetClipboardData(wxDF_TEXT, buf.data());      // save also as plain text
        wxCloseClipboard();
    }
#else
    ok = SetText(htmldata);
#endif
    return ok;
}

#if (wxVERSION_NUMBER < 2903)
static const char BOM_UTF32BE[] = { '\x00', '\x00', '\xFE', '\xFF' };
static const char BOM_UTF32LE[] = { '\xFF', '\xFE', '\x00', '\x00' };
static const char BOM_UTF16BE[] = { '\xFE', '\xFF'                 };
static const char BOM_UTF16LE[] = { '\xFF', '\xFE'                 };
static const char BOM_UTF8[]    = { '\xEF', '\xBB', '\xBF'         };

const char* wxConvAuto_GetBOMChars(wxBOM bom, size_t* count)
{
    wxCHECK_MSG( count , NULL, wxS("count pointer must be provided") );

    switch ( bom )
    {
        case wxBOM_UTF32BE: *count = WXSIZEOF(BOM_UTF32BE); return BOM_UTF32BE;
        case wxBOM_UTF32LE: *count = WXSIZEOF(BOM_UTF32LE); return BOM_UTF32LE;
        case wxBOM_UTF16BE: *count = WXSIZEOF(BOM_UTF16BE); return BOM_UTF16BE;
        case wxBOM_UTF16LE: *count = WXSIZEOF(BOM_UTF16LE); return BOM_UTF16LE;
        case wxBOM_UTF8   : *count = WXSIZEOF(BOM_UTF8   ); return BOM_UTF8;
        case wxBOM_Unknown:
        case wxBOM_None:
            wxFAIL_MSG( wxS("Invalid BOM type") );
            return NULL;
    }

    wxFAIL_MSG( wxS("Unknown BOM type") );
    return NULL;
}

#define BOM_EQUAL(src,array) ( 0 == memcmp((src), array, sizeof(array) ) )

wxBOM wxConvAuto_DetectBOM(const char *src, size_t srcLen)
{
    // examine the buffer for BOM presence
    //
    // quoting from http://www.unicode.org/faq/utf_bom.html#BOM:
    //
    //  Bytes           Encoding Form
    //
    //  00 00 FE FF     UTF-32, big-endian
    //  FF FE 00 00     UTF-32, little-endian
    //  FE FF           UTF-16, big-endian
    //  FF FE           UTF-16, little-endian
    //  EF BB BF        UTF-8
    //
    // as some BOMs are prefixes of other ones we may need to read more bytes
    // to disambiguate them

    switch ( srcLen )
    {
        case 0:
            return wxBOM_Unknown;

        case 1:
            if ( src[0] == '\x00' || src[0] == '\xFF' ||
                 src[0] == '\xFE' || src[0] == '\xEF')
            {
                // this could be a BOM but we don't know yet
                return wxBOM_Unknown;
            }
            break;

        case 2:
        case 3:
            if ( src[0] == '\xEF' && src[1] == '\xBB' )
            {
                if ( srcLen == 3 )
                    return src[2] == '\xBF' ? wxBOM_UTF8 : wxBOM_None;

                return wxBOM_Unknown;
            }

            if ( BOM_EQUAL(src, BOM_UTF16BE) )
                return wxBOM_UTF16BE;

            if ( BOM_EQUAL(src, BOM_UTF16LE) )
            {
                // if the next byte is 0, it could be an UTF-32LE BOM but if it
                // isn't we can be sure it's UTF-16LE
                if ( srcLen == 3 && src[2] != '\x00' )
                    return wxBOM_UTF16LE;

                return wxBOM_Unknown;
            }

            if ( src[0] == '\x00' && src[1] == '\x00' )
            {
                // this could only be UTF-32BE, check that the data we have so
                // far allows for it
                if ( srcLen == 3 && src[2] != '\xFE' )
                    return wxBOM_None;

                return wxBOM_Unknown;
            }
            break;

        default:
            // we have at least 4 characters so we may finally decide whether
            // we have a BOM or not
            if ( BOM_EQUAL(src, BOM_UTF8) )
                return wxBOM_UTF8;

            if ( BOM_EQUAL(src, BOM_UTF32BE) )
                return wxBOM_UTF32BE;

            if ( BOM_EQUAL(src, BOM_UTF32LE) )
                return wxBOM_UTF32LE;

            if ( BOM_EQUAL(src, BOM_UTF16BE) )
                return wxBOM_UTF16BE;

            if ( BOM_EQUAL(src, BOM_UTF16LE) )
                return wxBOM_UTF16LE;
    }

    return wxBOM_None;
}
#endif

// annoying method only here because the wxString ctor conv argument is WXUNUSED in some build types
/*static*/
bool wxTextEncoding::CharToString(wxString* dst_wxstr, const char* src, 
                                  const wxMBConv& conv, size_t len)
{
    wxString str;

    if (len)
    {
        size_t wlen;
        wxWCharBuffer buf(conv.cMB2WC(src, len, &wlen));

        str = wxString(buf.data(), wxConvLibc /*WXUNUSED*/, wlen);

        if (str.IsEmpty())
            return false;
    }

    if (dst_wxstr) *dst_wxstr = str;
    return true;
}

// annoying method only here because the wxString ctor conv argument is WXUNUSED in some build types
/*static*/
wxCharBuffer wxTextEncoding::StringToChar(const wxString& src_wxstr, const wxMBConv& conv)
{
    wxWCharBuffer wbuf(src_wxstr.wc_str(wxConvLibc /*WXUNUSED*/));
    wxCharBuffer buf(conv.cWC2MB(wbuf));

    return buf;
}

/*static*/
wxCharBuffer wxTextEncoding::StringToChar(const wxString& src_wxstr, TextEncoding_Type encoding_type, 
                                          size_t* buffer_size)
{
    wxCharBuffer buf;
    size_t size;

    switch (encoding_type)
    {
        case Ascii:
        {
            buf = src_wxstr.mb_str(*wxConvCurrent);
            size = wxBuffer_length(buf);
            break;
        }
        case Unicode_LE:
        {
            const wxWCharBuffer temp = src_wxstr.wc_str(*wxConvCurrent);
            size = wxBuffer_length(temp) * sizeof(wchar_t);

            buf.extend(size);
            memcpy(buf.data(), temp.data(), size);
            break;
        }
        case UTF8:
        {
            buf = StringToChar(src_wxstr, wxConvUTF8);
            size = wxBuffer_length(buf);
            break;
        }
        case ISO8859_1:
        {
            buf = StringToChar(src_wxstr, wxConvISO8859_1);
            size = wxBuffer_length(buf);
            break;
        }
    #ifdef __WXMSW__
        case OEM:
        {
            buf = StringToChar(src_wxstr, wxMBConvOEM());
            size = wxBuffer_length(buf);
            break;
        }
    #endif // __WXMSW__
        default:
        {
            size = 0;
            break;
        }
    }
    
    if (buffer_size) *buffer_size = size;

    return buf;
}

/*static */
bool wxTextEncoding::CharToStringDetectBOM(wxString* str_ptr, const wxCharBuffer& buf, size_t buf_len, wxBOM* file_bom_ptr)
{
    wxConvAuto conv_auto;
    wxBOM file_bom;
    wxString str;
    bool ok = true;

    if (buf_len == wxNO_LEN) buf_len = wxBuffer_length(buf);

#if (wxVERSION_NUMBER >= 2903) && wxUSE_UNICODE
    str = wxString(buf.data(), conv_auto, buf_len); // ctor conv arg WXUNUSED() in ansi builds in wx28 (in wx293 ansi builds ??)
    file_bom = conv_auto.GetBOM();
#else
    // The method wxAutoConv.GetBOM() is not in wx 2.8, so roll our own
    file_bom = wxConvAuto_DetectBOM(buf.data(), buf_len);
    size_t bom_charcount = 0;

    switch (file_bom)
    {
        case wxBOM_Unknown:
        case wxBOM_None:
            // wxConvAuto.GetBOMChars() barks if passed these
            break;
        default:
            wxConvAuto_GetBOMChars(file_bom, &bom_charcount);
            break;
    }

    switch (file_bom)
    {
        case wxBOM_UTF32BE:
        case wxBOM_UTF32LE:
        case wxBOM_UTF16BE:
            // not supported
            break;
        case wxBOM_UTF16LE:
            str = wxString((wchar_t*)(buf.data() + bom_charcount), *wxConvCurrent, (buf_len - bom_charcount) / sizeof(wchar_t));
            break;
        case wxBOM_UTF8:
            ok = CharToString(&str, buf.data() + bom_charcount, wxConvUTF8, buf_len - bom_charcount);
            break;
        default:
            str = wxString(buf.data(), wxConvLibc /*WXUNUSED*/, buf_len);
            break;
    }
#endif // 2.9
    if (ok)
    {
        if (str_ptr     ) *str_ptr      = str;
        if (file_bom_ptr) *file_bom_ptr = file_bom;
    }
    return ok;
}

/*static*/
const char* wxTextEncoding::GetBOMChars(TextEncoding_Type encoding, size_t* count)
{
    switch (encoding)
    {
        case UTF8:       return wxConvAuto_GetBOMChars(wxBOM_UTF8   , count);
        case Unicode_LE: return wxConvAuto_GetBOMChars(wxBOM_UTF16LE, count);
    #ifdef __WXMSW__
        case OEM:
    #endif
        case ISO8859_1:
        default:
            return NULL;
    }
}

#ifdef __WXMSW__

wxMBConvOEM::wxMBConvOEM() : wxMBConv()
{
}

wxMBConvOEM::~wxMBConvOEM()
{
}

size_t wxMBConvOEM::ToWChar(wchar_t*    dst, size_t dstLen,
                            const char* src, size_t srcLen) const
{
    if (srcLen == wxNO_LEN) srcLen = strlen(src);
    wxCharBuffer buf(srcLen);

    OemToCharBuffA(src, buf.data(), (DWORD)srcLen);
    return dst ? mbstowcs(dst, buf.data(), dstLen) : wxBuffer_length(buf);
}

size_t wxMBConvOEM::FromWChar(char*          dst, size_t dstLen,
                              const wchar_t* src, size_t srcLen) const
{
    size_t len;
    if (srcLen == wxNO_LEN) srcLen = wcslen(src);
    wxCharBuffer temp(srcLen);

    wcstombs(temp.data(), src, srcLen);
    if (dst)
    {
        CharToOemBuffA(temp.data(), dst, (DWORD)dstLen);
        len = strlen(dst);
    }
    else
    {
        len = wxBuffer_length(temp);
    }
    return len;
}
#endif

static const wxChar* const s_textencoding_text[] =
{
    wxT("Ascii"),
    wxT("UTF-8"),
    wxT("Unicode"),
    wxT("ISO-8859-1"),
#ifdef __WXMSW__
    wxT("OEM"),
#endif
};
#ifdef C_ASSERT
C_ASSERT(WXSIZEOF(s_textencoding_text) == wxTextEncoding::TextEncoding__Count);
#endif

/*static*/
wxTextEncoding::TextEncoding_Type wxTextEncoding::TypeFromString(const wxString& str)
{
    for (int i = 0; i < TextEncoding__Count; i++)
    {
        if (0 == str.CmpNoCase(s_textencoding_text[i]))
        {
            return (wxTextEncoding::TextEncoding_Type)i;
        }
    }
    return wxTextEncoding::Ascii;
}

/*static*/
wxString wxTextEncoding::TypeToString(TextEncoding_Type encoding_type)
{
    return (encoding_type < wxTextEncoding::TextEncoding__Count)
                ? s_textencoding_text[encoding_type]
                : wxEmptyString;
}

/*static*/
bool wxTextEncoding::TypeFromString(TextEncoding_Type* encoding_type, 
                                    const char* str,
                                    const char* identifier, const char* strpbrk_ctrl)
{
    const char* p = strstr(str, identifier);

    if (p)
    {
        const char* begin = p + strlen(identifier);
        const char* end   = strpbrk(begin, strpbrk_ctrl);

        if (begin && end)
        {
            if (encoding_type)
            {
                *encoding_type = TypeFromString(wxString::From8BitData(begin, end - begin));
            }
            return true;
        }
    }
    return false;
}

/*static*/
bool wxTextEncoding::CharToString(wxString* str_ptr, const wxCharBuffer& buf, size_t buf_len,
                                  TextEncoding_Type encoding, wxBOM bom)
{
    size_t bom_count = 0;
    wxString str;
    bool ok = true;

    if (wxNO_LEN == buf_len) buf_len = wxBuffer_length(buf);

    switch (bom)
    {
        case wxBOM_Unknown:
        case wxBOM_None:
            //wxConvAuto_GetBOMChars(wxBOM_Unknown) asserts :-(
            break;
        default:
            wxConvAuto_GetBOMChars(bom, &bom_count);
            buf_len -= bom_count;
            break;
    }

    switch (encoding)
    {
        case Unicode_LE:
            ok = CharToString(&str, buf.data() + bom_count, wxConvAuto(), buf_len);
            break;
        case UTF8:
            ok = CharToString(&str, buf.data() + bom_count, wxConvUTF8, buf_len);
            break;
        case ISO8859_1:
            ok = CharToString(&str, buf.data() + bom_count, wxConvISO8859_1, buf_len);
            break;
    #ifdef __WXMSW__
        case OEM:
            ok = CharToString(&str, buf.data() + bom_count, wxMBConvOEM(), buf_len);
            break;
    #endif
        case Ascii:
        default:
            str = wxConvertMB2WX(buf.data() + bom_count);
            break;
    }
    if (ok && str_ptr) *str_ptr = str;
    return ok;
}

/*static*/
bool wxTextEncoding::SaveFile(const wxString& s, wxOutputStream& stream, TextEncoding_Type encoding, bool file_bom)
{
    bool ok = true;
    const char* bom_chars;
    size_t size;

    // write bom
    if (ok && file_bom)
    {
        switch (encoding)
        {
            case Unicode_LE:
                bom_chars = wxConvAuto_GetBOMChars(wxBOM_UTF16LE, &size);
                ok = bom_chars && (size == stream.Write(bom_chars, size * sizeof(char)).LastWrite());
                break;
            case UTF8:
                bom_chars = wxConvAuto_GetBOMChars(wxBOM_UTF8, &size);
                ok = bom_chars && (size == stream.Write(bom_chars, size * sizeof(char)).LastWrite());
                break;
            case Ascii:
        #ifdef __WXMSW__
            case OEM:
        #endif
                break;
            default:
                ok = false;
                break;
        }
    }

    // write text
    if (ok)
    {
        const wxCharBuffer buf = StringToChar(s, encoding, &size);

        ok = (buf.data() != NULL);
        if (ok)
        {
            ok = (size == stream.Write(buf.data(), size).LastWrite());
        }
    }
    return ok;
}
