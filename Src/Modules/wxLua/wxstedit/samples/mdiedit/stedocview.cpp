///////////////////////////////////////////////////////////////////////////////
// Name:        stedocview.cpp
// Purpose:     MDI wxSTEditor app
// Author:      Troels K
// Modified by:
// Created:     2012-01-19
// Copyright:   (c) Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "wx/stedit/stedit.h"
#include "../stedit/stedoc.h"
#include "stedocview.h"
#include "../../src/wxext.h"

IMPLEMENT_DYNAMIC_CLASS(EditorDoc, wxSTEditorDoc)
IMPLEMENT_DYNAMIC_CLASS(EditorView, wxView)
IMPLEMENT_DYNAMIC_CLASS(EditorChildFrame,wxDocMDIChildFrame)
IMPLEMENT_CLASS(EditorDocTemplate, wxDocTemplate)

/*static*/ EditorDocTemplate* EditorDocTemplate::ms_instance = NULL;

EditorDocTemplate::EditorDocTemplate(wxDocManager* docManager, wxClassInfo* frameClassInfo) :
    wxDocTemplate(docManager, _("Text"), wxT("*.txt;*.text;*.h;*.c;*.cpp"),
      wxT(""), wxT("txt"), wxT("Editor doc"), wxT("Editor view"),
          CLASSINFO(EditorDoc), CLASSINFO(EditorView)),
    m_frameClassInfo(frameClassInfo), m_steOptions(STE_DEFAULT_OPTIONS)
{
    ms_instance = this;

    m_steOptions.GetMenuManager()->SetMenuItems(STE_MENU_FILE_MENU, 0);
    m_steOptions.GetMenuManager()->SetMenuItems(STE_MENU_HELP_MENU, 0);
}

/*static*/ EditorDocTemplate* EditorDocTemplate::Create(wxDocManager* docManager)
{
   return new EditorDocTemplate(docManager, CLASSINFO(EditorChildFrame));
}

wxFrame* EditorDocTemplate::CreateViewFrame(wxView* view)
{
    EditorChildFrame* subframe = wxStaticCast(m_frameClassInfo->CreateObject(), EditorChildFrame);

    if (subframe->Create(view, wxStaticCast(wxTheApp->GetTopWindow(), wxMDIParentFrame)))
    {
    }
    else
    {
        wxDELETE(subframe);
    }
    return subframe;
}

EditorChildFrame::EditorChildFrame() : wxDocMDIChildFrame()
{
    m_steSplitter      = NULL;
    m_mainSplitter     = NULL;
    m_sideSplitter     = NULL;
    m_sideSplitterWin1 = NULL;
    m_sideSplitterWin2 = NULL;
}

EditorChildFrame::~EditorChildFrame()
{
}

bool EditorChildFrame::Create(wxView* view, wxMDIParentFrame* frame)
{
    bool ok = wxDocMDIChildFrame::Create(view->GetDocument(), view, frame, wxID_ANY, wxEmptyString);

    if (ok)
    {
    wxConfigBase *config = wxConfig::Get();
    EditorDoc* doc = wxStaticCast(view->GetDocument(), EditorDoc);
    wxSTEditorOptions& options = doc->GetOptions();
    wxSTEditorMenuManager *steMM = options.GetMenuManager();

    if (steMM)
    {
        steMM->InitAcceleratorArray();
    }

    if (steMM && options.HasFrameOption(STF_CREATE_MENUBAR))
    {
        wxMenuBar *menuBar = GetMenuBar() ? GetMenuBar() : new wxMenuBar(wxMB_DOCKABLE);

        wxMenu* menu = new wxMenu();
        menu->Append(wxID_NEW);
        menu->Append(wxID_OPEN, wxGetStockLabelEx(wxID_OPEN));
        menu->Append(wxID_CLOSE, wxGetStockLabel(wxID_CLOSE) + wxT("\t") + _("Ctrl+W"));
        menu->Append(wxID_SAVE);
        menu->Append(wxID_SAVEAS, wxGetStockLabelEx(wxID_SAVEAS) + wxT("\t") + _("Ctrl+Shift+S"));
        menu->Append(wxID_REVERT, _("Re&vert..."));
        menu->AppendSeparator();
        menu->Append(ID_STE_PROPERTIES, wxGetStockLabelEx(wxID_PROPERTIES) + wxT("\t") + _("Alt+Enter"));
        menu->AppendSeparator();
        menu->Append(wxID_PRINT, wxGetStockLabelEx(wxID_PRINT) + wxT("\t") + _("Ctrl+P"));
        menu->Append(wxID_PRINT_SETUP, _("Print Set&up..."));
        menu->Append(wxID_PREVIEW, wxGetStockLabelEx(wxID_PREVIEW) + wxT("\t") + _("Ctrl+Shift+P"));
        menu->AppendSeparator();
        menu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT) + wxT("\t") + _("Ctrl+Q"));
        menuBar->Append(menu, wxGetStockLabel(wxID_FILE));

        steMM->CreateMenuBar(menuBar, true);

        menu = new wxMenu();
        menu->Append(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT) + wxT("\t") + _("Shift+F1"));
        menuBar->Append(menu, wxGetStockLabel(wxID_HELP));

        if (menuBar)
        {
            SetMenuBar(menuBar);
            wxAcceleratorHelper::SetAcceleratorTable(this, *steMM->GetAcceleratorArray());
            wxAcceleratorHelper::SetAccelText(menuBar, *steMM->GetAcceleratorArray());

            options.SetMenuBar(menuBar);
        }
    }
    if (steMM && options.HasFrameOption(STF_CREATE_TOOLBAR))
    {
        wxToolBar* toolBar = (GetToolBar() != NULL) ? GetToolBar() : CreateToolBar();
        steMM->CreateToolBar(toolBar);
        options.SetToolBar(toolBar);
    }
