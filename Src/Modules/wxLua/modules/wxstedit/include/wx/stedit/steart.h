///////////////////////////////////////////////////////////////////////////////
// Name:        steart.h
// Purpose:     wxSTEditorMenuManager
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     29/05/2010
// Copyright:   (c) John Labenski, Troels K, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file steart.h
/// @brief wxSTEditorArtProvider class and related functions.

#ifndef _STEART_H_
#define _STEART_H_

#include <wx/artprov.h>
#include "wx/stedit/stedefs.h"

// --------------------------------------------------------------------------
/// @name wxART_STEDIT_defines
/// wxArtIDs used by the wxSTEditor classes to get bitmaps.
/// Note: We cannot use the default wxArtProvider icons since we require a few
///       extra ones and mixing the two will look strange at best.
/// @{

#define wxART_STEDIT_APP            wxART_MAKE_ART_ID(wxART_STEDIT_APP)     ///< the pencil icon

#define wxART_STEDIT_NEW            wxART_MAKE_ART_ID(wxART_STEDIT_NEW)     ///< wxART_NEW
#define wxART_STEDIT_OPEN           wxART_MAKE_ART_ID(wxART_STEDIT_OPEN)    ///< wxART_FILE_OPEN
#define wxART_STEDIT_SAVE           wxART_MAKE_ART_ID(wxART_STEDIT_SAVE)    ///< wxART_FILE_SAVE
#define wxART_STEDIT_SAVEALL        wxART_MAKE_ART_ID(wxART_STEDIT_SAVEALL)
#define wxART_STEDIT_SAVEAS         wxART_MAKE_ART_ID(wxART_STEDIT_SAVEAS)  ///< wxART_FILE_SAVE_AS
#define wxART_STEDIT_PRINT          wxART_MAKE_ART_ID(wxART_STEDIT_PRINT)   ///< wxART_PRINT
#define wxART_STEDIT_PRINTPREVIEW   wxART_MAKE_ART_ID(wxART_STEDIT_PRINTPREVIEW)
#define wxART_STEDIT_PRINTSETUP     wxART_MAKE_ART_ID(wxART_STEDIT_PRINTSETUP)
#define wxART_STEDIT_PRINTPAGESETUP wxART_MAKE_ART_ID(wxART_STEDIT_PRINTPAGESETUP)
#define wxART_STEDIT_QUIT           wxART_MAKE_ART_ID(wxART_STEDIT_QUIT)    ///< wxART_QUIT
#define wxART_STEDIT_CUT            wxART_MAKE_ART_ID(wxART_STEDIT_CUT)     ///< wxART_CUT
#define wxART_STEDIT_COPY           wxART_MAKE_ART_ID(wxART_STEDIT_COPY)    ///< wxART_COPY
#define wxART_STEDIT_PASTE          wxART_MAKE_ART_ID(wxART_STEDIT_PASTE)   ///< wxART_PASTE
#define wxART_STEDIT_FIND           wxART_MAKE_ART_ID(wxART_STEDIT_FIND)    ///< wxART_FIND
#define wxART_STEDIT_FINDNEXT       wxART_MAKE_ART_ID(wxART_STEDIT_FINDNEXT)
#define wxART_STEDIT_FINDUP         wxART_MAKE_ART_ID(wxART_STEDIT_FINDUP)
#define wxART_STEDIT_FINDDOWN       wxART_MAKE_ART_ID(wxART_STEDIT_FINDDOWN)
#define wxART_STEDIT_REPLACE        wxART_MAKE_ART_ID(wxART_STEDIT_REPLACE) ///< wxART_FIND_AND_REPLACE
#define wxART_STEDIT_UNDO           wxART_MAKE_ART_ID(wxART_STEDIT_UNDO)    ///< wxART_UNDO
#define wxART_STEDIT_REDO           wxART_MAKE_ART_ID(wxART_STEDIT_REDO)    ///< wxART_REDO
#define wxART_STEDIT_CLEAR          wxART_MAKE_ART_ID(wxART_STEDIT_CLEAR)   ///< wxART_DELETE

/// @}
/// @name wxArtIDs for each preference dialog page
/// @{

