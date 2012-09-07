///////////////////////////////////////////////////////////////////////////////
// Name:        steframe.cpp
// Purpose:     wxSTEditorFrame
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// RCS-ID:
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "wx/stedit/stedit.h"
#include "wx/stedit/steframe.h"
#include "wx/stedit/steart.h"
#include "wx/stedit/stetree.h"

#include "wxext.h"

#include <wx/srchctrl.h>
#include <wx/dirctrl.h>

//-----------------------------------------------------------------------------
// wxSTEditorFrame
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSTEditorFrame, wxFrame)

BEGIN_EVENT_TABLE(wxSTEditorFrame, wxFrame)
    EVT_MENU_OPEN               (wxSTEditorFrame::OnMenuOpen)
    EVT_MENU                    (wxID_ANY, wxSTEditorFrame::OnMenu)
    EVT_SEARCHCTRL_SEARCH_BTN   (ID_STE_TOOLBAR_FIND_CTRL, wxSTEditorFrame::OnMenu) // wxCommandEvent so we can treat it like a menu
    EVT_TEXT_ENTER              (ID_STE_TOOLBAR_FIND_CTRL, wxSTEditorFrame::OnMenu) // wxCommandEvent so we can treat it like a menu

    //EVT_STEDITOR_CREATED      (wxID_ANY, wxSTEditorFrame::OnSTECreated)
    EVT_STEDITOR_STATE_CHANGED  (wxID_ANY, wxSTEditorFrame::OnSTEState)
    EVT_STC_UPDATEUI            (wxID_ANY, wxSTEditorFrame::OnSTCUpdateUI)
    EVT_STEDITOR_POPUPMENU      (wxID_ANY, wxSTEditorFrame::OnSTEPopupMenu)

    EVT_STNOTEBOOK_PAGE_CHANGED (wxID_ANY, wxSTEditorFrame::OnNotebookPageChanged)

    EVT_STEFIND_RESULTS_NEED_SHOWN(wxID_ANY, wxSTEditorFrame::OnFindAllResults)

    EVT_TREE_ITEM_ACTIVATED     (wxID_ANY, wxSTEditorFrame::OnDirCtrlItemActivation)

    EVT_CLOSE                   (wxSTEditorFrame::OnClose)
END_EVENT_TABLE()

void wxSTEditorFrame::Init()
{
    m_sideSplitter     = NULL;
    m_sideSplitterWin1 = NULL;
    m_sideSplitterWin2 = NULL;
    m_sideSplitter_pos = 200;

    m_sideNotebook     = NULL;
    m_steTreeCtrl      = NULL;
    m_dirCtrl          = NULL;

    m_mainSplitter     = NULL;
    m_mainSplitterWin1 = NULL;
    m_mainSplitterWin1 = NULL;

    m_steNotebook      = NULL;
    m_steSplitter      = NULL;

    m_resultsNotebook  = NULL;
    m_findResultsEditor= NULL;
}

bool wxSTEditorFrame::Create(wxWindow *parent, wxWindowID id,
                             const wxString& title,
                             const wxPoint& pos, const wxSize& size,
                             long  style,
                             const wxString& name)
{
    m_titleBase = title;

    if (!wxFrame::Create(parent, id, title, pos, size, style, name))
        return false;

    // Set the frame's icons
    SetIcons(wxSTEditorArtProvider::GetDialogIconBundle());

    ::wxFrame_SetInitialPosition(this, pos, size);

    return true;
}

wxSTEditorFrame::~wxSTEditorFrame()
{
    SetSendSTEEvents(false);
    if (GetToolBar() && (GetToolBar() == GetOptions().GetToolBar())) // remove for safety
        GetOptions().SetToolBar(NULL);
    if (GetMenuBar() && (GetMenuBar() == GetOptions().GetMenuBar())) // remove for file history
        GetOptions().SetMenuBar(NULL);
    if (GetStatusBar() && (GetStatusBar() == GetOptions().GetStatusBar()))
        GetOptions().SetStatusBar(NULL);

    // This stuff always gets saved when the frame closes
    wxConfigBase *config = GetConfigBase();
    if (config && GetOptions().HasConfigOption(STE_CONFIG_FILEHISTORY))
        GetOptions().SaveFileConfig(*config);

    if (config && GetOptions().HasConfigOption(STE_CONFIG_FINDREPLACE) &&
        GetOptions().GetFindReplaceData())
        GetOptions().GetFindReplaceData()->SaveConfig(*config,
                      GetOptions().GetConfigPath(STE_OPTION_CFGPATH_FINDREPLACE));
}