/*
    if ((GetStatusBar() == NULL) && GetOptions().HasFrameOption(STF_CREATE_STATUSBAR))
    {
        CreateStatusBar(1);
        GetOptions().SetStatusBar(GetStatusBar());
    }
*/
    if (options.HasFrameOption(STF_CREATE_STATUSBAR))
    {
        options.SetStatusBar(GetMDIParent()->GetStatusBar());
    }
    if (steMM)
    {
        if (options.HasEditorOption(STE_CREATE_POPUPMENU))
        {
            wxMenu* menu = steMM->CreateEditorPopupMenu();

            wxAcceleratorHelper::SetAccelText(menu, *steMM->GetAcceleratorArray());
            options.SetEditorPopupMenu(menu, false);
        }
        if (options.HasSplitterOption(STS_CREATE_POPUPMENU))
            options.SetSplitterPopupMenu(steMM->CreateSplitterPopupMenu(), false);
        if (options.HasNotebookOption(STN_CREATE_POPUPMENU))
            options.SetNotebookPopupMenu(steMM->CreateNotebookPopupMenu(), false);
    }

    /*
    m_mainSplitter = new wxSplitterWindow(m_sideSplitter ? (wxWindow*)m_sideSplitter : (wxWindow*)this, ID_STF_MAIN_SPLITTER);
    m_mainSplitter->SetMinimumPaneSize(10);

    m_steSplitter = new wxSTEditorSplitter(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    m_steSplitter->CreateOptions(options);
    m_mainSplitter->Initialize(m_steSplitter);
    */

    if (options.HasFrameOption(STF_CREATE_SIDEBAR) && GetSideSplitter() && m_sideSplitterWin1 && m_sideSplitterWin2)
    {
        GetSideSplitter()->SplitVertically(m_sideSplitterWin1, m_sideSplitterWin2, 100);
    }

    if (options.HasConfigOption(STE_CONFIG_FINDREPLACE) && config)
    {
        if (options.GetFindReplaceData() &&
            !options.GetFindReplaceData()->HasLoadedConfig())
            options.GetFindReplaceData()->LoadConfig(*config);
    }

    // if we've got an editor let it update gui
    wxSTEditor *editor = GetEditor();
    if (editor)
        editor->UpdateAllItems();

/*
        wxMenuBar* menuBar = new wxMenuBar();
        wxMenu* menu;

        menu = new wxMenu();
        menu->Append(wxID_NEW);
        menu->Append(wxID_OPEN);
        menu->Append(wxID_CLOSE, wxGetStockLabel(wxID_CLOSE) + wxT("\t") + _("Ctrl+W"));
        menu->Append(wxID_SAVE);
        menu->Append(wxID_SAVEAS, wxGetStockLabelEx(wxID_SAVEAS) + wxT("\t") + _("Ctrl+Shift+S"));
        menu->Append(wxID_REVERT, _("Re&vert..."));
        menu->AppendSeparator();
        menu->Append(ID_STE_PROPERTIES, wxGetStockLabelEx(wxID_PROPERTIES) + wxT("\t") + _("Alt+Enter"));
        menu->AppendSeparator();
        menu->Append(wxID_PRINT);
        menu->Append(wxID_PRINT_SETUP, _("Print Set&up..."));
        menu->Append(wxID_PREVIEW, wxGetStockLabelEx(wxID_PREVIEW) + wxT("\t") + _("Ctrl+Shift+P"));
        menu->AppendSeparator();
        menu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT) + wxT("\t") + _("Ctrl+Q"));
        menuBar->Append(menu, wxGetStockLabel(wxID_FILE));

        menu = new wxMenu();
        menu->Append(wxID_COPY);
        menu->Append(wxID_PASTE);
        menu->Append(wxID_SELECTALL, wxGetStockLabel(wxID_SELECTALL) + wxT("\t") + _("Ctrl+A"));
        menuBar->Append(menu, wxGetStockLabel(wxID_EDIT));

        menu = new wxMenu();
        menu->Append(ID_STE_SHOW_FULLSCREEN, wxString(_("&Fullscreen")) + wxT("\t") + _("F11"), wxEmptyString, wxITEM_CHECK);
        menuBar->Append(menu, _("&View"));

        menu = new wxMenu();
        menu->Append(ID_STE_PREFERENCES, wxGetStockLabel(wxID_PREFERENCES) + wxT("\t") + _("Ctrl+F9"));
        menuBar->Append(menu, wxGetStockLabel(wxID_PREFERENCES));

        menu = new wxMenu();
        menu->Append(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT) + wxT("\t") + _("Shift+F1"));
        menuBar->Append(menu, wxGetStockLabel(wxID_HELP));

        SetMenuBar(menuBar);
*/
    }
    return ok;
}