#define wxART_STEDIT_PREFDLG_VIEW       wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_VIEW)
#define wxART_STEDIT_PREFDLG_TABSEOL    wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_TABSEOL)
#define wxART_STEDIT_PREFDLG_FOLDWRAP   wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_FOLDWRAP)
#define wxART_STEDIT_PREFDLG_PRINT      wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_PRINT)
#define wxART_STEDIT_PREFDLG_LOADSAVE   wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_LOADSAVE)
#define wxART_STEDIT_PREFDLG_HIGHLIGHT  wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_HIGHLIGHT)
#define wxART_STEDIT_PREFDLG_STYLES     wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_STYLES)
#define wxART_STEDIT_PREFDLG_LANGS      wxART_MAKE_ART_ID(wxART_STEDIT_PREFDLG_LANGS)

/// @}
// --------------------------------------------------------------------------
/// @name wxSTEditorArtProvider Default Icon Sizes
/// @{

/// Normal icon size for this platform from wxSystemSettings::GetMetric(wxSYS_ICON_X and wxSYS_ICON_Y)
WXDLLIMPEXP_DATA_STEDIT(extern const wxSize) wxSTEIconSize;
/// Small icon size for this platform from wxSystemSettings::GetMetric(wxSYS_SMALLICON_X and wxSYS_SMALLICON_Y)
WXDLLIMPEXP_DATA_STEDIT(extern const wxSize) wxSTESmallIconSize;

/// @}
// --------------------------------------------------------------------------
/// @name wxSTEditorArtProvider Convenience Macros
/// @{

/// Convenience macro to get an appropriate menu bitmap for the menu client.
#define STE_ARTMENU(id) wxArtProvider::GetBitmap(id, wxART_MENU)
/// Convenience macro to get an appropriate toolbar bitmap for the toolbar client.
#define STE_ARTTOOL(id) wxArtProvider::GetBitmap(id, wxART_TOOLBAR)

/// @}
//---------------------------------------------------------------------------
/** @class wxSTEditorArtProvider
    @brief A holding place for our art for menu items and toolbar tools.

    The XPM files located in the "art" directory contain the images used by
    this art provider.
    If you wish to use your own images, create your own wxArtProvider and call
    wxArtProvider::PushProvider(new myArtProvider);
    and have it return your own bitmaps using the wxArtIDs set above
    in your overridden virtual CreateBitmap(...) function.
    @see wxART_STEDIT_defines
*/ // -----------------------------------------------------------------------

class WXDLLIMPEXP_STEDIT wxSTEditorArtProvider : public wxArtProvider
{
public:
    wxSTEditorArtProvider();
    virtual ~wxSTEditorArtProvider() {}

    // ----------------------------------------------------------------------

    /// Get a wxBitmap for one of the wxART_STEDIT_* wxArtIDs.
    ///   returns an invalid bitmap for all other wxArtIDs.
    /// Respects GetSizeHint(client) if size == wxDefaultSize
    /// User created wxArtProviders may call this function to guarantee that
    ///   they get one of our bitmaps if they wish.
    /// @see wxART_STEDIT_defines
    static wxBitmap DoGetBitmap(const wxArtID& id,
                                const wxArtClient& client,
                                const wxSize& size = wxDefaultSize);

    /// Helper function to resize the input bitmap to have the given size.
    static wxBitmap Resize(const wxBitmap& bmp, const wxSize& size);

    /// Get a wxIconBundle of the wxART_STEDIT_APP icon for wxSTEdit dialogs
    /// This function exists since static wxArtProvider::GetIconBundle()
    ///  is only in >= 2.9.
    static wxIconBundle GetDialogIconBundle();

protected:

    // ----------------------------------------------------------------------
    /// Overridden virtual functions from wxArtProvider to get bitmaps
    /// Always use the static wxArtProvider functions to get bitmaps

    virtual wxBitmap CreateBitmap(const wxArtID& id,
                                  const wxArtClient& client,
                                  const wxSize& size);

    /// This function is in wx >= 29.
    virtual wxIconBundle CreateIconBundle(const wxArtID& id,
                                          const wxArtClient& client);

    static wxBitmap m_app_large;
    static wxBitmap m_app_small;
};

#endif  // _STEART_H_