bool wxSTEditorFrame::Destroy()
{
    SetSendSTEEvents(false);
    if (GetToolBar() && (GetToolBar() == GetOptions().GetToolBar())) // remove for safety
        GetOptions().SetToolBar(NULL);
    if (GetMenuBar() && (GetMenuBar() == GetOptions().GetMenuBar())) // remove for file history
        GetOptions().SetMenuBar(NULL);
    if (GetStatusBar() && (GetStatusBar() == GetOptions().GetStatusBar()))
        GetOptions().SetStatusBar(NULL);

    return wxFrame::Destroy();
}

void wxSTEditorFrame::CreateOptions( const wxSTEditorOptions& options )
{
    m_options = options;

    wxConfigBase *config = GetConfigBase();
    wxSTEditorMenuManager *steMM = GetOptions().GetMenuManager();

    if (steMM && GetOptions().HasFrameOption(STF_CREATE_MENUBAR))
    {
        wxMenuBar *menuBar = GetMenuBar();

        if (!menuBar)
            menuBar = new wxMenuBar(wxMB_DOCKABLE);

        steMM->CreateMenuBar(menuBar, true);

        SetMenuBar(menuBar);
        wxAcceleratorHelper::SetAcceleratorTable(this, *steMM->GetAcceleratorArray());
        wxAcceleratorHelper::SetAccelText(menuBar, *steMM->GetAcceleratorArray());

        if (GetOptions().HasFrameOption(STF_CREATE_FILEHISTORY) && !GetOptions().GetFileHistory())
        {
            // If there is wxID_OPEN then we can use wxFileHistory to save them
            wxMenu* menu = NULL;
            wxMenuItem* item = menuBar->FindItem(wxID_OPEN, &menu);

            if (menu && item)
            {
                int open_index = menu->GetMenuItems().IndexOf(item);

                if (open_index != wxNOT_FOUND)
                {
                    wxMenu* submenu = new wxMenu();
                    menu->Insert(open_index + 1, wxID_ANY, _("Open &Recent"), submenu);
                    GetOptions().SetFileHistory(new wxFileHistory(9), false);
                    GetOptions().GetFileHistory()->UseMenu(submenu);
                    if (config)
                    {
                        GetOptions().LoadFileConfig(*config);
                    }
                }
            }

            GetOptions().SetMenuBar(menuBar);
        }
    }
    if (steMM && GetOptions().HasFrameOption(STF_CREATE_TOOLBAR))
    {
        wxToolBar* toolBar = (GetToolBar() != NULL) ? GetToolBar() : CreateToolBar();
        steMM->CreateToolBar(toolBar);
        GetOptions().SetToolBar(toolBar);
    }
    if ((GetStatusBar() == NULL) && GetOptions().HasFrameOption(STF_CREATE_STATUSBAR))
    {
        CreateStatusBar(1);
        GetOptions().SetStatusBar(GetStatusBar());
    }
    if (steMM)
    {
        if (GetOptions().HasEditorOption(STE_CREATE_POPUPMENU))
        {
            wxMenu* menu = steMM->CreateEditorPopupMenu();

            wxAcceleratorHelper::SetAccelText(menu, *steMM->GetAcceleratorArray());
            GetOptions().SetEditorPopupMenu(menu, false);
        }
        if (GetOptions().HasSplitterOption(STS_CREATE_POPUPMENU))
            GetOptions().SetSplitterPopupMenu(steMM->CreateSplitterPopupMenu(), false);
        if (GetOptions().HasNotebookOption(STN_CREATE_POPUPMENU))
            GetOptions().SetNotebookPopupMenu(steMM->CreateNotebookPopupMenu(), false);
    }

    if (!m_sideSplitter && GetOptions().HasFrameOption(STF_CREATE_SIDEBAR))
    {
        m_sideSplitter = new wxSplitterWindow(this, ID_STF_SIDE_SPLITTER);
        m_sideSplitter->SetMinimumPaneSize(10);
        m_sideNotebook = new wxNotebook(m_sideSplitter, ID_STF_SIDE_NOTEBOOK);
        m_steTreeCtrl  = new wxSTEditorTreeCtrl(m_sideNotebook, ID_STF_FILE_TREECTRL);
        m_dirCtrl      = new wxGenericDirCtrl(m_sideNotebook, ID_STF_FILE_DIRCTRL,
                                              wxFileName::GetCwd(),
                                              wxDefaultPosition, wxDefaultSize,
                                              wxDIRCTRL_3D_INTERNAL
#if wxCHECK_VERSION(2, 9, 2)
                                              |(GetOptions().HasFrameOption(STF_CREATE_NOTEBOOK) ? wxDIRCTRL_MULTIPLE : 0)
#endif // wxCHECK_VERSION(2, 9, 2)
                                              );

        m_sideNotebook->AddPage(m_steTreeCtrl, _("Files"));
        m_sideNotebook->AddPage(m_dirCtrl,     _("Open"));

        m_sideSplitterWin1 = m_sideNotebook;
    }

    if (!m_steNotebook && GetOptions().HasFrameOption(STF_CREATE_NOTEBOOK))
    {
        m_mainSplitter = new wxSplitterWindow(m_sideSplitter ? (wxWindow*)m_sideSplitter : (wxWindow*)this, ID_STF_MAIN_SPLITTER);
        m_mainSplitter->SetMinimumPaneSize(1);

        m_steNotebook = new wxSTEditorNotebook(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                               wxCLIP_CHILDREN);
        m_steNotebook->CreateOptions(m_options);
        (void)m_steNotebook->InsertEditorSplitter(-1, wxID_ANY, GetOptions().GetDefaultFileName(), true);
        // update after adding a single page
        m_steNotebook->UpdateAllItems();
        m_mainSplitter->Initialize(m_steNotebook);
        m_mainSplitterWin1 = m_steNotebook;
        m_sideSplitterWin2 = m_mainSplitter;

        if (m_steTreeCtrl)
            m_steTreeCtrl->SetSTENotebook(m_steNotebook);
    }
    else if (!m_steSplitter && GetOptions().HasFrameOption(STF_CREATE_SINGLEPAGE))
    {
        m_mainSplitter = new wxSplitterWindow(m_sideSplitter ? (wxWindow*)m_sideSplitter : (wxWindow*)this, ID_STF_MAIN_SPLITTER);
        m_mainSplitter->SetMinimumPaneSize(1);

        m_steSplitter = new wxSTEditorSplitter(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
        m_steSplitter->CreateOptions(m_options);
        m_mainSplitter->Initialize(m_steSplitter);
        m_mainSplitterWin1 = m_steSplitter;
    }
    //else user will set up the rest

    if (m_mainSplitter && m_mainSplitterWin1 && !m_resultsNotebook && GetOptions().HasFrameOption(STF_CREATE_RESULT_NOTEBOOK))
    {
        m_resultsNotebook = new wxNotebook(m_mainSplitter, wxID_ANY);

        m_findResultsEditor = new wxSTEditorFindResultsEditor(m_resultsNotebook, wxID_ANY);
        m_findResultsEditor->CreateOptionsFromEditorOptions(options);
        m_resultsNotebook->AddPage(m_findResultsEditor, _("Search Results"));

        wxSTEditorFindReplacePanel::SetFindResultsEditor(m_findResultsEditor);
        m_mainSplitter->SplitHorizontally(m_mainSplitterWin1, m_resultsNotebook, GetClientSize().GetHeight()*2/3);
        m_mainSplitterWin2 = m_resultsNotebook;
    }

    if (GetOptions().HasFrameOption(STF_CREATE_SIDEBAR) && GetSideSplitter() && m_sideSplitterWin1 && m_sideSplitterWin2)
    {
        GetSideSplitter()->SplitVertically(m_sideSplitterWin1, m_sideSplitterWin2, m_sideSplitter_pos);
    }

#if wxUSE_DRAG_AND_DROP
    if (GetOptions().HasFrameOption(STF_DO_DRAG_AND_DROP))
    {
        SetDropTarget(new wxSTEditorFileDropTarget(this));
    }
#endif //wxUSE_DRAG_AND_DROP

    if (GetOptions().HasConfigOption(STE_CONFIG_FINDREPLACE) && config)
    {
        if (GetOptions().GetFindReplaceData() &&
            !GetOptions().GetFindReplaceData()->HasLoadedConfig())
            GetOptions().GetFindReplaceData()->LoadConfig(*config);
    }

    if (config)
        LoadConfig(*config);

    // The config may change the frame size so relayout the splitters
    if (m_mainSplitter && m_mainSplitter->IsSplit()) //m_mainSplitterWin1 && m_resultsNotebook)
        m_mainSplitter->SetSashPosition(GetClientSize().GetHeight()*2/3);

    UpdateAllItems();

    // if we've got an editor let it update gui
    wxSTEditor *editor = GetEditor();
    if (editor)
        editor->UpdateAllItems();
}

// --------------------------------------------------------------------------

void wxSTEditorFrame::SetSendSTEEvents(bool send)
{
    if      (GetEditorNotebook()) GetEditorNotebook()->SetSendSTEEvents(send);
    else if (GetEditorSplitter()) GetEditorSplitter()->SetSendSTEEvents(send);
    else if (GetEditor())         GetEditor()->SetSendSTEEvents(send);
}

// --------------------------------------------------------------------------

wxSTEditor *wxSTEditorFrame::GetEditor(int page) const
{
    wxSTEditorSplitter *splitter = GetEditorSplitter(page);
    return splitter ? splitter->GetEditor() : (wxSTEditor*)NULL;
}

wxSTEditorSplitter *wxSTEditorFrame::GetEditorSplitter(int page) const
{
    return GetEditorNotebook() ? GetEditorNotebook()->GetEditorSplitter(page) : m_steSplitter;
}

void wxSTEditorFrame::ShowSidebar(bool show_left_side)
{
    wxSplitterWindow* sideSplitter = GetSideSplitter();

    if (sideSplitter && m_sideSplitterWin1 && m_sideSplitterWin2)
    {
        if (show_left_side)
        {
            if (!sideSplitter->IsSplit())
            {
                // If they want it shown, make it large enough to be vagely useful
                // but never wider than the window itself.
                int win_width = sideSplitter->GetSize().GetWidth();
                int sash_pos  = wxMax(m_sideSplitter_pos, 100);
                sash_pos      = wxMin(m_sideSplitter_pos, int(0.8*win_width));
                sideSplitter->SplitVertically(m_sideSplitterWin1, m_sideSplitterWin2, sash_pos);
                GetSideNotebook()->Show();
            }
        }
        else if (sideSplitter->IsSplit())
        {
            m_sideSplitter_pos = sideSplitter->GetSashPosition();
            sideSplitter->Unsplit(m_sideSplitterWin1);
        }
        UpdateAllItems();
    }
}

// --------------------------------------------------------------------------

bool wxSTEditorFrame::LoadFile(const wxFileName& fileName, bool show_error_dialog_on_error)
{
    bool ok;

    if (GetEditorNotebook())
    {
        ok = GetEditorNotebook()->LoadFile(fileName);
    }
    else if (GetEditor())
    {
        ok = GetEditor()->LoadFile(fileName);
    }
    else
    {
        ok = false;
    }

    if (show_error_dialog_on_error && !ok)
    {
        wxMessageBox(wxString::Format(_("Error opening file: '%s'"),
                     fileName.GetFullPath(GetOptions().GetDisplayPathSeparator()).wx_str()),
                     STE_APPDISPLAYNAME, wxOK|wxICON_ERROR , this);
    }

    return ok;
}

void wxSTEditorFrame::UpdateAllItems()
{
    UpdateItems(GetOptions().GetEditorPopupMenu(), GetOptions().GetMenuBar(),
                                                   GetOptions().GetToolBar());
    UpdateItems(GetOptions().GetNotebookPopupMenu());
    UpdateItems(GetOptions().GetSplitterPopupMenu());
}
void wxSTEditorFrame::UpdateItems(wxMenu *menu, wxMenuBar *menuBar, wxToolBar *toolBar)
{
    if (!menu && !menuBar && !toolBar) return;

    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STF_SHOW_SIDEBAR, GetSideSplitter() != NULL);
    STE_MM::DoCheckItem(menu, menuBar, toolBar, ID_STF_SHOW_SIDEBAR, (GetSideSplitter() != NULL) && GetSideSplitter()->IsSplit());
}