// ----------------------------------------------------------------------------
// EditorDoc
// ----------------------------------------------------------------------------

EditorDoc::EditorDoc() : wxSTEditorDoc(false)
{
}

EditorDoc::~EditorDoc()
{
}

bool EditorDoc::OnCreate(const wxString& path, long flags)
{
    m_options = wxStaticCast(GetDocumentTemplate(), EditorDocTemplate)->m_steOptions;
    m_stePrefs = m_options.GetEditorPrefs();
    m_steStyles = m_options.GetEditorStyles();
    m_steLangs = m_options.GetEditorLangs();

    if ( !wxSTEditorDoc::OnCreate(path, flags) )
        return false;

    return true;
}

bool EditorDoc::OnNewDocument()
{
    bool ok = wxSTEditorDoc::OnNewDocument();

    if (ok)
    {
    }
    return ok;
}

bool EditorDoc::DoOpenDocument(const wxString& filename)
{
    wxString str;
    wxFileInputStream stream(filename);
    bool ok = stream.IsOk() && LoadFileToString(&str, stream, wxFileName(filename));

    if (ok)
    {
        GetTextCtrl()->SetTextAndInitialize(str);
    }
    return ok;
}

bool EditorDoc::DoSaveDocument(const wxString& filename)
{
    wxFileOutputStream stream(filename);

    return stream.IsOk() && GetTextCtrl()->SaveFile(stream);
}

// ----------------------------------------------------------------------------
// EditorDoc implementation
// ----------------------------------------------------------------------------

wxSTEditor* EditorDoc::GetTextCtrl() const
{
    wxView* view = GetFirstView();

    return view ? wxStaticCast(view, EditorView)->GetEditor() : NULL;
}

// ----------------------------------------------------------------------------
// EditorView implementation
// ----------------------------------------------------------------------------

EditorView::EditorView() : wxSTEditorView()
{
}

EditorView::~EditorView()
{
    m_text->AttachRefData(new wxSTEditorRefData());
}

BEGIN_EVENT_TABLE(EditorView, wxSTEditorView)
    EVT_MENU(wxID_ANY, EditorView::OnMenu)
END_EVENT_TABLE()

bool EditorView::OnCreate(wxDocument* doc, long flags)
{
    bool ok = wxSTEditorView::OnCreate(doc, flags);

    if (ok)
    {
        wxFrame* frame = wxStaticCast(doc, EditorDoc)->GetDocumentTemplate()->CreateViewFrame(this);
        wxASSERT(frame == GetFrame());
        wxSTEditor* text = new wxSTEditor();

        ok = text->Create(frame);
        if (ok)
        {
            m_text = text;
            delete m_text->AttachRefData(wxStaticCast(doc, EditorDoc));
            frame->SetIcon(wxICON(text));
            frame->Show();
        }
        else
        {
            delete text;
            delete frame;
            SetFrame(NULL);
        }
    }
    return ok;
}

void EditorView::OnMenu(wxCommandEvent &event)
{
    switch (event.GetId())
    {
#ifdef x__WXDEBUG__
        case ID_STE_PROPERTIES:
            _asm NOP
#endif
        case wxID_NEW:
        case wxID_OPEN:
        case wxID_SAVE:
        case wxID_SAVEAS:
            // leave these to the app and/or wxDocManager
            event.Skip();
            break;
        default:
            if (!m_text->HandleMenuEvent(event))
                event.Skip();
            break;
    }
}

void EditorView::OnChangeFilename()
{
    wxSTEditorView::OnChangeFilename();
}

bool EditorView::OnClose(bool deleteWindow)
{
    if ( !wxSTEditorView::OnClose(deleteWindow) )
        return false;

    Activate(false);

    GetDocument()->GetOptions().SetMenuBar(NULL);
    GetDocument()->GetOptions().SetToolBar(NULL);

    if ( deleteWindow )
    {
        GetFrame()->Destroy();
        SetFrame(NULL);
    }
    return true;
}
