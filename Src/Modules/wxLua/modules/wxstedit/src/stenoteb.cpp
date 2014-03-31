///////////////////////////////////////////////////////////////////////////////
// Name:        stenoteb.cpp
// Purpose:     wxSTEditorNotebook
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// RCS-ID:
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "wx/stedit/stedit.h"
#include "wx/stedit/stenoteb.h"
#include "wx/stedit/stetree.h"
#include "wx/stedit/stedlgs.h"
#include <wx/progdlg.h>   // wxProgressDialog

#include "wxext.h"   // FileNameArray

//-----------------------------------------------------------------------------
// sorting function for strings, after = is the page #, don't sort by that
//-----------------------------------------------------------------------------
int wxCMPFUNC_CONV STN_SortNameCompareFunction(const wxString& first, const wxString& second)
{
    int ret = wxStrcmp(first.BeforeLast(wxT('=')), second.BeforeLast(wxT('=')));
    if (ret == 0)
    {
        // same names, keep the same order
        long f = 0, s = 0;
        wxCHECK_MSG(first.AfterLast(wxT('=')).ToLong(&f), 0, wxT("Invalid first page name for sorting"));
        wxCHECK_MSG(second.AfterLast(wxT('=')).ToLong(&s), 0, wxT("Invalid second page name for sorting"));
        ret = f > s ? 1 : -1;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// wxSTEditorNotebook
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSTEditorNotebook, wxNotebook)

BEGIN_EVENT_TABLE(wxSTEditorNotebook, wxNotebook)
#if defined(__WXMSW__) && (wxVERSION_NUMBER >= 2900)
    EVT_LEFT_UP                (wxSTEditorNotebook::OnLeftUp)
#endif
    EVT_RIGHT_UP               (wxSTEditorNotebook::OnRightUp)
    EVT_MIDDLE_UP              (wxSTEditorNotebook::OnMiddleUp)
    EVT_MENU                   (wxID_ANY, wxSTEditorNotebook::OnMenu)
    EVT_STEDITOR_STATE_CHANGED (wxID_ANY, wxSTEditorNotebook::OnSTEState)
    EVT_NOTEBOOK_PAGE_CHANGED  (wxID_ANY, wxSTEditorNotebook::OnPageChanged)

    EVT_FIND                   (wxID_ANY, wxSTEditorNotebook::OnFindDialog)
    EVT_FIND_NEXT              (wxID_ANY, wxSTEditorNotebook::OnFindDialog)
    EVT_FIND_REPLACE           (wxID_ANY, wxSTEditorNotebook::OnFindDialog)
    EVT_FIND_REPLACE_ALL       (wxID_ANY, wxSTEditorNotebook::OnFindDialog)
    EVT_FIND_CLOSE             (wxID_ANY, wxSTEditorNotebook::OnFindDialog)
    EVT_STEFIND_GOTO           (wxID_ANY, wxSTEditorNotebook::OnFindDialog)
END_EVENT_TABLE()

void wxSTEditorNotebook::Init()
{
    m_stn_page_count     = 0;
    m_stn_selection      = -1;
    m_stn_max_page_count = STN_NOTEBOOK_PAGES_ALLOWED;
}

bool wxSTEditorNotebook::Create( wxWindow *parent, wxWindowID id,
                                 const wxPoint& pos, const wxSize& size,
                                 long style, const wxString& name)
{
    if (!wxNotebook::Create(parent, id, pos, size, style, name))
        return false;

    wxCommandEvent event(wxEVT_STNOTEBOOK_CREATED, GetId());
    event.SetEventObject(this);
    GetParent()->GetEventHandler()->ProcessEvent(event);
    return true;
}

wxSTEditorNotebook::~wxSTEditorNotebook()
{
    SetSendSTEEvents(false);
}

bool wxSTEditorNotebook::Destroy()
{
    SetSendSTEEvents(false);
    return wxNotebook::Destroy();
}
void wxSTEditorNotebook::SetSendSTEEvents(bool send)
{
    int n, count = (int)GetPageCount();
    for (n = 0; n < count; n++)
    {
        wxSTEditorSplitter *splitter = GetEditorSplitter(n);
        if (splitter) splitter->SetSendSTEEvents(send);
    }
}

void wxSTEditorNotebook::CreateOptions(const wxSTEditorOptions& options)
{
    m_options = options;

    // create the popupmenu if desired
    wxSTEditorMenuManager *steMM = GetOptions().GetMenuManager();
    if (steMM && GetOptions().HasNotebookOption(STN_CREATE_POPUPMENU) &&
        !GetOptions().GetNotebookPopupMenu())
    {
        GetOptions().SetNotebookPopupMenu(steMM->CreateNotebookPopupMenu(), false);
    }

#if wxUSE_DRAG_AND_DROP
    if (GetOptions().HasNotebookOption(STN_DO_DRAG_AND_DROP))
    {
        SetDropTarget(new wxSTEditorFileDropTarget(this));
    }
#endif //wxUSE_DRAG_AND_DROP
}

wxString wxSTEditorNotebook::FileNameToTabName(const wxSTEditor* editor) const
{
    wxString name(editor->GetFileName().GetFullName());
#ifdef __WXMSW__
    name.Replace(wxT("&"), wxT("&&"));
#endif
    if (editor->GetReadOnly())
    {
        name << wxT(" [") << _("Read only") << wxT("]");
    }
    if (editor->IsModified())
    {
        name << wxMODIFIED_ASTERISK;
    }
    return name;
}

void wxSTEditorNotebook::OnPageChanged(wxNotebookEvent &event)
{
    // this is our fake event to ensure selection is correct
    if (event.GetString() == wxT("wxSTEditorNotebook Page Change"))
    {
        SetSelection(event.GetExtraLong()); // FIXME no Clone in wxNotebookEvent
        return;
    }

    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);
    bool update_page_state = !guard.IsInside();

#if defined(__WXMSW__) && (wxVERSION_NUMBER < 2902) // OnSelChange() removed in trunk
    // let the msw notebook really change the page first
    wxNotebook::OnSelChange(event);
    event.Skip(false);
#else
    // trac.wxwidgets.org/ticket/12688, fixed now
    event.Skip();
#endif

    int sel = event.GetSelection();

    // NOTE: GTK selection can get out of sync, we "fix" it in GetEditorSplitter
    //if (sel >= wxNotebook::GetPageCount())
    //    sel = wxNotebook::GetPageCount()-1;

    // make sure the cursor is shown by setting the STC focus
    if ((sel >= 0) && GetEditor(sel))
    {
        GetEditor(sel)->SetSTCFocus(true);
        GetEditor(sel)->SetFocus();
    }

    if (update_page_state) UpdatePageState();
}

