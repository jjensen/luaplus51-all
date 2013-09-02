///////////////////////////////////////////////////////////////////////////////
// Name:        stemenum.cpp
// Purpose:     wxSTEditorMenuManager
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// RCS-ID:
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include <wx/srchctrl.h>

#include "wx/stedit/stemenum.h"
#include "wx/stedit/stedit.h"
#include "wx/stedit/steart.h"
#include "wxext.h"

//-----------------------------------------------------------------------------
// wxSTEditorMenuManager - a holding place for menu generating code
//-----------------------------------------------------------------------------

wxSTEditorMenuManager::~wxSTEditorMenuManager()
{
    delete m_accelEntryArray;
}

void wxSTEditorMenuManager::Init()
{
    m_enabledEditorItems = true;
    m_menuOptionTypes = 0;
    m_menuItemTypes.Add(0, STE_MENU_HELP_MENU+1);
    m_toolBarToolTypes = 0;

    m_accels_dirty = true;
    m_accelEntryArray = new wxArrayAcceleratorEntry();
}

void wxSTEditorMenuManager::InitAcceleratorArray() const
{
    // No need to recreate if nothing has changed.
    if (!m_accels_dirty) return;

    // After running this function, they're all created.
    m_accels_dirty = false;

    if (m_accelEntryArray->GetCount() > 0)
        m_accelEntryArray->Clear();

    // File menu items --------------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_FILE_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_NEW));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_OPEN));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_SAVE));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_SAVEAS));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_PRINT));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_PREVIEW));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_EXIT));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_ALT,                  WXK_RETURN, ID_STE_PROPERTIES));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'W',        ID_STN_CLOSE_PAGE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'W',        ID_STN_CLOSE_ALL));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'A',        ID_STN_SAVE_ALL));
    }

    // Edit menu items --------------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_EDIT_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_UNDO));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_REDO));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_CUT));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_COPY));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_PASTE));
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_SELECTALL));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'V', ID_STE_PASTE_NEW));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_ALT,   'V', ID_STE_PASTE_RECT));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'L', ID_STE_LINE_CUT));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'T', ID_STE_LINE_COPY));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'C', ID_STE_COPYPATH));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'L', ID_STE_LINE_DELETE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 ' ', ID_STE_COMPLETEWORD));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'T', ID_STE_LINE_TRANSPOSE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'D', ID_STE_LINE_DUPLICATE));
      //m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'R', ID_STE_PREF_SELECTION_MODE));
    #ifdef __UNIX__
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'C', ID_STE_COPY_PRIMARY));
    #endif // __UNIX__
    }

    // View menu items  -------------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_VIEW_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_FULLSCREEN, ID_STE_SHOW_FULLSCREEN));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,   WXK_F10,        ID_STE_VIEW_NONPRINT));
    }

    // Search menu items  -------------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_SEARCH_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorHelper::GetStockAccelerator(wxID_FIND));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,   'H',    wxID_REPLACE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F3, ID_STE_FIND_NEXT));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_SHIFT , WXK_F3, ID_STE_FIND_PREV));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F2, ID_STE_FIND_DOWN));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,   'G',    ID_STE_GOTO_LINE));
    }

    // Insert menu items  -------------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_INSERT_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL, 'I', ID_STE_INSERT_TEXT));
    }

    // Tools menu items  -------------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_TOOLS_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'U', ID_STE_UPPERCASE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'U', ID_STE_LOWERCASE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'J', ID_STE_LINES_JOIN));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'K', ID_STE_LINES_SPLIT));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_ALT,   'E', ID_STE_TRAILING_WHITESPACE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL,                 'E', ID_STE_REMOVE_CHARSAROUND));
    }

    // Bookmark menu items  ---------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_BOOKMARK_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F4, ID_STE_BOOKMARK_TOGGLE));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_SHIFT,  WXK_F5, ID_STE_BOOKMARK_FIRST));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F5, ID_STE_BOOKMARK_PREVIOUS));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F6, ID_STE_BOOKMARK_NEXT));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_SHIFT,  WXK_F6, ID_STE_BOOKMARK_LAST));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_SHIFT,  WXK_F4, ID_STE_BOOKMARK_CLEAR));
    }

    // Preference menu items  -------------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_PREFS_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL, WXK_F9, ID_STE_PREFERENCES));
    }

    // Window menu items  -----------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_WINDOW_MENU))
    {
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'L',    ID_STS_SPLIT_HORIZ));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'T',    ID_STS_SPLIT_VERT));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_SHIFT,                WXK_F8, ID_STN_WIN_PREVIOUS));
        m_accelEntryArray->Add(wxAcceleratorEntry(wxACCEL_NORMAL,               WXK_F8, ID_STN_WIN_NEXT));
    }

    // Help menu items  -----------------------------------------------------
    if (GetMenuItemTypes(STE_MENU_HELP_MENU))
    {
    }
}

void wxSTEditorMenuManager::CreateForSinglePage()
{
    m_menuOptionTypes  = 0;
    m_menuItemTypes[STE_MENU_FILE_MENU]     = STE_MENU_FILE_DEFAULT;
    m_menuItemTypes[STE_MENU_EDIT_MENU]     = STE_MENU_EDIT_DEFAULT;
    m_menuItemTypes[STE_MENU_SEARCH_MENU]   = STE_MENU_SEARCH_DEFAULT;
    m_menuItemTypes[STE_MENU_TOOLS_MENU]    = STE_MENU_TOOLS_DEFAULT;
    m_menuItemTypes[STE_MENU_VIEW_MENU]     = STE_MENU_VIEW_DEFAULT;
    m_menuItemTypes[STE_MENU_BOOKMARK_MENU] = STE_MENU_BOOKMARK_DEFAULT;
    m_menuItemTypes[STE_MENU_PREFS_MENU]    = STE_MENU_PREFS_DEFAULT;
    m_menuItemTypes[STE_MENU_WINDOW_MENU]   = STE_MENU_WINDOW_DEFAULT;
    m_menuItemTypes[STE_MENU_HELP_MENU]     = STE_MENU_HELP_DEFAULT;
    m_toolBarToolTypes = STE_TOOLBAR_TOOLS;
}

void wxSTEditorMenuManager::CreateForNotebook()
{
    m_menuOptionTypes  = STE_MENU_NOTEBOOK;
    m_menuItemTypes[STE_MENU_FILE_MENU]     = STE_MENU_FILE_DEFAULT;
    m_menuItemTypes[STE_MENU_EDIT_MENU]     = STE_MENU_EDIT_DEFAULT;
    m_menuItemTypes[STE_MENU_SEARCH_MENU]   = STE_MENU_SEARCH_DEFAULT;
    m_menuItemTypes[STE_MENU_TOOLS_MENU]    = STE_MENU_TOOLS_DEFAULT;
    m_menuItemTypes[STE_MENU_VIEW_MENU]     = STE_MENU_VIEW_DEFAULT;
    m_menuItemTypes[STE_MENU_BOOKMARK_MENU] = STE_MENU_BOOKMARK_DEFAULT;
    m_menuItemTypes[STE_MENU_PREFS_MENU]    = STE_MENU_PREFS_DEFAULT;
    m_menuItemTypes[STE_MENU_WINDOW_MENU]   = STE_MENU_WINDOW_DEFAULT|STE_MENU_WINDOW_FILECHOOSER|STE_MENU_WINDOW_PREVNEXT|STE_MENU_WINDOW_WINDOWS;
    m_menuItemTypes[STE_MENU_HELP_MENU]     = STE_MENU_HELP_DEFAULT;
    m_toolBarToolTypes = STE_TOOLBAR_TOOLS;
}