// --------------------------------------------------------------------------

wxConfigBase* wxSTEditorFrame::GetConfigBase()
{
    return wxConfigBase::Get(false);
}

void wxSTEditorFrame::LoadConfig(wxConfigBase &config, const wxString &configPath_)
{
    wxString configPath = wxSTEditorOptions::FixConfigPath(configPath_, false);

    if (GetMenuBar() && GetMenuBar()->FindItem(ID_STF_SHOW_SIDEBAR))
    {
        long val = 0;
        if (config.Read(configPath + wxT("/ShowSidebar"), &val))
        {
            wxSTEditorMenuManager::DoCheckItem(NULL, GetMenuBar(), NULL,
                                               ID_STF_SHOW_SIDEBAR, val != 0);
            // send fake event to HandleEvent
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_STF_SHOW_SIDEBAR);
            evt.SetInt(int(val));
            HandleMenuEvent(evt);
        }
    }

    wxString str;
    if (config.Read(configPath + wxT("/FrameSize"), &str))
    {
        wxRect rect = GetRect();
        long lrect[4] = { rect.x, rect.y, rect.width, rect.height };
        wxArrayString arrStr = wxStringTokenize(str, wxT(","));
        if (arrStr.GetCount() == 4u)
        {
            for (size_t n = 0; n < 4; n++)
                arrStr[n].ToLong(&lrect[n]);

            wxRect cfgRect((int)lrect[0], (int)lrect[1], (int)lrect[2], (int)lrect[3]);
            cfgRect = cfgRect.Intersect(wxGetClientDisplayRect());

            if ((cfgRect != rect) && (cfgRect.width>=100) && (cfgRect.height>=100))
                SetSize(cfgRect);
        }
    }
}