wxSTEditor *wxSTEditorNotebook::GetEditor( int page )
{
    wxSTEditorSplitter *splitter = GetEditorSplitter(page);
    if (splitter)
        return splitter->GetEditor();

    return NULL;
}

wxSTEditorSplitter *wxSTEditorNotebook::GetEditorSplitter( int page )
{
    int page_count = (int)GetPageCount();
    if (page_count == 0)
        return NULL;

    if ((page < 0) || (page >= page_count))
        page = GetSelection();

    if (((page < 0) && (page_count > 0)) || (page >= page_count))
    {
        SetSelection(0); // GTK can get out of sync, force it
        page = GetSelection();
    }
    if (page < 0)
        return NULL;

    return wxDynamicCast(GetPage(page), wxSTEditorSplitter);
}

int wxSTEditorNotebook::FindEditorPage(const wxSTEditor* editor)
{
    int sel = GetSelection();

    // assume that we want the selected editor so check it first
    if ((sel >= 0) && GetEditorSplitter(sel) &&
        ((GetEditorSplitter(sel)->GetEditor1() == editor) ||
         (GetEditorSplitter(sel)->GetEditor2() == editor)) )
    {
        return sel;
    }
    else
    {
        int n, n_pages = (int)GetPageCount();
        for (n = 0; n < n_pages; n++)
        {
            if (n == sel) continue;

            if (GetEditorSplitter(n) &&
                ((GetEditorSplitter(n)->GetEditor1() == editor) ||
                 (GetEditorSplitter(n)->GetEditor2() == editor)) )
            {
                return n;
            }
        }
    }

    return wxNOT_FOUND;
}

int wxSTEditorNotebook::FindEditorPageByFileName(const wxFileName& filename)
{
    int n, count = (int)GetPageCount();
    for (n = 0; n < count; n++)
    {
        wxSTEditor* editor = GetEditor(n);

        if (editor && (editor->GetFileName() == filename))
            return n;
    }
    return wxNOT_FOUND;
}

int wxSTEditorNotebook::FindEditorPageById(wxWindowID win_id)
{
    // TODO? Find a different type of window in the notebook?
    //       people probably shouldn't mix window types in this notebook.
    wxWindow *win = FindWindow(win_id);
    if (win && wxDynamicCast(win, wxSTEditor))
        return FindEditorPage((wxSTEditor*)win);

    return wxNOT_FOUND;
}

wxSTEditorSplitter *wxSTEditorNotebook::CreateSplitter(wxWindowID id)
{
    wxSTEditorSplitter *newSplitter = NULL;

    // Let someone create an editor and put it back into the event
    wxCommandEvent event(wxEVT_STNOTEBOOK_CREATE_SPLITTER, GetId());
    event.SetEventObject(this);
    event.SetInt((int)id);
    GetEventHandler()->ProcessEvent(event);

    // did anyone get the event and set their own splitter window
    if ((event.GetEventObject() != NULL) &&
        (wxDynamicCast(event.GetEventObject(), wxSTEditorSplitter) != NULL))
    {
        newSplitter = wxDynamicCast(event.GetEventObject(), wxSTEditorSplitter);
        if (newSplitter->GetParent() != this)
        {
            wxFAIL_MSG(wxT("Incorrect parent for wxSTEditorSplitter, should be wxSTEditorNotebook"));
            return NULL;
        }
    }
    else
    {
        newSplitter = new wxSTEditorSplitter(this, id, wxDefaultPosition,
                                             wxDefaultSize, wxSP_3D);
        newSplitter->CreateOptions(GetOptions());
    }

    return newSplitter;
}

wxSTEditorSplitter* wxSTEditorNotebook::InsertEditorSplitter(int nPage, wxWindowID win_id,
                                                             const wxString& title, bool bSelect)
{
    if (GetPageCount() >= GetMaxPageCount())
    {
        wxMessageBox(_("Maximum number of notebook pages exceeded,\nplease close one first."),
                     _("Too many pages opened"), wxOK|wxICON_ERROR, this);

        return NULL;
    }

    wxSTEditorSplitter *splitter = CreateSplitter(win_id);
    wxCHECK_MSG(splitter, NULL, wxT("Invalid splitter"));
    splitter->GetEditor()->NewFile(title);
    if (!InsertEditorSplitter(nPage, splitter, bSelect))
    {
       wxDELETE(splitter); // failed to insert it, delete it to not leak memory
    }
    return splitter;
}