wxMenu *wxSTEditorMenuManager::CreateEditorPopupMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_;
    if (!menu) menu = new wxMenu;

    bool add_sep = false;

    wxMenu *fileMenu     = GetMenuItemTypes(STE_MENU_FILE_MENU)     != 0 ? CreateFileMenu()     : NULL;
    wxMenu *editMenu     = GetMenuItemTypes(STE_MENU_EDIT_MENU)     != 0 ? CreateEditMenu()     : NULL;
    wxMenu *searchMenu   = GetMenuItemTypes(STE_MENU_SEARCH_MENU)   != 0 ? CreateSearchMenu()   : NULL;
    wxMenu *toolsMenu    = GetMenuItemTypes(STE_MENU_TOOLS_MENU)    != 0 ? CreateToolsMenu()    : NULL;
    wxMenu *insertMenu   = GetMenuItemTypes(STE_MENU_INSERT_MENU)   != 0 ? CreateInsertMenu()   : NULL;
    wxMenu *viewMenu     = GetMenuItemTypes(STE_MENU_VIEW_MENU)     != 0 ? CreateViewMenu()     : NULL;
    wxMenu *bookmarkMenu = GetMenuItemTypes(STE_MENU_BOOKMARK_MENU) != 0 ? CreateBookmarkMenu() : NULL;
    wxMenu *prefMenu     = GetMenuItemTypes(STE_MENU_PREFS_MENU)    != 0 ? CreatePreferenceMenu() : NULL;
    wxMenu *windowMenu   = GetMenuItemTypes(STE_MENU_WINDOW_MENU)   != 0 ? CreateWindowMenu()   : NULL;
    wxMenu *helpMenu     = GetMenuItemTypes(STE_MENU_HELP_MENU)     != 0 ? CreateHelpMenu()     : NULL;

    if (fileMenu)
    {
        menu->Append(ID_STE_MENU_FILE, wxGetStockLabel(wxID_FILE), fileMenu);
        add_sep = true;
    }

    if (editMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_EDIT, wxGetStockLabel(wxID_EDIT), editMenu);
        add_sep = true;
    }

    if (viewMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_VIEW, _("&View"), viewMenu);
        add_sep = true;
    }

    if (searchMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_SEARCH, _("&Search"), searchMenu);
        add_sep = true;
    }

    if (toolsMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_TOOLS, _("&Tools"), toolsMenu);
        add_sep = true;
    }

    if (insertMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_INSERT, _("&Insert"), insertMenu);
        add_sep = true;
    }

    if (bookmarkMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_BOOKMARK, _("&Bookmarks"), bookmarkMenu);
        add_sep = true;
    }

    if (prefMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_PREF, wxGetStockLabel(wxID_PREFERENCES), prefMenu);
        add_sep = true;
    }

    if (windowMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_PREF, _("&Window"), windowMenu);
    }

    if (helpMenu)
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(ID_STE_MENU_HELP, wxGetStockLabel(wxID_HELP), helpMenu);
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu* wxSTEditorMenuManager::CreateSplitterPopupMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_;
    if (!menu) menu = new wxMenu;

    menu->AppendRadioItem(ID_STS_UNSPLIT,     _("&Unsplit editor"),            _("Unsplit the editor"));
    menu->AppendRadioItem(ID_STS_SPLIT_HORIZ, _("Split editor &horizontally"), _("Split editor horizontally"));
    menu->AppendRadioItem(ID_STS_SPLIT_VERT,  _("Split editor &vertically"),   _("Split editor vertically"));

    return menu;
}
wxMenu* wxSTEditorMenuManager::CreateNotebookPopupMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_;
    if (!menu) menu = new wxMenu;

    menu->Append(wxID_NEW,         _("&Add empty page"));
    menu->Append(wxID_OPEN,        _("&Open file(s)..."));
    menu->Append(ID_STN_SAVE_ALL,  _("&Save all files"));

    menu->AppendSeparator();
    // These are empty and are filled by wxSTEditorNotebook::UpdateMenuItems()
    wxMenu *gotoMenu  = new wxMenu;
    wxMenu *closeMenu = new wxMenu;

    menu->Append(ID_STN_WIN_PREVIOUS, _("Previous page"));
    menu->Append(ID_STN_WIN_NEXT,     _("Next page"));

    menu->Append(ID_STN_MENU_GOTO,    _("Goto page"), gotoMenu);
    menu->AppendSeparator();
    menu->Append(ID_STN_CLOSE_PAGE,       _("Close current page"));
    menu->Append(ID_STN_CLOSE_ALL,        _("Close all pages..."));
    menu->Append(ID_STN_CLOSE_ALL_OTHERS, _("Close all other pages"));
    menu->Append(ID_STN_MENU_CLOSE,       _("Close page"), closeMenu);
    menu->AppendSeparator();
    menu->Append(ID_STN_WINDOWS,          _("&Windows..."), _("Manage opened windows"));

    return menu;
}

bool wxSTEditorMenuManager::CreateMenuBar(wxMenuBar *menuBar, bool for_frame) const
{
    wxCHECK_MSG(menuBar, false, wxT("Invalid wxMenuBar"));
    size_t menu_count = menuBar->GetMenuCount();

    // Note! here's where we specify that we want menu items for the frame
    int was_set_frame = HasMenuOptionType(STE_MENU_FRAME);
    if (!was_set_frame && for_frame)
        ((wxSTEditorMenuManager*)this)->SetMenuOptionType(STE_MENU_FRAME, true);

    wxMenu *fileMenu     = GetMenuItemTypes(STE_MENU_FILE_MENU)     != 0 ? CreateFileMenu()     : NULL;
    wxMenu *editMenu     = GetMenuItemTypes(STE_MENU_EDIT_MENU)     != 0 ? CreateEditMenu()     : NULL;
    wxMenu *searchMenu   = GetMenuItemTypes(STE_MENU_SEARCH_MENU)   != 0 ? CreateSearchMenu()   : NULL;
    wxMenu *toolsMenu    = GetMenuItemTypes(STE_MENU_TOOLS_MENU)    != 0 ? CreateToolsMenu()    : NULL;
    wxMenu *insertMenu   = GetMenuItemTypes(STE_MENU_INSERT_MENU)   != 0 ? CreateInsertMenu()   : NULL;
    wxMenu *viewMenu     = GetMenuItemTypes(STE_MENU_VIEW_MENU)     != 0 ? CreateViewMenu()     : NULL;
    wxMenu *bookmarkMenu = GetMenuItemTypes(STE_MENU_BOOKMARK_MENU) != 0 ? CreateBookmarkMenu() : NULL;
    wxMenu *prefMenu     = GetMenuItemTypes(STE_MENU_PREFS_MENU)    != 0 ? CreatePreferenceMenu() : NULL;
    wxMenu *windowMenu   = GetMenuItemTypes(STE_MENU_WINDOW_MENU)   != 0 ? CreateWindowMenu()   : NULL;
    wxMenu *helpMenu     = GetMenuItemTypes(STE_MENU_HELP_MENU)     != 0 ? CreateHelpMenu()     : NULL;


    if (fileMenu)     menuBar->Append(fileMenu,     wxGetStockLabel(wxID_FILE));
    if (editMenu)     menuBar->Append(editMenu,     wxGetStockLabel(wxID_EDIT));
    if (viewMenu)     menuBar->Append(viewMenu,     _("&View"));
    if (searchMenu)   menuBar->Append(searchMenu,   _("&Search"));
    if (toolsMenu)    menuBar->Append(toolsMenu,    _("&Tools"));
    if (insertMenu)   menuBar->Append(insertMenu,   _("&Insert"));
    if (bookmarkMenu) menuBar->Append(bookmarkMenu, _("&Bookmarks"));
    if (prefMenu)     menuBar->Append(prefMenu,     wxGetStockLabel(wxID_PREFERENCES));
    if (windowMenu)   menuBar->Append(windowMenu,   _("&Window"));
    if (helpMenu)     menuBar->Append(helpMenu,     wxGetStockLabel(wxID_HELP));

    // reset the frame bit if it wasn't set
    if (!was_set_frame)
        ((wxSTEditorMenuManager*)this)->SetMenuOptionType(STE_MENU_FRAME, false);

    return menuBar->GetMenuCount() > menu_count;
}