void wxSTEditorFrame::SaveConfig(wxConfigBase &config, const wxString &configPath_)
{
    wxString configPath = wxSTEditorOptions::FixConfigPath(configPath_, false);
    if (GetMenuBar() && GetMenuBar()->FindItem(ID_STF_SHOW_SIDEBAR))
    {
        wxString val = GetMenuBar()->IsChecked(ID_STF_SHOW_SIDEBAR) ? wxT("1") : wxT("0");
        config.Write(configPath + wxT("/ShowSidebar"), val);
    }

    wxRect rect = GetRect();
    if ((rect.x>=0) && (rect.y>=0) && (rect.width>=100) && (rect.height>=100))
       config.Write(configPath + wxT("/FrameSize"), wxString::Format(wxT("%d,%d,%d,%d"), rect.x, rect.y, rect.width, rect.height));
}

void wxSTEditorFrame::OnNotebookPageChanged(wxNotebookEvent &WXUNUSED(event))
{
    wxSTEditor *editor = GetEditor();
    wxString title;
    wxSTEditorMenuManager *steMM = GetOptions().GetMenuManager();

    if (editor)
    {
        title = MakeTitle(editor);

        if ( steMM && !steMM->HasEnabledEditorItems())
            steMM->EnableEditorItems(true, NULL, GetMenuBar(), GetToolBar());
    }
    else
    {
        title = m_titleBase;

        if (steMM && steMM->HasEnabledEditorItems())
            steMM->EnableEditorItems(false, NULL, GetMenuBar(), GetToolBar());
    }

    SetTitle(title);
}