bool wxSTEditorNotebook::InsertEditorSplitter(int nPage, wxSTEditorSplitter* splitter,
                                              bool bSelect)
{
    wxCHECK_MSG(splitter && (splitter->GetParent() == this), false,
                wxT("Invalid wxSTEditorSplitter or parent"));

    if (GetPageCount() >= GetMaxPageCount())
    {
        wxMessageBox(_("Maximum number of notebook pages exceeded,\nplease close one first."),
                     _("Too many pages opened"), wxOK|wxICON_ERROR, this);

        delete splitter;
        return false;
    }

    wxString title(FileNameToTabName(splitter->GetEditor()));
    size_t n_pages = GetPageCount();

    if (nPage < 0) // they want to insert it anywhere
    {
        // presort the insert page to reduce flicker
        if ((n_pages > 0) && GetOptions().HasNotebookOption(STN_ALPHABETICAL_TABS))
        {
            wxArrayString names;
            names.Add(title+wxT("=999999")); // insert after any other pages with same name

            for (size_t n = 0; n < n_pages; n++)
            {
                wxString name(GetPageText(n));
                if ((name.Length() > 0) && (name[0u] == wxT('*')))
                    name = name.Mid(1);

                names.Add(name + wxString::Format(wxT("=%d"), (int)n));
            }

            names.Sort(STN_SortNameCompareFunction);
            nPage = names.Index(title+wxT("=999999"));
        }
        else
            nPage = (int)n_pages;
    }

    if (n_pages < 1)
        bSelect = true;
    if (nPage < int(n_pages))
        return InsertPage(nPage, splitter, title, bSelect);

    bool ret = AddPage(splitter, title, bSelect);
    UpdateAllItems();
    return ret;
}

void wxSTEditorNotebook::SortTabs(int style)
{
    if ((int)GetPageCount() < 2)
        return;

    if (STE_HASBIT(style, STN_ALPHABETICAL_TABS))
    {
        int sel = GetSelection();
        int new_sel = sel;
        size_t page_count = GetPageCount();
        size_t n;

        if (page_count < 2)
            return;

        wxString curPageName;
        wxArrayString names;

        for (n = 0; n < page_count; n++)
        {
            wxString name(GetPageText(n));
            if ((name.Length() > 0) && (name[0u] == wxT('*')))
                name = name.Mid(1);

            names.Add(name + wxString::Format(wxT("=%d"), (int)n));
        }

        names.Sort(STN_SortNameCompareFunction);

        bool sel_changed = false;

        for (n = 0; n < page_count; n++)
        {
            long old_page = 0;
            names[n].AfterLast(wxT('=')).ToLong(&old_page);

            if (old_page != long(n))
            {
                wxWindow *oldWin = GetPage(old_page);
                wxString oldName(GetPageText(old_page));

                if (oldWin && RemovePage(old_page))
                {
                    sel_changed = true;

                    if (old_page == sel)
                        new_sel = (int)n;

                    if (n < page_count - 1)
                        InsertPage((int)(n+1), oldWin, oldName, old_page == sel);
                    else
                        AddPage(oldWin, oldName, old_page == sel);
                }
            }
        }

        if (sel_changed)
        {
            wxNotebookEvent noteEvent(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, GetId(),
                                    new_sel, new_sel);
            noteEvent.SetString(wxT("wxSTEditorNotebook Page Change"));
            noteEvent.SetExtraLong(new_sel); // FIXME no Clone in wxNotebookEvent
            // NOTE: this may have to be AddPendingEvent for wx < 2.7 since gtk
            //       can become reentrant
            GetEventHandler()->AddPendingEvent(noteEvent);
        }

        // causes reentrant assert in gtk, even though it's necessary sometimes
        //SetSelection(new_sel); // force selection for GTK
    }
}

bool wxSTEditorNotebook::ClosePage(int n, bool query_save_if_modified)
{
    wxCHECK_MSG((n >= 0) && (n < (int)GetPageCount()), false, wxT("Invalid page"));
    wxSTEditor *editor = GetEditor(n);

    if (!editor)
        return false;

    int ret = wxID_NO;
    int sel = GetSelection();

    if (query_save_if_modified)
        ret = editor->QuerySaveIfModified(true);

    if (ret != wxCANCEL)
        ret = int(DeletePage(n));

    if ((GetPageCount() == 0) && !GetOptions().HasNotebookOption(STN_ALLOW_NO_PAGES))
        InsertEditorSplitter(-1, wxID_ANY, GetOptions().GetDefaultFileName(), true);

    // Force selection for GTK, else if try to close the "current page" without
    //  first clicking in it you delete some other page
    int page_count = (int)GetPageCount();
    if ((sel >= page_count) && (page_count > 0))
        SetSelection(wxMax(0, wxMin(sel, page_count-1)));

    UpdateAllItems();

    return ret != 0;
}

bool wxSTEditorNotebook::CloseAllPages(bool query_save_if_modified, int except_this_page)
{
    if (query_save_if_modified && !QuerySaveIfModified())
        return false;

    if (except_this_page < 0)
    {
        DeleteAllPages();
    }
    else
    {
        wxWindow* win = GetPage(except_this_page);
        wxString title(GetPageText(except_this_page));

        if (win && RemovePage(except_this_page))
        {
            DeleteAllPages();
            AddPage(win, title, true);
        }
    }

    if ((GetPageCount() == 0) && !GetOptions().HasNotebookOption(STN_ALLOW_NO_PAGES))
        InsertEditorSplitter(-1, wxID_ANY, GetOptions().GetDefaultFileName(), true);

    UpdateAllItems();
    return true;
}

bool wxSTEditorNotebook::QuerySaveIfModified(int style)
{
    int n_pages = (int)GetPageCount();

    for (int n = 0; n < n_pages; n++)
    {
        wxSTEditor *editor = GetEditor(n);
        if (editor)
        {
            if (editor->QuerySaveIfModified(true, style) == wxCANCEL)
                return false;
        }
    }

    return true;
}

bool wxSTEditorNotebook::CanSaveAll()
{
    int n, n_pages = (int)GetPageCount();
    wxSTEditor *editor = NULL;

    for (n = 0; n < n_pages; n++)
    {
        editor = GetEditor(n);

        if (editor && editor->CanSave())
            return true;
    }

    return false;
}

