///////////////////////////////////////////////////////////////////////////////
// Name:        steart.cpp
// Purpose:     wxSTEditorArtProvider
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     29/05/2010
// RCS-ID:
// Copyright:   (c) John Labenski, Troels K, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "wx/stedit/stedefs.h"
#include "wx/stedit/steart.h"
#include "wxext.h"

#include <wx/image.h>
#include <wx/settings.h> // for wxSystemSettings

// Bitmaps used for the toolbar in the wxSTEditorFrame
#include "../art/pencil16.xpm"
#include "../art/pencil32.xpm"
#include "../art/new.xpm"
#include "../art/open.xpm"
#include "../art/save.xpm"
#include "../art/saveall.xpm"
#include "../art/saveas.xpm"
#include "../art/print.xpm"
#include "../art/print_preview.xpm"
#include "../art/print_setup.xpm"
#include "../art/print_page_setup.xpm"
#include "../art/x_red.xpm"

#include "../art/cut.xpm"
#include "../art/copy.xpm"
#include "../art/paste.xpm"
#include "../art/find.xpm"
#include "../art/findnext.xpm"
#include "../art/finddown.xpm"
#include "../art/findup.xpm"
#include "../art/replace.xpm"
#include "../art/undo.xpm"
#include "../art/redo.xpm"
#include "../art/cross.xpm"


const wxSize wxSTEIconSize     (wxSystemSettings::GetMetric(wxSYS_ICON_X     ), wxSystemSettings::GetMetric(wxSYS_ICON_Y     ));
const wxSize wxSTESmallIconSize(wxSystemSettings::GetMetric(wxSYS_SMALLICON_X), wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y));

//-----------------------------------------------------------------------------
// wxSTEditorArtProvider
//-----------------------------------------------------------------------------

/*static*/ wxBitmap wxSTEditorArtProvider::m_app_small; // can't create in GTK until later
/*static*/ wxBitmap wxSTEditorArtProvider::m_app_large;

wxSTEditorArtProvider::wxSTEditorArtProvider() : wxArtProvider()
{
    if (!m_app_small.IsOk())
    {
        // these should always be created since a wxSTEditorArtProvider is created in wxSTEditorModule
        m_app_small = wxBitmap(pencil16_xpm); // must create after wxApp initialization
        m_app_large = wxBitmap(pencil32_xpm);
    }
}