void wxSTEditorFrame::OnFindAllResults(wxCommandEvent& )
{
    // nothing to do
    if (!m_findResultsEditor)
        return;

    // try to select the page in the results notebook
    if (m_resultsNotebook)
    {
        size_t n, count = m_resultsNotebook->GetPageCount();

        for (n = 0; n < count; ++n)
        {
            if (m_resultsNotebook->GetPage(n) == m_findResultsEditor)
            {
                m_resultsNotebook->SetSelection(n);
                break;
            }
        }
    }

    // check that the results editor is in the main splitter
    bool is_in_mainsplitter = false;

    wxWindow* parent = m_findResultsEditor->GetParent();
    while (parent)
    {
        if (parent == m_mainSplitter)
        {
            is_in_mainsplitter = true;
            break;
        }

        parent = parent->GetParent();
    }

    // show the find results in the splitter
    if (is_in_mainsplitter && m_mainSplitter && m_mainSplitterWin1 && m_mainSplitterWin2)
    {
        int split_win_height = m_mainSplitter->GetClientSize().GetHeight();

        if (!m_mainSplitter->IsSplit())
        {
            m_mainSplitter->SplitHorizontally(m_mainSplitterWin1, m_mainSplitterWin2, split_win_height*2/3);
        }
        else if (m_mainSplitterWin2->GetSize().GetHeight() < 59)
        {
            m_mainSplitter->SetSashPosition(wxMax(split_win_height/2, 100));
        }
    }
}