#if defined(__WXMSW__) && (wxVERSION_NUMBER >= 2900)
void wxSTEditorNotebook::OnLeftUp(wxMouseEvent &event)
{
    wxPoint pos = event.GetPosition();
    int page = HitTest(pos, NULL);

    if (page != wxNOT_FOUND)
    {
        GetPage(page)->SetFocus();
    }
    event.Skip();
}
#endif

void wxSTEditorNotebook::OnRightUp(wxMouseEvent &event)
{
    wxMenu* popupMenu = GetOptions().GetNotebookPopupMenu();
    if (popupMenu)
    {
        UpdateItems(popupMenu);
        PopupMenu(popupMenu, event.GetPosition());
    }
    else
        event.Skip();
}

void wxSTEditorNotebook::OnMiddleUp(wxMouseEvent &event)
{
    wxPoint pos = event.GetPosition();
    long flags = 0;
    int page = HitTest(pos, &flags);

    if ((page != wxNOT_FOUND) && ((flags & wxBK_HITTEST_NOWHERE) == 0))
    {
        ClosePage(page, true);
    }
    else
        event.Skip();
}

void wxSTEditorNotebook::OnMenu(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_OnMenu);
    if (guard.IsInside()) return;

    if (!HandleMenuEvent(event))
        event.Skip();
}

bool wxSTEditorNotebook::HandleMenuEvent(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_HandleMenuEvent);
    if (guard.IsInside()) return false;

    int n_page = (int)GetPageCount();
    int win_id = event.GetId();

    switch (win_id)
    {
        case wxID_NEW:
        {
            NewPage();
            return true;
        }
        case wxID_OPEN:
        {
            LoadFiles();
            return true;
        }
        case wxID_SAVEAS:
        {
            wxSTEditor *editor = GetEditor();
            if (!editor) return true; // event handled, but we couldn't do anything with it.

            if (!editor->IsFileFromDisk())
            {
                editor->SaveFile(true);
            }
            else
            {
                wxFileName selectedFileName;
                wxString   selectedFileEncoding;
                bool       selected_file_bom = false;

                bool ok = editor->SaveFileDialog(true, wxEmptyString, &selectedFileName, &selectedFileEncoding, &selected_file_bom);
                if (!ok) return true; // they probably canceled the dialog

                if (selectedFileName == editor->GetFileName())
                {
                    // They want to save to the same filename, update current editor.
                    editor->SaveFile(selectedFileName, selectedFileEncoding, selected_file_bom);
                    return true;
                }
                else
                {
                    // Make a new editor for the new filename, leave the original editor as is.
                    wxSTEditorSplitter *splitter = CreateSplitter(wxID_ANY);
                    wxCHECK_MSG(splitter, true, wxT("Invalid splitter"));
                    wxSTEditor *newEditor = splitter->GetEditor();
                    wxCHECK_MSG(newEditor, true, wxT("Invalid splitter editor"));

                    // Make this new editor identical to the original one
                    // these are probably not necessary
                    //splitter->GetEditor()->SetOptions(editor->GetOptions());
                    //splitter->GetEditor()->RegisterPrefs(editor->GetEditorPrefs());
                    //splitter->GetEditor()->RegisterStyles(editor->GetEditorStyles());
                    //splitter->GetEditor()->RegisterLangs(editor->GetEditorLangs());

                    newEditor->SetLanguage(editor->GetLanguageId());
                    newEditor->SetFileName(editor->GetFileName());
                    newEditor->SetFileEncoding(editor->GetFileEncoding());
                    newEditor->SetFileBOM(editor->GetFileBOM());

                    newEditor->SetText(editor->GetText());
                    newEditor->ColouriseDocument();
                    newEditor->GotoPos(editor->PositionFromLine(editor->LineFromPosition(editor->GetCurrentPos())));
                    newEditor->GotoPos(editor->GetCurrentPos());
                    newEditor->ScrollToLine(editor->GetFirstVisibleLine());

                    // if we can save it, then add it to the notebook
                    if (newEditor->SaveFile(selectedFileName, selectedFileEncoding, selected_file_bom))
                    {
                        if (!InsertEditorSplitter(-1, splitter, true))
                            splitter->Destroy();
                    }
                    else
                        splitter->Destroy(); // problem saving, delete new editor
                }
            }
            return true;
        }
        case ID_STN_SAVE_ALL:
        {
            SaveAllFiles();
            return true;
        }
        case ID_STN_CLOSE_PAGE:
        {
            if ((GetSelection() != -1) && GetEditor(GetSelection()))
            {
                ClosePage(GetSelection(), true);
            }
            return true;
        }
        case ID_STN_CLOSE_ALL:
        {
            if (wxYES == wxMessageBox(_("Close all pages?"), _("Confim closing all pages"),
                                   wxICON_QUESTION|wxYES_NO, this))
            {
                CloseAllPages(true, -1);
            }
            return true;
        }
        case ID_STN_CLOSE_ALL_OTHERS:
        {
            CloseAllPages(true, GetSelection());
            return true;
        }
        case ID_STN_WIN_PREVIOUS:
        {
            if ((GetPageCount() > 0) && (GetSelection() - 1 >= 0))
                SetSelection(GetSelection() - 1);
            else if (GetPageCount() > 0)
                SetSelection((int)GetPageCount() - 1);
            return true;
        }
        case ID_STN_WIN_NEXT:
        {
            if ((GetPageCount() > 0) && (GetSelection() + 1 < (int)GetPageCount()))
                SetSelection(GetSelection() + 1);
            else if (GetPageCount() > 0)
                SetSelection(0);
            return true;
        }
        case ID_STN_WINDOWS:
        {
            wxSTEditorWindowsDialog(this, _("Windows"));
            return true;
        }
        case ID_STE_PASTE_NEW:
        {
            wxString text;
            if (wxSTEditor::GetClipboardText(&text))
            {
                NewPage();
                wxSTEditor* editor = GetEditor();
                if (editor)
                {
                    editor->SetText(text);
                    editor->SetModified(false);
                }
            }
            return true;
        }
        default:
        {
            if ((win_id >= ID_STN_GOTO_PAGE_START) && (win_id < ID_STN_GOTO_PAGE_START+n_page))
            {
                SetSelection(win_id - ID_STN_GOTO_PAGE_START);
                return true;
            }
            else if ((win_id >= ID_STN_CLOSE_PAGE_START) && (win_id < ID_STN_CLOSE_PAGE_START+n_page))
            {
                ClosePage(win_id - ID_STN_CLOSE_PAGE_START);
                return true;
            }
            break;
        }
    }
    return false;
}