// static
wxBitmap wxSTEditorArtProvider::DoGetBitmap(const wxArtID& art_id,
                                            const wxArtClient& client,
                                            const wxSize& size_)
{
    static const struct art_item
    {
        // wxArtID id; - can't have wxString in struct for MSVC6
#if (wxVERSION_NUMBER >= 2902)
        const char* art_id;
#else
        const wxChar* art_id;
#endif
        const char* const* xpm;
    } s_xpm_array[] =
    {
        { wxART_STEDIT_NEW,            new_xpm },
        { wxART_STEDIT_OPEN,           open_xpm },
        { wxART_STEDIT_SAVE,           save_xpm },
        { wxART_STEDIT_SAVEALL,        saveall_xpm },
        { wxART_STEDIT_SAVEAS,         saveas_xpm },
        { wxART_STEDIT_PRINT,          print_xpm },
        { wxART_STEDIT_PRINTPREVIEW,   print_preview_xpm },
        { wxART_STEDIT_PRINTSETUP,     print_setup_xpm },
        { wxART_STEDIT_PRINTPAGESETUP, print_page_setup_xpm },
        { wxART_STEDIT_QUIT,           x_red_xpm },
        { wxART_STEDIT_CUT,            cut_xpm },
        { wxART_STEDIT_COPY,           copy_xpm },
        { wxART_STEDIT_PASTE,          paste_xpm },
        { wxART_STEDIT_FIND,           find_xpm },
        { wxART_STEDIT_FINDNEXT,       findnext_xpm },
        { wxART_STEDIT_FINDUP,         findup_xpm },
        { wxART_STEDIT_FINDDOWN,       finddown_xpm },
        { wxART_STEDIT_REPLACE,        replace_xpm },
        { wxART_STEDIT_UNDO,           undo_xpm },
        { wxART_STEDIT_REDO,           redo_xpm },
        { wxART_STEDIT_CLEAR,          cross_xpm }
    };

    static const size_t s_xpm_array_size = WXSIZEOF(s_xpm_array);

    wxBitmap bmp;
    // If wxDefaultSize is requested, use size hint from client
    wxSize size(size_);
    if (size == wxDefaultSize)
        size = GetSizeHint(client);

    if (art_id == wxART_STEDIT_PREFDLG_VIEW)
        bmp = wxArtProvider::GetBitmap(wxART_FIND, client, size);
    else if (art_id == wxART_STEDIT_PREFDLG_TABSEOL)
        bmp = wxArtProvider::GetBitmap(wxART_LIST_VIEW, client, size);
    else if (art_id == wxART_STEDIT_PREFDLG_FOLDWRAP)
        bmp = wxArtProvider::GetBitmap(wxART_COPY, client, size);
    else if (art_id == wxART_STEDIT_PREFDLG_PRINT)
        bmp = wxArtProvider::GetBitmap(wxART_PRINT, client, size);
    else if (art_id == wxART_STEDIT_PREFDLG_LOADSAVE)
        bmp = wxArtProvider::GetBitmap(wxART_FILE_SAVE, client, size);
    else if (art_id == wxART_STEDIT_PREFDLG_HIGHLIGHT)
        bmp = wxArtProvider::GetBitmap(wxART_TIP, client, size);
    else if (art_id == wxART_STEDIT_PREFDLG_STYLES)
        bmp = wxArtProvider::GetBitmap(wxART_HELP_BOOK, client, size);
    else if (art_id == wxART_STEDIT_PREFDLG_LANGS)
        bmp = wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, client, size);
    else if (art_id == wxART_STEDIT_APP)
    {
        // try to get the bitmap that is closest in size to the requested size
        // we will resize it later if necessary
        if ((size.GetWidth()  > m_app_small.GetWidth()  + 5) ||
            (size.GetHeight() > m_app_small.GetHeight() + 5))
            bmp = m_app_large;
        else
           bmp = m_app_small;
    }
    else
    {
        // we don't need to be fast about this since the wxArtProvider caches them
        for (size_t i = 0; i < s_xpm_array_size; ++i)
        {
            if (s_xpm_array[i].art_id == art_id)
            {
                bmp = wxBitmap(s_xpm_array[i].xpm);
                break;
            }
        }
    }

    return Resize(bmp, size); // does nothing if already correct size
}

// static
wxBitmap wxSTEditorArtProvider::Resize(const wxBitmap& bmp_, const wxSize& size)
{
    wxBitmap bmp(bmp_);

    if (!bmp.IsOk() || (size.GetWidth() < 1) || (size.GetHeight() < 1))
        return bmp;

    int bmp_w = bmp.GetWidth();
    int bmp_h = bmp.GetHeight();

    if ((bmp_w != size.GetWidth()) || (bmp_h != size.GetHeight()))
    {
        wxPoint offset((size.GetWidth() - bmp_w)/2, (size.GetHeight() - bmp_h)/2);
        wxImage img = bmp.ConvertToImage();
        img.Resize(size, offset);
        bmp = wxBitmap(img);
    }

    return bmp;
}

// static
wxIconBundle wxSTEditorArtProvider::GetDialogIconBundle()
{
    wxIcon icon1, icon2;
    icon1.CopyFromBitmap(wxArtProvider::GetBitmap(wxART_STEDIT_APP, wxART_OTHER, wxSTESmallIconSize));
    icon2.CopyFromBitmap(wxArtProvider::GetBitmap(wxART_STEDIT_APP, wxART_OTHER, wxSTEIconSize));

    wxIconBundle iconBundle(icon1);
    iconBundle.AddIcon(icon2);
    return iconBundle;
}

wxBitmap wxSTEditorArtProvider::CreateBitmap(const wxArtID& id,
                                             const wxArtClient& client,
                                             const wxSize& size)
{
    wxBitmap bmp = DoGetBitmap(id, client, size);

    return bmp; // ok to return invalid bitmap, the wxArtProvider will search other providers
}

wxIconBundle wxSTEditorArtProvider::CreateIconBundle(const wxArtID& id,
                                                     const wxArtClient& WXUNUSED(client))
{
    if (id == wxART_STEDIT_APP)
        return GetDialogIconBundle();

#if wxCHECK_VERSION(2,9,0)
    return wxNullIconBundle;
#else
    return wxIconBundle();
#endif
}