void wxSTEditorFrame::OnDirCtrlItemActivation(wxTreeEvent &WXUNUSED(event))
{
    if (!m_dirCtrl) return;

    wxArrayString files;

    if (m_dirCtrl->GetTreeCtrl()->HasFlag(wxTR_MULTIPLE))
    {
        // We won't reach here in 2.8 since wxDIRCTRL_MULTIPLE doesn't exist
        #if wxCHECK_VERSION(2, 9, 2)
            // Avoid assert in GTK for calling wxTreeCtrl::GetSelection() on multiple selection treectrl
            m_dirCtrl->GetFilePaths(files);
        #endif
    }
    else
    {
        wxString filePath = m_dirCtrl->GetFilePath();
        if (!filePath.IsEmpty())
            files.Add(filePath);
    }

    if (files.IsEmpty())
        return;

    if (GetEditorNotebook())
    {
        GetEditorNotebook()->LoadFiles(&files, wxEmptyString);
    }
    else
        LoadFile(files[0], true); // just load the first one
}

void wxSTEditorFrame::OnSTECreated(wxCommandEvent &event)
{
    event.Skip();
    if (m_steTreeCtrl != NULL)
        m_steTreeCtrl->UpdateFromNotebook();
}

void wxSTEditorFrame::OnSTEPopupMenu(wxSTEditorEvent &event)
{
    event.Skip();
    wxSTEditor *editor = event.GetEditor();
    UpdateItems(editor->GetOptions().GetEditorPopupMenu());
}

wxString wxSTEditorFrame::MakeTitle(const wxSTEditor* editor) const
{
    wxFileName fileName = editor ? editor->GetFileName() : wxFileName();

    wxString title(fileName.GetFullPath(GetOptions().GetDisplayPathSeparator()));
    if (editor->IsModified()) title += wxMODIFIED_ASTERISK;
    title += wxT(" - ") + m_titleBase;

    return title;
}

void wxSTEditorFrame::OnSTEState(wxSTEditorEvent &event)
{
    event.Skip();
    wxSTEditor *editor = event.GetEditor();

    if ( event.HasStateChange(STE_FILENAME | STE_MODIFIED | STE_EDITABLE) )
    {
        if (wxDynamicCast(editor, wxSTEditorFindResultsEditor) == NULL)
        {
            wxString title = MakeTitle(editor);
            if (GetTitle() != title)
                SetTitle(title);
        }

        if (event.HasStateChange(STE_FILENAME) && GetOptions().GetFileHistory())
        {
            if (wxFileExists(event.GetString()))
                GetOptions().GetFileHistory()->AddFileToHistory( event.GetString() );
        }
    }
}