void wxSTEditorNotebook::OnSTEState(wxSTEditorEvent &event)
{
    event.Skip(true);
    wxSTEditor *editor = event.GetEditor();

    if ( event.HasStateChange(STE_FILENAME | STE_MODIFIED) )
    {
        if (GetOptions().HasNotebookOption(STN_UPDATE_TITLES))
        {
            int page = FindEditorPage(editor);
            if (page >= 0) // if < 0 then not in notebook (or at least yet)
            {
                SetPageText(page, FileNameToTabName(editor));
                SortTabs(GetOptions().GetNotebookOptions());
            }
        }
    }

    if (event.HasStateChange(STE_FILENAME | STE_MODIFIED | STE_CANSAVE))
    {
        UpdateAllItems();
    }
}

bool wxSTEditorNotebook::NewPage( const wxString& title_ )
{
    wxString title(title_);

    if (title.IsEmpty())
    {
        title = GetOptions().GetDefaultFileName();
                //wxGetTextFromUser(_("New file name"), _("New file"),
                //                  GetOptions().GetDefaultFileName(), this);
    }

    if (title.Length())
    {
        wxSTEditorSplitter *splitter = CreateSplitter(wxID_ANY);
        wxCHECK_MSG(splitter, true, wxT("Invalid splitter"));
        splitter->GetEditor()->NewFile(title);
        InsertEditorSplitter(-1, splitter, true);
        return true;
    }

    return false;
}