static wxString wxToolBarTool_MakeShortHelp(const wxArrayAcceleratorEntry& accel, int id)
{
    return wxToolBarTool_MakeShortHelp(wxGetStockLabelEx(id, wxSTOCK_PLAINTEXT), accel, id);
}

bool wxSTEditorMenuManager::CreateToolBar(wxToolBar *tb) const
{
    wxCHECK_MSG(tb, false, wxT("Invalid toolbar"));

    InitAcceleratorArray(); // ALWAYS call this before accessing m_accelEntryArray

    size_t tools_count = tb->GetToolsCount();

    if (HasToolbarToolType(STE_TOOLBAR_FILE_NEW))
    {
        tb->AddTool(wxID_NEW, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_NEW), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_NEW), _("Clear editor for new file"));
    }
    if (HasToolbarToolType(STE_TOOLBAR_FILE_OPEN))
    {
        tb->AddTool(wxID_OPEN, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_OPEN), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_OPEN), _("Open a file to edit"));
    }
    if (HasToolbarToolType(STE_TOOLBAR_FILE_SAVE))
    {
        tb->AddTool(wxID_SAVE,   wxEmptyString, STE_ARTTOOL(wxART_STEDIT_SAVE  ), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_SAVE  ), _("Save current file"));
        tb->AddTool(wxID_SAVEAS, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_SAVEAS), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_SAVEAS), _("Save to a specific filename"));
        tb->EnableTool(wxID_SAVE, false);

        if (HasMenuOptionType(STE_MENU_NOTEBOOK))
        {
            tb->AddTool(ID_STN_SAVE_ALL, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_SAVEALL), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Save all files"), *m_accelEntryArray, ID_STN_SAVE_ALL), _("Save all open files"));
            tb->EnableTool(ID_STN_SAVE_ALL, false);
        }
    }
    if (HasToolbarToolType(STE_TOOLBAR_PRINT))
    {
        if (tb->GetToolsCount()) tb->AddSeparator();
        tb->AddTool(wxID_PRINT  , wxEmptyString, STE_ARTTOOL(wxART_STEDIT_PRINT)       , wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_PRINT), wxGetStockHelpString(wxID_PRINT));
        tb->AddTool(wxID_PREVIEW, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_PRINTPREVIEW), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_PREVIEW), wxGetStockHelpString(wxID_PREVIEW));
    }
    if (HasToolbarToolType(STE_TOOLBAR_EDIT_CUTCOPYPASTE))
    {
        if (tb->GetToolsCount()) tb->AddSeparator();
        tb->AddTool(wxID_CUT,    wxEmptyString, STE_ARTTOOL(wxART_STEDIT_CUT  ), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_CUT  ), _("Cut selected text"));
        tb->AddTool(wxID_COPY,   wxEmptyString, STE_ARTTOOL(wxART_STEDIT_COPY ), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_COPY ), _("Copy selected text"));
        tb->AddTool(wxID_PASTE,  wxEmptyString, STE_ARTTOOL(wxART_STEDIT_PASTE), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_PASTE), _("Paste text at cursor"));
    }
    if (HasToolbarToolType(STE_TOOLBAR_EDIT_UNDOREDO))
    {
        if (tb->GetToolsCount()) tb->AddSeparator();
        tb->AddTool(wxID_UNDO, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_UNDO), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_UNDO), _("Undo last editing"));
        tb->AddTool(wxID_REDO, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_REDO), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_REDO), _("Redo last undo"));
    }
    if (HasToolbarToolType(STE_TOOLBAR_EDIT_FINDREPLACE))
    {
        if (tb->GetToolsCount()) tb->AddSeparator();
        //tb->AddTool(ID_STE_FIND_DOWN, _("Search direction"), STE_ARTTOOL(wxART_STEDIT_FINDDOWN), wxNullBitmap, wxITEM_CHECK, _("Search direction"), _("Search direction for next occurance in document"));
        tb->AddTool(wxID_FIND       , wxEmptyString, STE_ARTTOOL(wxART_STEDIT_FIND    ), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_FIND), _("Find text in document..."));
        tb->AddTool(ID_STE_FIND_NEXT, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_FINDDOWN), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Find next"), *m_accelEntryArray, ID_STE_FIND_NEXT), _("Find next occurance in document"));
        tb->AddTool(ID_STE_FIND_PREV, wxEmptyString, STE_ARTTOOL(wxART_STEDIT_FINDUP  ), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Find previous"), *m_accelEntryArray, ID_STE_FIND_PREV), _("Find previous occurance in document"));
        tb->AddTool(wxID_REPLACE    , wxEmptyString, STE_ARTTOOL(wxART_STEDIT_REPLACE ), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(*m_accelEntryArray, wxID_REPLACE), _("Replace text in document"));
    }
    if (HasToolbarToolType(STE_TOOLBAR_EDIT_SEARCH_CTRL))
    {
        if (tb->GetToolsCount()) tb->AddSeparator();
        wxSearchCtrl *searchCtrl = new wxSearchCtrl(tb, ID_STE_TOOLBAR_SEARCHCTRL, wxT(""), wxDefaultPosition, wxSize(200, -1), wxTE_PROCESS_ENTER);
        //searchCtrl->SetMenu(new wxMenu);
        tb->AddControl(searchCtrl);
    }
    if (HasToolbarToolType(STE_TOOLBAR_BOOKMARK))
    {
        if (tb->GetToolsCount()) tb->AddSeparator();
        tb->AddTool(ID_STE_BOOKMARK_TOGGLE,   wxEmptyString, STE_ARTTOOL(wxART_ADD_BOOKMARK), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Toggle bookmark"),   *m_accelEntryArray, ID_STE_BOOKMARK_TOGGLE),   _("Toggle a bookmark on cursor line"));
        tb->AddTool(ID_STE_BOOKMARK_FIRST,    wxEmptyString, STE_ARTTOOL(wxART_GO_UP),        wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("First bookmark"),    *m_accelEntryArray, ID_STE_BOOKMARK_FIRST),    _("Goto first bookmark"));
        tb->AddTool(ID_STE_BOOKMARK_PREVIOUS, wxEmptyString, STE_ARTTOOL(wxART_GO_BACK),      wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Previous bookmark"), *m_accelEntryArray, ID_STE_BOOKMARK_PREVIOUS), _("Goto previous bookmark"));
        tb->AddTool(ID_STE_BOOKMARK_NEXT,     wxEmptyString, STE_ARTTOOL(wxART_GO_FORWARD),   wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Next bookmark"),     *m_accelEntryArray, ID_STE_BOOKMARK_NEXT),     _("Goto next bookmark"));
        tb->AddTool(ID_STE_BOOKMARK_LAST,     wxEmptyString, STE_ARTTOOL(wxART_GO_DOWN),      wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Last bookmark"),     *m_accelEntryArray, ID_STE_BOOKMARK_LAST),     _("Goto last bookmark"));
        tb->AddTool(ID_STE_BOOKMARK_CLEAR,    wxEmptyString, STE_ARTTOOL(wxART_DEL_BOOKMARK), wxNullBitmap, wxITEM_NORMAL, ::wxToolBarTool_MakeShortHelp(_("Clear bookmarks"),   *m_accelEntryArray, ID_STE_BOOKMARK_CLEAR),    _("Clear all bookmarks"));
    }
    tb->Realize();

    return tb->GetToolsCount() > tools_count;
}

wxMenu *wxSTEditorMenuManager::CreateFileMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();
    bool add_sep = false;

    if (HasMenuItemType(STE_MENU_FILE_MENU, STE_MENU_FILE_NEW))
    {
        menu->Append(MenuItem(menu, wxID_NEW, wxGetStockLabel(wxID_NEW), _("Clear contents and start a new file"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_NEW)));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_FILE_MENU, STE_MENU_FILE_OPEN))
    {
        menu->Append(MenuItem(menu, wxID_OPEN, wxGetStockLabelEx(wxID_OPEN), _("Open file"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_OPEN)));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_FILE_MENU, STE_MENU_FILE_CLOSE) && HasMenuOptionType(STE_MENU_FRAME))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STN_CLOSE_PAGE, _("&Close current page"), _("Close current page"));
        if (HasMenuOptionType(STE_MENU_NOTEBOOK))
        {
            menu->Append(ID_STN_CLOSE_ALL,        _("Close all pages..."), _("Close all pages"));
            menu->Append(ID_STN_CLOSE_ALL_OTHERS, _("Close all other pages"), _("Close all other pages"));
        }

        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_FILE_MENU, STE_MENU_FILE_SAVE))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(MenuItem(menu, wxID_SAVE, wxGetStockLabel(wxID_SAVE), _("Save current file"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_SAVE)));
        menu->Enable(wxID_SAVE, false);
        menu->Append(MenuItem(menu, wxID_SAVEAS, wxGetStockLabelEx(wxID_SAVEAS), _("Save as file"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_SAVEAS)));
        if (HasMenuOptionType(STE_MENU_NOTEBOOK))
        {
            menu->Append(MenuItem(menu, ID_STN_SAVE_ALL, _("Save A&ll"), _("Save all files"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_SAVEALL)));
            menu->Enable(ID_STN_SAVE_ALL, false);
        }
        menu->Append(MenuItem(menu, wxID_REVERT, wxGetStockLabelEx(wxID_REVERT), _("Revert to saved version of the file"), wxITEM_NORMAL));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_FILE_MENU, STE_MENU_FILE_EXPORT))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(MenuItem(menu, ID_STE_EXPORT, _("E&xport..."), _("Export to file"), wxITEM_NORMAL));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_FILE_MENU, STE_MENU_FILE_PROPERTY))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_PROPERTIES, wxGetStockLabelEx(wxID_PROPERTIES), _("Show document properties dialog"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_FILE_MENU, STE_MENU_FILE_PRINT))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(MenuItem(menu, wxID_PRINT,              wxGetStockLabelEx(wxID_PRINT  ), _("Print current document"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_PRINT)));
        menu->Append(MenuItem(menu, wxID_PREVIEW,            wxGetStockLabelEx(wxID_PREVIEW), _("Print preview of the current document"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_PRINTPREVIEW)));
   #ifdef __WXMSW__
        // The wxID_PRINT_SETUP dialog is the same as the wxID_PRINT one, at least on Windows; confusing to the user
   #else
        menu->Append(MenuItem(menu, wxID_PRINT_SETUP,        _("Printer set&up..."), _("Setup the printer"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_PRINTSETUP)));
   #endif
        menu->Append(MenuItem(menu, ID_STE_PRINT_PAGE_SETUP, _("Printer pa&ge setup..."), _("Setup the printout page"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_PRINTPAGESETUP)));
        menu->Append(MenuItem(menu, ID_STE_PRINT_OPTIONS,    _("Printer options..."), _("Set other printout options"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_PRINTPREVIEW)));
    }

    if (HasMenuOptionType(STE_MENU_FRAME))
    {
        if (add_sep) menu->AppendSeparator();
        menu->Append(MenuItem(menu, wxID_EXIT, wxGetStockLabelEx(wxID_EXIT), _("Exit editor"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_QUIT)));
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateEditMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();
    bool add_sep = false;

    if (HasMenuItemType(STE_MENU_EDIT_MENU, STE_MENU_EDIT_UNDOREDO) && !HasMenuOptionType(STE_MENU_READONLY))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(MenuItem(menu, wxID_UNDO, wxGetStockLabel(wxID_UNDO), _("Undo last operation"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_UNDO)));
        menu->Append(MenuItem(menu, wxID_REDO, wxGetStockLabel(wxID_REDO), _("Redo last undo"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_REDO)));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_EDIT_MENU, STE_MENU_EDIT_CUTCOPYPASTE))
    {
        if (add_sep) menu->AppendSeparator();

        if (!HasMenuOptionType(STE_MENU_READONLY))
            menu->Append(MenuItem(menu, wxID_CUT,  wxGetStockLabel(wxID_CUT), _("Cut selected text to clipboard"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_CUT)));
        menu->Append(MenuItem(menu, wxID_COPY,  wxGetStockLabel(wxID_COPY), _("Copy selected text to clipboard"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_COPY)));
        menu->Append(ID_STE_COPY_HTML, _("Copy as &HTML"), _("Copy selected text to clipboard with text markup"));
#ifdef __UNIX__
        menu->Append(ID_STE_COPY_PRIMARY,  _("Copy primary"), _("Copy selected text to primary clipboard"));
#endif // __UNIX__
        if (!HasMenuOptionType(STE_MENU_READONLY))
        {
            menu->Append(MenuItem(menu, wxID_PASTE, wxGetStockLabel(wxID_PASTE), _("Paste text from clipboard"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_PASTE)));
            menu->Append(ID_STE_PASTE_RECT, _("Paste &Rectangle"), _("Paste rectangular text from clipboard (select with Shift+Alt)"));
        }

        if (HasMenuOptionType(STE_MENU_NOTEBOOK))
        {
            // FIXME - this is probably not needed, it's pretty easy to simply do new then paste
            //menu->Append(ID_STE_PASTE_NEW, _("Paste as &New"), _("Paste text from clipboard into new notebook tab"));
        }

        // FIXME - ID_STE_PREF_SELECTION_MODE remmed out since I can't make it work in GTK
        //menu->AppendCheckItem(ID_STE_PREF_SELECTION_MODE, _("Rectan&gular Selection"), _("Rectangular selections for cut/copy/paste"));

        // WXK_DELETE - no accelerator to allow scintilla to handle it appropriately
        menu->Append(MenuItem(menu, wxID_CLEAR, wxGetStockLabel(wxID_DELETE), _("Delete selection"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_CLEAR)));

        add_sep = true;
    }
    if (add_sep) menu->AppendSeparator();
    menu->Append(wxID_SELECTALL, wxGetStockLabelEx(wxID_SELECTALL), _("Selects entire document"));
    add_sep = true;
    if (HasMenuItemType(STE_MENU_EDIT_MENU, STE_MENU_EDIT_LINE))
    {
        if (add_sep) menu->AppendSeparator();

        wxMenu *m = new wxMenu();

        // FIXME - these aren't Ctrl-L since the stc handles the keys already (works everywhere?)
        if (!HasMenuOptionType(STE_MENU_READONLY))
            m->Append(ID_STE_LINE_CUT,   _("Line Cu&t"), _("Cut current line to clipboard"));

        m->Append(ID_STE_LINE_COPY,  _("Line &Copy"), _("Copy current line to clipboard"));

        if (!HasMenuOptionType(STE_MENU_READONLY))
        {
            m->Append(ID_STE_LINE_DELETE,    _("Line &Delete"),    _("Delete current line"));
            m->Append(ID_STE_LINE_TRANSPOSE, _("Line &Transpose"), _("Transpose current line upwards"));
            m->Append(ID_STE_LINE_DUPLICATE, _("Line D&uplicate"), _("Duplicate current line"));
        }

        menu->Append(ID_STE_MENU_LINE, _("L&ine Editing"), m);
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_EDIT_MENU, STE_MENU_EDIT_READONLY) && !HasMenuOptionType(STE_MENU_READONLY))
    {
        if (add_sep) menu->AppendSeparator();

        menu->AppendCheckItem(ID_STE_READONLY, _("Read only"), _("Make document read only"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_EDIT_MENU, STE_MENU_EDIT_COMPLETEWORD) && !HasMenuOptionType(STE_MENU_READONLY))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_COMPLETEWORD, _("Complete w&ord"), _("Complete word at cursor"));
        add_sep = true;
    }

    if (HasMenuItemType(STE_MENU_EDIT_MENU, STE_MENU_EDIT_COPYPATH))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_COPYPATH, _("Copy &path"), _("Copy full path to clipboard"));
        add_sep = true;
    }

    if ( (menu_ == NULL) && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateSearchMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();
    bool add_sep = false;

    if (HasMenuItemType(STE_MENU_SEARCH_MENU, STE_MENU_SEARCH_FINDREPLACE))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(MenuItem(menu, wxID_FIND, wxGetStockLabelEx(wxID_FIND), _("Find text"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_FIND)));
        menu->Append(MenuItem(menu, ID_STE_FIND_NEXT, _("Find &Next"),      _("Find next occurance"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_FINDNEXT)));
        menu->Append(MenuItem(menu, ID_STE_FIND_PREV, _("Find &Previous"),  _("Find previous occurance"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_FINDUP)));
        menu->AppendCheckItem(ID_STE_FIND_DOWN,       _("Search For&ward"), _("Search forward/reverse in document"));
        if (!HasMenuOptionType(STE_MENU_READONLY))
            menu->Append(MenuItem(menu, wxID_REPLACE, wxGetStockLabelEx(wxID_REPLACE), _("Replace text"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_REPLACE)));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_SEARCH_MENU, STE_MENU_SEARCH_GOTOLINE))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_GOTO_LINE, _("&Go to Line..."), _("Goto line number"));
        add_sep = true;
    }

    if ((menu_ == NULL) && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateToolsMenu(wxMenu *menu_) const
{
    // all of these modify the document
    if (HasMenuOptionType(STE_MENU_READONLY))
        return menu_;

    wxMenu *menu = menu_ ? menu_ : new wxMenu();
    bool add_sep = false;

    if (HasMenuItemType(STE_MENU_TOOLS_MENU, STE_MENU_TOOLS_CASE))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_UPPERCASE, _("Selection &uppercase"), _("Convert the selected text to uppercase"));
        menu->Append(ID_STE_LOWERCASE, _("Selection &lowercase"), _("Convert the selected text to lowercase"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_TOOLS_MENU, STE_MENU_TOOLS_INDENT))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_INCREASE_INDENT, _("&Increase indent"), _("Increase indent of selected text or current line"));
        menu->Append(ID_STE_DECREASE_INDENT, _("&Decrease indent"), _("Decrease indent of selected text or current line"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_TOOLS_MENU, STE_MENU_TOOLS_JOINSPLIT))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_LINES_JOIN,  _("&Join selected lines"),  _("Join selected lines together"));
        menu->Append(ID_STE_LINES_SPLIT, _("&Split selected lines"), _("Split selected lines to edge marker column"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_TOOLS_MENU, STE_MENU_TOOLS_TABS_SP))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_TABS_TO_SPACES, _("Convert &tabs to spaces"), _("Convert tabs to spaces in selection or current line"));
        menu->Append(ID_STE_SPACES_TO_TABS, _("Convert s&paces to tabs"), _("Convert spaces to tabs in selection or current line"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_TOOLS_MENU, STE_MENU_TOOLS_EOL))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_CONVERT_EOL, _("Convert &EOL characters..."), _("Convert all end of line characters in doc"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_TOOLS_MENU, STE_MENU_TOOLS_WHITE))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_TRAILING_WHITESPACE, _("Remove trailing &whitespace"),  _("Remove whitespace at the ends of lines"));
        menu->Append(ID_STE_REMOVE_CHARSAROUND,  _("Remove w&hitespace at cursor"), _("Remove whitespace before and after cursor"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_TOOLS_MENU, STE_MENU_TOOLS_COLUMNIZE))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_COLUMNIZE, _("&Columnize..."), _("Reformat selected lines in columns..."));
        add_sep = true;
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateInsertMenu(wxMenu *menu_) const
{
    // all of these modify the document
    if (HasMenuOptionType(STE_MENU_READONLY))
        return menu_;

    wxMenu *menu = menu_ ? menu_ : new wxMenu();

    if (HasMenuItemType(STE_MENU_INSERT_MENU, STE_MENU_INSERT_TEXT))
    {
        menu->Append(ID_STE_INSERT_TEXT, _("I&nsert text..."), _("Prepend, Append, or insert text at column..."));
    }

    if (HasMenuItemType(STE_MENU_INSERT_MENU, STE_MENU_INSERT_DATETIME))
    {
        menu->Append(ID_STE_INSERT_DATETIME, _("Insert &date and time"), _("Insert date and time"));
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateViewMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();
    bool add_sep = false;
    wxMenu* submenu;

    if (HasMenuItemType(STE_MENU_VIEW_MENU, STE_MENU_VIEW_WRAP))
    {
        menu->AppendCheckItem(ID_STE_PREF_WRAP_MODE, _("&Wrap text to window"), _("Wrap the text to fit inside window"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_VIEW_MENU, STE_MENU_VIEW_GUI))
    {
        if (add_sep) menu->AppendSeparator();

        menu->AppendCheckItem(ID_STE_VIEW_NONPRINT,        _("&Nonprinting Characters"), _("Show end of line symbols and whitespace"));
        menu->AppendCheckItem(ID_STE_PREF_VIEW_EOL,        _("&EOL"), _("Show end of line symbols"));
        menu->AppendCheckItem(ID_STE_PREF_VIEW_WHITESPACE, _("Whi&tespace"), _("Show whitespace using symbols"));

        menu->AppendSeparator();

        submenu = new wxMenu();
        submenu->AppendCheckItem(ID_STE_PREF_INDENT_GUIDES,   _("Show indent &guides"), _("Show indentation column guides"));
        submenu->AppendCheckItem(ID_STE_PREF_EDGE_MODE,       _("Show l&ong line guide"), _("Show column guide for long lines"));
        submenu->Append(ID_STE_PREF_EDGE_COLUMN,              _("Set long l&ine guide column..."), _("Set column long line guide..."));
        menu->Append(ID_STE_MENU_GUIDES, _("&Guides"), submenu);

        menu->AppendSeparator();

        submenu = new wxMenu();
        submenu->AppendCheckItem(ID_STE_PREF_VIEW_LINEMARGIN, _("Show &line number margin"), _("Show line number margin"));
        submenu->AppendCheckItem(ID_STE_PREF_VIEW_MARKERMARGIN, _("Show &marker margin"), _("Show a margin for markers"));
        submenu->AppendCheckItem(ID_STE_PREF_VIEW_FOLDMARGIN, _("Show &folding margin"), _("Show code folding margin"));
        menu->Append(ID_STE_MENU_MARGINS, _("&Margins"), submenu);
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_VIEW_MENU, STE_MENU_VIEW_FOLD))
    {
        if (add_sep) menu->AppendSeparator();

        submenu = new wxMenu();
        submenu->Append(ID_STE_FOLDS_TOGGLE_CURRENT, _("To&ggle current fold"), _("Toggle the current fold level"));
        submenu->Append(ID_STE_FOLDS_COLLAPSE_LEVEL, _("&Collapse folds below level..."), _("Collapse all folds below the level in document"));
        submenu->Append(ID_STE_FOLDS_EXPAND_LEVEL,   _("E&xpand folds above level..."), _("Expand all folds above the level in document"));
        submenu->Append(ID_STE_FOLDS_COLLAPSE_ALL,   _("&Collapse all folds"), _("Collapse all folds in document"));
        submenu->Append(ID_STE_FOLDS_EXPAND_ALL,     _("E&xpand all folds"), _("Expand all folds in document"));
        menu->Append(ID_STE_MENU_FOLDING, _("&Folding"), submenu);
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_VIEW_MENU, STE_MENU_VIEW_HILIGHT))
    {
        if (add_sep) menu->AppendSeparator();

        menu->AppendCheckItem(ID_STE_PREF_HIGHLIGHT_SYNTAX, _("S&yntax coloring"), _("Hilight document based on the syntax"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_VIEW_MENU, STE_MENU_VIEW_ZOOM))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_PREF_ZOOM, _("&Scale font size..."), _("Increase or decrease the size of the text"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_VIEW_MENU, STE_MENU_VIEW_FULLSCREEN) && HasMenuOptionType(STE_MENU_FRAME))
    {
        if (add_sep) menu->AppendSeparator();

        menu->AppendCheckItem(ID_STE_SHOW_FULLSCREEN, _("Show f&ullscreen"), _("Show the editor fullscreen"));
        add_sep = true;
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateBookmarkMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_;

    if (HasMenuItemType(STE_MENU_BOOKMARK_MENU, STE_MENU_BOOKMARK_DEFAULT))
    {
        if (menu == NULL) menu = new wxMenu;
        menu->Append(MenuItem(menu, ID_STE_BOOKMARKS, _("&Bookmarks..."), _("View all bookmarks"), wxITEM_NORMAL, STE_ARTMENU(wxART_HELP_BOOK)));
        menu->AppendSeparator();
        menu->Append(MenuItem(menu, ID_STE_BOOKMARK_TOGGLE, _("&Toggle bookmark"), _("Toggle a bookmark on cursor line"), wxITEM_NORMAL, STE_ARTMENU(wxART_ADD_BOOKMARK)));
        menu->AppendSeparator();
        menu->Append(MenuItem(menu, ID_STE_BOOKMARK_FIRST,    _("&First bookmark"),    _("Goto first bookmark"), wxITEM_NORMAL, STE_ARTMENU(wxART_GO_UP)));
        menu->Append(MenuItem(menu, ID_STE_BOOKMARK_PREVIOUS, _("&Previous bookmark"), _("Goto previous bookmark"), wxITEM_NORMAL, STE_ARTMENU(wxART_GO_BACK)));
        menu->Append(MenuItem(menu, ID_STE_BOOKMARK_NEXT,     _("&Next bookmark"),     _("Goto next bookmark"), wxITEM_NORMAL, STE_ARTMENU(wxART_GO_FORWARD)));
        menu->Append(MenuItem(menu, ID_STE_BOOKMARK_LAST,     _("&Last bookmark"),     _("Goto last bookmark"), wxITEM_NORMAL, STE_ARTMENU(wxART_GO_DOWN)));
        menu->AppendSeparator();
        menu->Append(MenuItem(menu, ID_STE_BOOKMARK_CLEAR, _("&Clear all bookmarks"), _("Clear all bookmarks"), wxITEM_NORMAL, STE_ARTMENU(wxART_DEL_BOOKMARK)));
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreatePreferenceMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();
    bool add_sep = false;

    if (HasMenuItemType(STE_MENU_PREFS_MENU, STE_MENU_PREFS_DLG))
    {
        menu->Append(ID_STE_PREFERENCES, _("Show &preference dialog..."), _("Show preference dialog..."));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_PREFS_MENU, STE_MENU_PREFS_INDENT))
    {
        if (add_sep) menu->AppendSeparator();

        menu->AppendCheckItem(ID_STE_PREF_USE_TABS,        _("Use &tabs"),    _("Tab key inserts a tab character"));
        menu->AppendCheckItem(ID_STE_PREF_TAB_INDENTS,     _("Tab &indents"), _("Tab key indents"));
        menu->AppendCheckItem(ID_STE_PREF_BACKSPACE_UNINDENTS, _("&Backspace unindents"), _("Backspace key unindents"));
        menu->AppendCheckItem(ID_STE_PREF_AUTOINDENT,     _("&Auto indent"), _("Indent new lines to previous indentation"));

        menu->Append(ID_STE_PREF_TAB_WIDTH,    _("Set tab &width..."),    _("Set the number of spaces to show for tab character"));
        menu->Append(ID_STE_PREF_INDENT_WIDTH, _("Set indent wi&dth..."), _("Set the number of spaces to use for indentation"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_PREFS_MENU, STE_MENU_PREFS_EOL))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STE_PREF_EOL_MODE, _("&EOL Mode..."), _("Set the end of line mode"));
        add_sep = true;
    }
    if (HasMenuItemType(STE_MENU_PREFS_MENU, STE_MENU_PREFS_SAVE))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(MenuItem(menu, ID_STE_SAVE_PREFERENCES, _("Save preferences"), _("Save current preferences"), wxITEM_NORMAL, STE_ARTMENU(wxART_STEDIT_SAVE)));
        add_sep = true;
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateWindowMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();
    bool add_sep = false;

    if (HasMenuItemType(STE_MENU_WINDOW_MENU, STE_MENU_WINDOW_SPLIT))
    {
        menu = CreateSplitterPopupMenu(menu);
        add_sep = true;
    }

    if (HasMenuItemType(STE_MENU_WINDOW_MENU, STE_MENU_WINDOW_FILECHOOSER))
    {
        if (add_sep) menu->AppendSeparator();
        menu->AppendCheckItem(ID_STF_SHOW_SIDEBAR, _("&Show sidebar"), _("Show the sidebar panel"));
    }

    if (HasMenuItemType(STE_MENU_WINDOW_MENU, STE_MENU_WINDOW_PREVNEXT))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STN_WIN_PREVIOUS, _("Pr&evious page"), _("Goto previous page"));
        menu->Append(ID_STN_WIN_NEXT,     _("Ne&xt page"),     _("Goto next page"));
        add_sep = true;
    }

    if (HasMenuItemType(STE_MENU_WINDOW_MENU, STE_MENU_WINDOW_WINDOWS))
    {
        if (add_sep) menu->AppendSeparator();

        menu->Append(ID_STN_WINDOWS, _("&Windows..."), _("Manage opened windows"));
        add_sep = true;
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

wxMenu *wxSTEditorMenuManager::CreateHelpMenu(wxMenu *menu_) const
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();

    if (HasMenuOptionType(STE_MENU_FRAME) && HasMenuItemType(STE_MENU_HELP_MENU, STE_MENU_HELP_ABOUT))
    {
        wxMenuItem* item = new wxMenuItem(menu, wxID_ABOUT, wxGetStockLabelEx(wxID_ABOUT), _("About this program"));
        item->SetBitmap(STE_ARTMENU(wxART_STEDIT_APP));
        menu->Append(item);
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

// static
wxMenu* wxSTEditorMenuManager::CreateInsertCharsMenu(wxMenu *menu_, int types)
{
    wxMenu *menu = menu_ ? menu_ : new wxMenu();

    if (STE_HASBIT(types, STE_MENU_INSERTCHARS_CHARS))
    {
        menu->Append(ID_STEDLG_INSERTMENU_TAB, _("Tab character"));
        menu->Append(ID_STEDLG_INSERTMENU_CR,  _("Carriage return"));
        menu->Append(ID_STEDLG_INSERTMENU_LF,  _("Line feed"));
    }
    if (STE_HASBIT(types, STE_MENU_INSERTCHARS_REGEXP))
    {
        wxMenu* reMenu = new wxMenu;
        reMenu->Append(ID_STEDLG_INSERTMENURE_ANYCHAR,   _("Any character"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_RANGE,     _("Character in range"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_NOTRANGE,  _("Character not in range"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_BEGINLINE, _("Beginning of line"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_ENDLINE,   _("End of line"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_TAGEXPR,   _("Tagged expression"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_0MATCHES,  _("0 or more matches"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_1MATCHES,  _("1 or more matches"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_01MATCHES, _("0 or 1 matches"));
        reMenu->AppendSeparator();
        reMenu->Append(ID_STEDLG_INSERTMENURE_ALPHANUM,  _("Alphanumeric characters"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_ALPHA,     _("Alphabetical characters"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_NUMERIC,   _("Numeric characters"));
        reMenu->Append(ID_STEDLG_INSERTMENURE_TAB,       _("Tab characters"));

        menu->Append(ID_STEDLG_MENU_INSERTMENURE, _("Regexp"), reMenu);
    }

    if (!menu_ && menu && (menu->GetMenuItemCount() == 0))
    {
        wxDELETE(menu);
    }

    return menu;
}

static bool AddAccelFromMenuItem(const wxMenu* menu, wxArrayPtrVoid& entries)
{
    if (!menu)
        return false;

    bool ret = false;
    const wxMenuItemList& itemList = menu->GetMenuItems();

    for ( wxMenuItemList::const_iterator it = itemList.begin();
          it != itemList.end();
          it++)
    {
        wxMenuItem *menuItem = *it;
        if (!menuItem)
            continue;

        if (menuItem->IsSubMenu())
            ret |= AddAccelFromMenuItem(menuItem->GetSubMenu(), entries);
        else
        {
            wxAcceleratorEntry *entry = menuItem->GetAccel();
            if (entry)
            {
                ret = true;
                // make sure the id is set correctly (for GTK at least)
                entry->Set(entry->GetFlags(), entry->GetKeyCode(), menuItem->GetId());
                bool exists = false;
                size_t n, count = entries.GetCount();
                for (n = 0; n < count; n++)
                {
                    if (*entry == *((wxAcceleratorEntry*)entries[n]))
                    {
                        exists = true;
                        break;
                    }
                }

                if (exists)
                    delete entry;
                else
                    entries.Add(entry);
            }
        }
    }

    return ret;
}

// static
bool wxSTEditorMenuManager::GetAcceleratorEntries(const wxMenu* menu,
                                                  const wxMenuBar* menuBar,
                                                  wxArrayPtrVoid& entries)
{
    bool ret = false;
    if (menu)
        ret = AddAccelFromMenuItem(menu, entries);
    if (menuBar)
    {
        size_t n, count = menuBar->GetMenuCount();
        for (n = 0; n < count; n++)
            ret |= AddAccelFromMenuItem(menuBar->GetMenu(n), entries);
    }

    return ret;
}

// static
wxAcceleratorTable wxSTEditorMenuManager::CreateAcceleratorTable(wxArrayPtrVoid& entries)
{
    if (entries.GetCount() == 0)
        return wxAcceleratorTable();

    return wxAcceleratorTable((int)entries.GetCount(),
                              (wxAcceleratorEntry*)&entries.Item(0));
}

// static
wxAcceleratorTable wxSTEditorMenuManager::CreateAcceleratorTable(const wxMenu* menu,
                                                                 const wxMenuBar* menuBar)
{
    wxArrayPtrVoid entries;
    GetAcceleratorEntries(menu, menuBar, entries);
    wxAcceleratorTable table(CreateAcceleratorTable(entries));
    while ( entries.GetCount() > 0 )
    {
        wxAcceleratorEntry *item = (wxAcceleratorEntry*)entries[0];
        //wxPrintf(wxT("Accel %d, '%c' %d %d\n"), item->GetFlags(), (wxChar)item->GetKeyCode(), item->GetKeyCode(), item->GetCommand()); fflush(stdout);
        delete item;
        entries.RemoveAt(0);
    }
    return table;
}

void wxSTEditorMenuManager::EnableEditorItems(bool enable, wxMenu *menu,
                                              wxMenuBar *menuBar, wxToolBar *toolBar)
{
    m_enabledEditorItems = enable;

    int n, count;
    for (n = ID_STE_PREF__FIRST; n <= ID_STE_PREF__LAST; n++)
        DoEnableItem(menu, menuBar, toolBar, n, enable);

    count = (int)m_enableItemsArray.GetCount();
    for (n = 0; n < count; n++)
        DoEnableItem(menu, menuBar, toolBar, m_enableItemsArray[n], enable);

    const int menuIds[] = {
        wxID_SAVE,
        wxID_SAVEAS,
        ID_STN_SAVE_ALL,
        ID_STN_CLOSE_PAGE,
        ID_STN_CLOSE_ALL,
        ID_STN_CLOSE_ALL_OTHERS,
        ID_STE_PROPERTIES,
        wxID_PRINT,
        wxID_PREVIEW,
        wxID_PRINT_SETUP,
        ID_STE_PRINT_PAGE_SETUP,
        ID_STE_PRINT_OPTIONS,

        wxID_CUT,
        wxID_COPY,
        ID_STE_COPY_PRIMARY,
        wxID_PASTE,
        ID_STE_PASTE_RECT,
        wxID_SELECTALL,
        wxID_FIND,
        ID_STE_FIND_NEXT,
        ID_STE_FIND_DOWN,
        wxID_REPLACE,
        ID_STE_GOTO_LINE,
        wxID_UNDO,
        wxID_REDO,

        ID_STE_UPPERCASE,
        ID_STE_INCREASE_INDENT,
        ID_STE_DECREASE_INDENT,
        ID_STE_LINES_JOIN,
        ID_STE_LINES_SPLIT,
        ID_STE_TABS_TO_SPACES,
        ID_STE_SPACES_TO_TABS,
        ID_STE_CONVERT_EOL,
        ID_STE_TRAILING_WHITESPACE,
        ID_STE_REMOVE_CHARSAROUND,

        ID_STE_FOLDS_TOGGLE_CURRENT,
        ID_STE_FOLDS_COLLAPSE_LEVEL,
        ID_STE_FOLDS_EXPAND_LEVEL,
        ID_STE_FOLDS_COLLAPSE_ALL,
        ID_STE_FOLDS_EXPAND_ALL,

        ID_STE_BOOKMARK_TOGGLE,
        ID_STE_BOOKMARK_FIRST,
        ID_STE_BOOKMARK_PREVIOUS,
        ID_STE_BOOKMARK_NEXT,
        ID_STE_BOOKMARK_LAST,
        ID_STE_BOOKMARK_CLEAR,

        ID_STE_PREFERENCES,
        ID_STE_SAVE_PREFERENCES,

        ID_STS_UNSPLIT,
        ID_STS_SPLIT_HORIZ,
        ID_STS_SPLIT_VERT
    };

    count = WXSIZEOF(menuIds);
    for (n = 0; n < count; n++)
        DoEnableItem(menu, menuBar, toolBar, menuIds[n], enable);
}

// static
wxMenuItem *wxSTEditorMenuManager::MenuItem(wxMenu *menu, wxWindowID win_id,
                                     const wxString &text, const wxString &help,
                                     wxItemKind kind, const wxBitmap &bitmap)
{
    wxMenuItem *item = new wxMenuItem(menu, win_id, text, help, kind);
    if (bitmap.IsOk())
        item->SetBitmap(bitmap);
    return item;
}

// static
void wxSTEditorMenuManager::DestroyMenuItem(wxMenu *menu, int menu_id, bool clean_sep)
{
    wxCHECK_RET(menu, wxT("Invalid menu"));
    wxMenuItem *lastItem = menu->FindItem(menu_id);
    if (lastItem)
        menu->Destroy(lastItem);

    if (!clean_sep) return;

    // find any separators that are next to each other and delete them
    wxMenuItemList &menuItems = menu->GetMenuItems();
    wxMenuItemList::compatibility_iterator node = menuItems.GetFirst();

    // delete leading separator
    if (node && wxStaticCast(node->GetData(), wxMenuItem)->IsSeparator())
    {
        menu->Destroy(wxStaticCast(node->GetData(), wxMenuItem));
        node = node->GetNext();
    }

    // delete duplicate separators
    for (;
         node;
         node = node->GetNext())
    {
        wxMenuItem* item = wxStaticCast(node->GetData(), wxMenuItem);
        if (lastItem && lastItem->IsSeparator() && item->IsSeparator())
            menu->Destroy(lastItem);

        lastItem = item;
    }

    // delete trailing separator too
    node = menuItems.GetLast();
    if (node && wxStaticCast(node->GetData(), wxMenuItem)->IsSeparator())
    {
        menu->Destroy(wxStaticCast(node->GetData(), wxMenuItem));
    }
}

bool wxSTEditorMenuManager::DoEnableItem(wxMenu *menu, wxMenuBar *menuBar,
                               wxToolBar *toolBar, wxWindowID menu_id, bool val)
{
    //wxPrintf(wxT("DoEnableItem %d - val %d\n"), menu_id, int(val));
    bool ret = false;

    if (menu)
    {
        wxMenuItem *menuItem = menu->FindItem(menu_id);
        if (menuItem)
        {
            menuItem->Enable(val);
            ret = true;
        }
    }
    if (menuBar)
    {
        wxMenuItem *menuItem = menuBar->FindItem(menu_id);
        if (menuItem)
        {
            menuItem->Enable(val);
            ret = true;
        }
    }
    if (toolBar)
    {
        toolBar->EnableTool(menu_id, val);
        ret = true; // don't know if it exists, pretend that it did...
    }

    return ret;
}
bool wxSTEditorMenuManager::DoCheckItem(wxMenu *menu, wxMenuBar *menuBar,
                              wxToolBar *toolBar, wxWindowID menu_id, bool val)
{
    //wxPrintf(wxT("DoCheckItem %d - val %d\n"), menu_id, int(val));
    bool ret = false;

    if (menu)
    {
        wxMenuItem *menuItem = menu->FindItem(menu_id);
        if (menuItem)
        {
            menuItem->Check(val);
            ret = true;
        }
    }
    if (menuBar)
    {
        wxMenuItem *menuItem = menuBar->FindItem(menu_id);
        if (menuItem)
        {
            menuItem->Check(val);
            ret = true;
        }
    }
    if (toolBar)
    {
        toolBar->ToggleTool(menu_id, val);
        ret = true; // don't know if it exists, pretend that it did...
    }

    return ret;
}
bool wxSTEditorMenuManager::DoSetTextItem(wxMenu *menu, wxMenuBar *menuBar,
                                        wxWindowID menu_id, const wxString &val)
{
    bool ret = false;

    if (menu)
    {
        wxMenuItem *menuItem = menu->FindItem(menu_id);
        if (menuItem)
        {
            menuItem->SetItemLabel(val);
            ret = true;
        }
    }
    if (menuBar)
    {
        wxMenuItem *menuItem = menuBar->FindItem(menu_id);
        if (menuItem)
        {
            menuItem->SetItemLabel(val);
            ret = true;
        }
    }

    return ret;
}