void wxSTEditorFrame::OnSTCUpdateUI(wxStyledTextEvent &event)
{
    event.Skip();
    if (!GetStatusBar()) // nothing to do
        return;

    wxStyledTextCtrl* editor = wxStaticCast(event.GetEventObject(), wxStyledTextCtrl);
    STE_TextPos pos = editor->GetCurrentPos();
    int line        = editor->GetCurrentLine() + 1; // start at 1
    int lines       = editor->GetLineCount();
    int col         = editor->GetColumn(pos) + 1;   // start at 1
    int chars       = editor->GetLength();

    wxString txt = wxString::Format(wxT("Line %6d of %6d, Col %4d, Chars %6d  "), line, lines, col, chars);
    txt += editor->GetOvertype() ? wxT("[OVR]") : wxT("[INS]");

    if (txt != GetStatusBar()->GetStatusText()) // minor flicker reduction
        SetStatusText(txt, 0);
}

void wxSTEditorFrame::OnMenuOpen(wxMenuEvent &WXUNUSED(event))
{
    // might need to update the preferences, the rest should be ok though
    wxSTEditor* editor = NULL;
    wxWindow* win = wxWindow::FindFocus();

    // see if there's an editor that's a child of this and has the focus
    if (win != NULL)
    {
        editor = wxDynamicCast(win, wxSTEditor);
        if (editor)
        {
            wxWindow* parent = editor->GetParent();
            while (parent && (parent != this))
                parent = parent->GetParent();

            if (parent != this)
                editor = NULL;
        }
    }

    // just use the currently focused editor notebook
    if (editor == NULL)
        editor = GetEditor();

    if (editor && GetMenuBar())
        editor->UpdateItems(NULL, GetMenuBar());
}

void wxSTEditorFrame::OnMenu(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_OnMenu);
    if (guard.IsInside()) return;

    if (!HandleMenuEvent(event))
        event.Skip();
}

bool wxSTEditorFrame::HandleMenuEvent(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_HandleMenuEvent);
    if (guard.IsInside()) return false;

    int win_id  = event.GetId();

    // menu items that the frame handles before children
    switch (win_id)
    {
        case ID_STE_SAVE_PREFERENCES :
        {
            // we save everything the children do and more
            wxConfigBase *config = GetConfigBase();
            if (config)
            {
                SaveConfig(*config, GetOptions().GetConfigPath(STE_OPTION_CFGPATH_FRAME));
                GetOptions().SaveConfig(*config);
            }

            return true;
        }
    }

    wxWindow*           focusWin = FindFocus();
    wxSTEditor*         editor   = GetEditor();
    wxSTEditorNotebook* notebook = GetEditorNotebook();

    if (focusWin && wxDynamicCast(focusWin, wxSTEditorNotebook))
        notebook = wxDynamicCast(focusWin, wxSTEditorNotebook);
    else if (focusWin && wxDynamicCast(focusWin, wxSTEditor))
        editor = wxDynamicCast(focusWin, wxSTEditor);

    // Try the children to see if they'll handle the event first
    if (notebook && notebook->HandleMenuEvent(event))
        return true;

    if (editor)
    {
        if (wxDynamicCast(editor->GetParent(), wxSTEditorSplitter) &&
            wxDynamicCast(editor->GetParent(), wxSTEditorSplitter)->HandleMenuEvent(event))
            return true;
        if (editor->HandleMenuEvent(event))
            return true;
    }

    if ((win_id >= wxID_FILE1) && (win_id <= wxID_FILE9))
    {
        if (GetOptions().GetFileHistory())
        {
            wxFileName fileName = GetOptions().GetFileHistory()->GetHistoryFile(win_id-wxID_FILE1);
            LoadFile(fileName, true);
        }

        return true;
    }

    switch (win_id)
    {
        case ID_STE_SHOW_FULLSCREEN :
        {
            long style = wxFULLSCREEN_NOBORDER|wxFULLSCREEN_NOTOOLBAR|wxFULLSCREEN_NOCAPTION;
            ShowFullScreen(event.IsChecked(), style);
            return true;
        }
        case ID_STF_SHOW_SIDEBAR :
        {
            ShowSidebar(event.IsChecked());
            return true;
        }
        case wxID_EXIT :
        {
            if (GetEditorNotebook())
            {
                if (!GetEditorNotebook()->QuerySaveIfModified())
                    return true;
            }
            else if (editor && (editor->QuerySaveIfModified(true) == wxCANCEL))
                return true;

            Destroy();
            return true;
        }
        case wxID_ABOUT :
        {
            wxSTEditorAboutDialog(this);
            return true;
        }
        default : break;
    }

    return false;
}