bool wxSTEditorNotebook::LoadFile( const wxFileName &fileName_,
                                   const wxString &extensions_,
                                   const wxString& encoding_ref)
{
    wxString encoding(encoding_ref);
    wxFileName fileName(fileName_);
    wxString extensions(extensions_.Length() ? extensions_ : GetOptions().GetDefaultFileExtensions());

    if (fileName.GetFullPath().IsEmpty())
    {
        wxSTEditorFileDialog fileDialog( this, _("Open file into new notebook page"),
                                 GetOptions().GetDefaultFilePath(),
                                 extensions,
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        fileDialog.m_encoding = encoding;
        if (fileDialog.ShowModal() == wxID_OK)
        {
            fileName = fileDialog.GetPath();
            encoding = fileDialog.m_encoding;
        }
        else
            return false;
    }

    bool ok = fileName.FileExists();
    if (ok)
    {
        // load the file from disk and only load it once
        GetOptions().SetDefaultFilePath(fileName.GetPath());

        int page = FindEditorPageByFileName(fileName);
        if (page != wxNOT_FOUND)
        {
            ok = GetEditor(page)->LoadFile(fileName, wxEmptyString, true, encoding);
            SetSelection(page);
        }
        else if ( (GetEditor() == NULL) || GetEditor()->IsModified() || GetEditor()->IsFileFromDisk()) // non-empty editor?
        {
            // new splitter+editor
            wxSTEditorSplitter *splitter = CreateSplitter(wxID_ANY);
            wxCHECK_MSG(splitter, false, wxT("Invalid splitter"));
            ok = splitter->GetEditor()->LoadFile(fileName, wxEmptyString, true, encoding);
            if (ok)
            {
                ok = InsertEditorSplitter(-1, splitter, true);
            }
        }
        else // empty editor
        {
            // reuse editor
            ok = GetEditor()->LoadFile(fileName, wxEmptyString, true, encoding);
        }
    }
    return ok;
}

bool wxSTEditorNotebook::LoadFiles( wxArrayString *filePaths_,
                                    const wxString &extensions_)
{
    wxString extensions(extensions_.Length() ? extensions_ : GetOptions().GetDefaultFileExtensions());
    wxArrayString filePaths;
    wxString encoding;

    if (filePaths_)
        filePaths = *filePaths_;

    if (filePaths.GetCount() < 1u)
    {
        wxSTEditorFileDialog fileDialog( this, _("Open file(s) into new notebook page"),
                                 GetOptions().GetDefaultFilePath(),
                                 extensions,
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

        fileDialog.m_encoding = encoding;
        if (fileDialog.ShowModal() == wxID_OK)
        {
            fileDialog.GetPaths(filePaths);
            encoding = fileDialog.m_encoding;
        }
        else
            return false;
    }

    if (!filePaths.GetCount())
        return false;

    size_t n, count = filePaths.GetCount();
    size_t max_filename_len = 80;
    //for (n = 0; n < count; n++) max_filename_len = wxMax(max_filename_len, filePaths[n].Length());

    wxProgressDialog progDlg(_("Loading files..."),
                             wxString(wxT('_'), max_filename_len + 10), // +10 for file count
                             (int)filePaths.GetCount(), this,
                             wxPD_CAN_ABORT|wxPD_ELAPSED_TIME|wxPD_APP_MODAL|wxPD_AUTO_HIDE);

    // block updating the pages while loading them
    if (m_editorTreeCtrl != NULL) m_editorTreeCtrl->Freeze();
    {
    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);

    for (n = 0; n < count; n++)
    {
        wxString fileName(filePaths[n]);
        wxString progressFileName;
        // Ellipsize filename that are too long.
        if (fileName.Length() > max_filename_len)
            progressFileName = fileName.Mid(0, max_filename_len/2-2) + wxT(" ... ") + fileName.Mid(fileName.Length()-max_filename_len/2+2);
        else
            progressFileName = fileName;

        if (!progDlg.Update((int)n, wxString::Format(wxT("%d/%d : "), (int)n+1, (int)count) + progressFileName))
            break;

        if (fileName.IsEmpty() || !wxFileExists(fileName))
        {
            // when selecting multiple files with file selector you can easily
            // select the dir "..", throw it away
            wxString theFileName(fileName.AfterLast(wxFILE_SEP_PATH));
            if ((theFileName != wxT("..")) && (theFileName != wxT(".")))
            {
                wxSTEditorSplitter *splitter = CreateSplitter(wxID_ANY);
                wxCHECK_MSG(splitter, false, wxT("invalid splitter"));
                splitter->GetEditor()->NewFile(fileName);
                if (!InsertEditorSplitter(-1, splitter)) break; // checks overflow
            }
        }
        else
        {
            if (!LoadFile(fileName, wxEmptyString, encoding)) break;
        }
    }
    }

    UpdatePageState();
    if (m_editorTreeCtrl != NULL)
    {
        m_editorTreeCtrl->Thaw();
        m_editorTreeCtrl->UpdateFromNotebook();
    }

    return true;
}

bool wxSTEditorNotebook::LoadFiles( const wxArrayFileName* fileNames,
                                    const wxString &extensions)
{
    if (fileNames != NULL)
    {
        wxArrayString filePaths;

        size_t n, count = fileNames->GetCount();
        for (n = 0; n < count; n++)
        {
            filePaths.Add(fileNames->Item(n).GetFullPath());
        }

        return LoadFiles(&filePaths, extensions);
    }

    return LoadFiles((wxArrayString*)NULL, extensions); // use file dialog to ask user
}

void wxSTEditorNotebook::SaveAllFiles()
{
    int n_page = (int)GetPageCount();
    wxSTEditor *editor = NULL;

    for (int n = 0; n < n_page; n++)
    {
        editor = GetEditor(n);
        if (editor && editor->CanSave())
            editor->SaveFile(false);
    }
}

void wxSTEditorNotebook::UpdateGotoCloseMenu(wxMenu *menu, int startID)
{
    if (!menu) return;

    size_t n, page_count = GetPageCount();
    size_t item_count = menu->GetMenuItemCount();

// ========  Radio items have problems in gtk
/*
    // delete extra menu items (if any)
    if (page_count < item_count)
    {
        for (n=page_count; n < item_count; n++)
            menu->Delete(startID+n);

        item_count = page_count;
    }

    wxString label;

    // change labels of existing items
    for (n=0; n<item_count; n++)
    {
        label = wxString::Format(wxT("%2d : %s"), n+1, GetPageText(n).wx_str());
        if (menu->GetLabel(startID+n) != label)
            menu->SetLabel(startID+n, label);
    }
    // append new pages
    for (n=item_count; n<page_count; n++)
        menu->AppendRadioItem(startID+n, wxString::Format(wxT("%2d : %s"), n+1, GetPageText(n).wx_str()));
*/
/*
    // This just clears it and adds the items fresh
    for (n=0; n<item_count; n++)
        menu->Delete(startID+n);
    for (n=0; n<page_count; n++)
        menu->AppendRadioItem(startID+n, wxString::Format(wxT("%2d : %s"), n+1, GetPageText(n).wx_str()));
*/

// ==== check items do not

    // delete extra menu items (if any)
    if (page_count < item_count)
    {
        for (n = page_count; n < item_count; n++)
            menu->Delete(int(startID+n));

        item_count = page_count;
    }

    wxString label;

    // change labels of existing items
    for (n = 0; n < item_count; n++)
    {
        label = wxString::Format(wxT("%2d : %s"), (int)n+1, GetPageText(n).wx_str());
        if (menu->GetLabel(int(startID+n)) != label)
            menu->SetLabel(int(startID+n), label);

        menu->Check(int(startID+n), false);
    }
    // append new pages
    for (n = item_count; n < page_count; n++)
        menu->AppendCheckItem(int(startID+n), wxString::Format(wxT("%2d : %s"), (int)n+1, GetPageText(n).wx_str()));

/*
    // use check items
    for (n = 0; n < item_count; n++)
        menu->Delete(startID+n);
    for (n = 0; n < page_count; n++)
        menu->AppendCheckItem(startID+n, wxString::Format(wxT("%2d : %s"), n+1, GetPageText(n).wx_str()));
*/

    // show what page we're on
    int sel = GetSelection();
    if ((sel >= 0) && (page_count >= 0))
        menu->Check(startID+sel, true);
}

void wxSTEditorNotebook::UpdateAllItems()
{
    UpdateItems(GetOptions().GetEditorPopupMenu(), GetOptions().GetMenuBar(),
                                                   GetOptions().GetToolBar());
    UpdateItems(GetOptions().GetNotebookPopupMenu());
    UpdateItems(GetOptions().GetSplitterPopupMenu());
}
void wxSTEditorNotebook::UpdateItems(wxMenu *menu, wxMenuBar *menuBar, wxToolBar *toolBar)
{
    if (!menu && !menuBar && !toolBar) return;

    bool has_pages    = GetPageCount() > 0;
    bool can_save_all = CanSaveAll();
    bool editor_page  = GetEditor() != NULL;

    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_SAVE_ALL, can_save_all);

    if (menu)
    {
        wxMenuItem *gotoMenuItem = menu->FindItem(ID_STN_MENU_GOTO);
        if (gotoMenuItem)
            UpdateGotoCloseMenu(gotoMenuItem->GetSubMenu(), ID_STN_GOTO_PAGE_START);

        wxMenuItem *closeMenuItem = menu->FindItem(ID_STN_MENU_CLOSE);
        if (closeMenuItem)
            UpdateGotoCloseMenu(closeMenuItem->GetSubMenu(), ID_STN_CLOSE_PAGE_START);
    }
    if (menuBar)
    {
        wxMenuItem *gotoMenuItem = menuBar->FindItem(ID_STN_MENU_GOTO);
        if (gotoMenuItem)
            UpdateGotoCloseMenu(gotoMenuItem->GetSubMenu(), ID_STN_GOTO_PAGE_START);

        wxMenuItem *closeMenuItem = menuBar->FindItem(ID_STN_MENU_CLOSE);
        if (closeMenuItem)
            UpdateGotoCloseMenu(closeMenuItem->GetSubMenu(), ID_STN_CLOSE_PAGE_START);
    }

    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_WIN_PREVIOUS, has_pages); // && (GetSelection() > 0));
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_WIN_NEXT,     has_pages); // && (GetSelection()+1 < (int)GetPageCount()));

    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_MENU_GOTO,        has_pages);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_CLOSE_PAGE,       editor_page);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_CLOSE_ALL,        has_pages);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_CLOSE_ALL_OTHERS, has_pages);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STN_MENU_CLOSE,       has_pages);
}

void wxSTEditorNotebook::UpdatePageState()
{
    int page_count = (int)GetPageCount();
    int selection  = GetSelection();
    if (page_count < 1) selection = -1; // force for gtk

    if ((page_count == m_stn_page_count) && (selection == m_stn_selection))
        return;

    wxNotebookEvent stnEvent(wxEVT_STNOTEBOOK_PAGE_CHANGED, GetId());
    stnEvent.SetEventObject(this);
    stnEvent.SetSelection(selection);
    stnEvent.SetOldSelection(m_stn_selection);

    m_stn_page_count = page_count;
    m_stn_selection  = selection;

    GetEventHandler()->ProcessEvent(stnEvent);

    UpdateAllItems();
}

bool wxSTEditorNotebook::AddPage(wxWindow *page, const wxString& text,
                                 bool bSelect, int imageId)
{
    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);
    bool ret = wxNotebook::AddPage(page, text, bSelect, imageId);
    if (!guard.IsInside()) UpdatePageState();
    return ret;
}
bool wxSTEditorNotebook::InsertPage(size_t nPage, wxNotebookPage *pPage,
                                    const wxString& strText, bool bSelect, int imageId)
{
    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);
    bool ret = wxNotebook::InsertPage(nPage, pPage, strText, bSelect, imageId);
    if (!guard.IsInside()) UpdatePageState();
    return ret;
}
int wxSTEditorNotebook::GetSelection() const
{
    wxSTEditorNotebook* noteBook = (wxSTEditorNotebook*)this; // unconst this
    wxSTERecursionGuard guard(noteBook->m_rGuard_UpdatePageState);
    int  ret = wxNotebook::GetSelection();
    if (!guard.IsInside()) noteBook->UpdatePageState();
    return ret;
}
int wxSTEditorNotebook::SetSelection(size_t nPage)
{
    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);
    int  ret = wxNotebook::SetSelection(nPage);
    if (!guard.IsInside()) UpdatePageState();
    return ret;
}
bool wxSTEditorNotebook::DeletePage(size_t nPage)
{
    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);
    bool ret = wxNotebook::DeletePage(nPage);
    if (!guard.IsInside()) UpdatePageState();
    return ret;
}
bool wxSTEditorNotebook::RemovePage(size_t nPage)
{
    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);
    bool ret = wxNotebook::RemovePage(nPage);
    if (!guard.IsInside()) UpdatePageState();
    return ret;
}
bool wxSTEditorNotebook::DeleteAllPages()
{
    wxSTERecursionGuard guard(m_rGuard_UpdatePageState);
    bool ret = wxNotebook::DeleteAllPages();
    if (!guard.IsInside()) UpdatePageState();
    return ret;
}