void wxSTEditorFrame::OnClose( wxCloseEvent &event )
{
    int style = event.CanVeto() ? wxYES_NO|wxCANCEL : wxYES_NO;

    if (GetEditorNotebook())
    {
        if (!GetEditorNotebook()->QuerySaveIfModified(style))
        {
            event.Veto(true);
            return;
        }
    }
    else if (GetEditor() && (GetEditor()->QuerySaveIfModified(true, style) == wxCANCEL))
    {
        event.Veto(true);
        return;
    }

    // **** Shutdown so that the focus event doesn't crash the editors ****
    SetSendSTEEvents(false);
    event.Skip();
}

//-----------------------------------------------------------------------------
// wxSTEditorFileDropTarget
//-----------------------------------------------------------------------------
#if wxUSE_DRAG_AND_DROP

bool wxSTEditorFileDropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                           const wxArrayString& filenames)
{
    wxCHECK_MSG(m_owner, false, wxT("Invalid file drop target"));

    const size_t count = filenames.GetCount();
    if (count == 0)
        return false;

    // Try to find the best window to use to load the files
    wxSTEditorFrame*    stEditorFrame    = NULL;
    wxSTEditorNotebook* stEditorNotebook = NULL;
    wxSTEditorSplitter* stEditorSplitter = NULL;
    wxSTEditor*         stEditor         = NULL;

    wxWindow* parent = m_owner;

    while (parent)
    {
        if (wxDynamicCast(parent, wxSTEditorFrame) != NULL)
        {
            stEditorFrame = wxDynamicCast(parent, wxSTEditorFrame);
            break;
        }
        else if (wxDynamicCast(parent, wxSTEditorNotebook) != NULL)
        {
            stEditorNotebook = wxDynamicCast(parent, wxSTEditorNotebook);
            break; // once we find a notebook, we'll use it
        }
        else if (wxDynamicCast(parent, wxSTEditorSplitter) != NULL)
        {
            stEditorSplitter = wxDynamicCast(parent, wxSTEditorSplitter);
        }
        else if (wxDynamicCast(parent, wxSTEditor) != NULL)
        {
            stEditor = wxDynamicCast(parent, wxSTEditor);
        }

        parent = parent->GetParent();
    }

    // These are in order of preference to use to load the files

    if (stEditorFrame != NULL)
    {
        // see if it has a notebook and use it to load the files
        if (stEditorFrame->GetEditorNotebook())
        {
            wxArrayString files = filenames;
            stEditorFrame->GetEditorNotebook()->LoadFiles(&files);
        }
        else if (stEditorFrame->GetEditor())
            stEditorFrame->GetEditor()->LoadFile(filenames[0]);

        return true;
    }
    else if (stEditorNotebook != NULL)
    {
        wxArrayString files = filenames;
        stEditorNotebook->LoadFiles(&files);
        return true;
    }
    else if (stEditorSplitter != NULL)
    {
        stEditorSplitter->GetEditor()->LoadFile(filenames[0]);
        return true;
    }
    else if (stEditor != NULL)
    {
        stEditor->LoadFile(filenames[0]);
        return true;
    }

    return false;
}

#endif //wxUSE_DRAG_AND_DROP