void wxSTEditorNotebook::OnFindDialog(wxFindDialogEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_OnFindDialog);
    if (guard.IsInside()) return;

    wxEventType eventType = event.GetEventType();
    wxString findString   = event.GetFindString();
    long flags            = event.GetFlags();

    // For this event we only go to the previously found location.
    if (eventType == wxEVT_STEFIND_GOTO)
    {
        wxSTEditorFoundStringData foundStringData;
        bool ok = foundStringData.FromString(event.GetString());

        int page = wxNOT_FOUND;

        if (ok)
            page = FindEditorPageByFileName(foundStringData.GetFileName());

        if (page != wxNOT_FOUND)
        {
            SetSelection(page);
            GetEditor(page)->HandleFindDialogEvent(event);
        }

        return;
    }

    // currently opened page is where the search starts
    wxSTEditor *editor = GetEditor();

    if (!editor)
        return;

    // just search the given page by letting the editor handle it
    if (!STE_HASBIT(event.GetFlags(), STE_FR_ALLDOCS))
    {
        editor->HandleFindDialogEvent(event);
        return;
    }

    editor->SetFindString(findString, true);
    editor->SetFindFlags(flags, true);

    STE_TextPos pos = editor->GetCurrentPos();
    if ((eventType == wxEVT_COMMAND_FIND) && STE_HASBIT(flags, STE_FR_WHOLEDOC))
        pos = -1;

    // we have to move cursor to start of word if last backwards search suceeded
    //   note cmp is ok since regexp doesn't handle searching backwards
    if ((eventType == wxEVT_COMMAND_FIND_NEXT) && !STE_HASBIT(flags, wxFR_DOWN))
    {
        if ((labs(editor->GetSelectionEnd() - editor->GetSelectionStart()) == long(findString.Length()))
            && (editor->GetFindReplaceData()->StringCmp(findString, editor->GetSelectedText(), flags)))
                pos -= (STE_TextPos)findString.Length() + 1; // doesn't matter if it matches or not, skip it
    }

    if ((eventType == wxEVT_COMMAND_FIND) || (eventType == wxEVT_COMMAND_FIND_NEXT))
    {
        if (STE_HASBIT(flags, STE_FR_FINDALL|STE_FR_BOOKMARKALL))
        {
            // sum up all of the find strings in all editors
            int n, count = (int)GetPageCount();

            for (n = 0; n < count; n++)
            {
                wxSTEditor* e = GetEditor(n);
                if (e)
                    e->HandleFindDialogEvent(event);
            }
        }
        else
        {
            if ((eventType == wxEVT_COMMAND_FIND) && STE_HASBIT(flags, STE_FR_WHOLEDOC))
                pos = 0;

            pos = FindString(findString, pos, flags, STE_FINDSTRING_SELECT|STE_FINDSTRING_GOTO);

            if (pos >= 0)
            {
                //editor->SetFocus();
            }
            else
            {
                wxBell(); // bell ok to signify no more occurances?
            }
        }
    }
    else if (eventType == wxEVT_COMMAND_FIND_REPLACE)
    {
        if (!editor->SelectionIsFindString(findString, flags))
        {
            wxBell();
            return;
        }

        STE_TextPos pos = editor->GetSelectionStart();
        wxString replaceString(event.GetReplaceString());
        editor->ReplaceSelection(replaceString);
        editor->EnsureCaretVisible();
        editor->SetSelection(pos, pos + (STE_TextPos)replaceString.Length());
        editor->UpdateCanDo(true);
        //editor->SetFocus();
    }
    else if (eventType == wxEVT_COMMAND_FIND_REPLACE_ALL)
    {
        wxString replaceString(event.GetReplaceString());
        if (editor->GetFindReplaceData()->StringCmp(findString, replaceString, flags))
            return;

        wxBusyCursor busy;

        int pages = 0;
        int count = ReplaceAllStrings(findString, replaceString, flags, &pages);

        wxString msg( wxString::Format(_("Replaced %d occurances of\n'%s' with '%s'\nin %d documents."),
                                       count, findString.wx_str(), replaceString.wx_str(), pages) );

        wxMessageBox( msg, _("Finished replacing"),
                      wxOK|wxICON_INFORMATION|wxSTAY_ON_TOP,
                      wxGetTopLevelParent(this) ); // make it be on top in GTK
                      //wxDynamicCast(event.GetEventObject(), wxDialog));
    }
    else if (eventType == wxEVT_COMMAND_FIND_CLOSE)
    {
        //if (wxDynamicCast(event.GetEventObject(), wxDialog))
        //    ((wxDialog*)event.GetEventObject())->Destroy();
    }
}

int wxSTEditorNotebook::FindString(const wxString &str, STE_TextPos start_pos,
                                   int flags, int action)
{
    int n_pages = (int)GetPageCount();
    int n_sel = GetSelection();
    int n = -1;
    STE_TextPos pos = start_pos;
    bool forward = STE_HASBIT(flags, wxFR_DOWN) != 0;
    int noteb_flags = flags & (~STE_FR_WRAPAROUND); // switch to new page

    wxSTEditor *editor = NULL;
    if (n_sel < 0) return wxNOT_FOUND; // oops

    // search this page and later or before to end
    for (n = n_sel;
         forward ? n < n_pages : n >= 0;
         n = forward ? n+1 : n-1)
    {
        editor = GetEditor(n);
        if (!editor)
            continue;

        if (n != n_sel)
            pos = forward ? 0 : editor->GetLength();

        pos = editor->FindString(str, pos, -1, noteb_flags, action);

        if (pos != wxNOT_FOUND)
        {
            SetSelection(n);
            editor->UpdateCanDo(true); // make sure CanFind is updated
            return pos;
        }
    }

    // search through remaining pages
    for (n = forward ? 0 : n_pages-1;
         forward ? n < n_sel : n > n_sel;
         n = forward ? n+1 : n-1)
    {
        editor = GetEditor(n);
        if (!editor)
            continue;

        pos = forward ? 0 : editor->GetLength();

        pos = editor->FindString(str, pos, -1, noteb_flags, action);

        if (pos != wxNOT_FOUND)
        {
            SetSelection(n);
            editor->UpdateCanDo(true); // make sure CanFind is updated
            return pos;
        }
    }

    // if we haven't found the string then try to wrap around on this doc.
    editor = GetEditor(n_sel);
    if ((editor != NULL) && STE_HASBIT(flags, STE_FR_WRAPAROUND))
    {
        pos = editor->FindString(str, start_pos, -1, flags, action);
        editor->UpdateCanDo(true); // make sure CanFind is updated
        return pos;
    }

    return wxNOT_FOUND;
}

int wxSTEditorNotebook::ReplaceAllStrings(const wxString &findString,
                                          const wxString &replaceString,
                                          int flags, int *pages_)
{
    flags &= (~STE_FR_WRAPAROUND); // switch to new page

    if (findString.IsEmpty() || (findString == replaceString))
    {
        if (pages_) *pages_ = 0;
        return 0;
    }

    int count = 0, pages = 0;
    int n_pages = (int)GetPageCount();
    for (int n = 0; n < n_pages; n++)
    {
        wxSTEditor *editor = GetEditor(n);
        if (editor)
        {
            int c = editor->ReplaceAllStrings(findString, replaceString, flags);
            editor->UpdateCanDo(true);
            count += c;
            if (c > 0) pages++;
        }
    }

    if (pages_) *pages_ = pages;

    return count;
}
