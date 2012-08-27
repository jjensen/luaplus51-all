///////////////////////////////////////////////////////////////////////////////
// Name:        app.cpp
// Purpose:     MDI wxSTEditor app
// Author:      Troels K
// Modified by:
// Created:     2012-01-19
// Copyright:   (c) Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "stedocview.h"
#include "../../src/wxext.h" // FileNameArray
#include "app.h"
#include "wx/stedit/stedit.h"
#include "wxtrunk.h"

#define APP_NAME_SHORT   wxT("mdiedit")
#define APP_NAME_DISPLAY wxT("MDIEdit")

// ----------------------------------------------------------------------------
// App implementation
// ----------------------------------------------------------------------------

IMPLEMENT_APP(App)

BEGIN_EVENT_TABLE(App, wxApp)
END_EVENT_TABLE()

App::App() : wxApp()
{
    m_cmdLine.m_lang = wxLANGUAGE_DEFAULT;
}

bool App::OnInit()
{
    bool ok = wxApp::OnInit();

    if (ok)
    {
        wxLocaleHelper::Init(&m_locale, STE_APPNAME, m_cmdLine.m_lang);
        ::wxInitAllImageHandlers();

        // Fill in the application information fields before creating wxConfig.
        SetVendorName(wxT("wxWidgets"));
        SetAppName(APP_NAME_SHORT);
    #if (wxVERSION_NUMBER >= 2900)
        SetAppDisplayName(APP_NAME_DISPLAY);
    #endif

        // Create a document manager
        wxDocManager* docManager = CreateDocManager();

        // create the main frame window
        MainFrame* frame = new MainFrame();
        
        ok = frame->Create(docManager, GetAppDisplayName());
        if (ok)
        {
            frame->Show();
            OpenDocuments(docManager);
        }
        else
        {
            delete frame;
        }
    }
    return ok;
}

#ifdef _WX_PERSIST_H_
template <typename T>
inline bool wxPersistentRestore(T *obj, const wxString& name)
{
    obj->SetName(name);

    return wxPersistenceManager::Get().Restore(obj);
}
template <typename T>
inline void wxPersistentSave(T *obj, const wxString& name)
{
    obj->SetName(name);

    wxPersistenceManager::Get().Save(obj);
}
#endif

void App::OpenDocuments(wxDocManager* docManager)
{
    wxFileHistory history;
    wxDocument* first_doc = NULL;
    wxConfigBase* config = wxConfig::Get();

    config->SetPath(wxT("/OpenWindows"));
    history.Load(*config);
    config->SetPath(wxT("/"));
    size_t j = 0;

    for (size_t i = 0; i < history.GetCount(); i++)
    {
        wxFileName filename = history.GetHistoryFile(i);
        wxDocument* doc = docManager->CreateDocument(filename.GetFullPath(), wxDOC_SILENT);
        
        if (doc)
        {
            if (first_doc == NULL)
            {
                first_doc = doc;
            }
        #ifdef _WX_PERSIST_H_
            // asserts in wxPersistenceManager::Restore(), "not registered"
            //::wxPersistentRestore(wxStaticCast(doc->GetFirstView()->GetFrame(), wxMDIChildFrame), wxString::Format(wxT("Pos%d"), j));

            // comes up with bad size (0,0)
            //::wxPersistentRegisterAndRestore(wxStaticCast(doc->GetFirstView()->GetFrame(), wxMDIChildFrame), wxString::Format(wxT("Pos%d"), j));
        #endif
            j++;
        }
        else
        {
            // no error message
        }
    }

    // filenames from the commandline
    for (size_t i = 0; i < m_cmdLine.m_fileNames.GetCount(); i++)
    {
        const wxFileName& filename = m_cmdLine.m_fileNames.Item(i);
        wxDocument* doc = docManager->CreateDocument(filename.GetFullPath(), wxDOC_SILENT);

        if (doc)
        {
            if (first_doc == NULL)
            {
                first_doc = doc;
            }
        }
        else
        {
            docManager->OnOpenFileFailure();
            //wxMessageBox(wxString::Format(_("Failed to open %s"), filename.GetFullPath().wx_str()));
        }
    }
    if (first_doc == NULL)
    {
        first_doc = CreateNewDefaultDocument();
    }
    if (first_doc)
    {
        // activate first instead of last
        wxStaticCast(first_doc->GetFirstView()->GetFrame(), wxMDIChildFrame)->Activate();
    }
}

int App::OnExit()
{
    wxDocManager* docManager = wxDocManager::GetDocumentManager();

    docManager->FileHistorySave(*wxConfig::Get());
    delete docManager;
    wxTheClipboard->Flush();
    return wxApp::OnExit();
}

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_PARAM,  wxT_2(""),  wxT_2(""), _("input filename(s)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL|wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, 0 },
};

void App::OnInitCmdLine(wxCmdLineParser& parser)
{
   parser.SetDesc(cmdLineDesc);
   wxApp::OnInitCmdLine(parser);
}

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
   for (size_t i = 0; i < parser.GetParamCount(); i++)
   {
      m_cmdLine.m_fileNames.Add(wxFileName(parser.GetParam(i)));
   }
   return wxApp::OnCmdLineParsed(parser);
}

BEGIN_EVENT_TABLE(MainFrame, wxDocMDIParentFrame)
    EVT_MENU(ID_STE_PROPERTIES, MainFrame::OnProperties)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(ID_STE_SHOW_FULLSCREEN, MainFrame::OnFullscreen)
    EVT_UPDATE_UI(ID_STE_SHOW_FULLSCREEN, MainFrame::OnUpdateFullscreen)
END_EVENT_TABLE()

bool MainFrame::Create(wxDocManager* docManager, const wxString& title)
{
    bool ok = wxDocMDIParentFrame::Create(docManager, NULL, wxID_ANY, title);

    if (ok)
    {
        SetIcon(wxICON(text));

        wxMenu* menu;
        
        menu = new wxMenu();
        menu->Append(wxID_NEW);
        menu->Append(wxID_OPEN);
        menu->AppendSeparator();
        menu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT) + wxT("\t") + _("Ctrl+Q"));

        docManager->FileHistoryUseMenu(menu);
        docManager->FileHistoryLoad(*wxConfig::Get());

        wxMenuBar* menubar = new wxMenuBar();

        menubar->Append(menu, wxGetStockLabel(wxID_FILE));

        menu = new wxMenu();
        menu->Append(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT) + wxT("\t") + _("Shift+F1"));
        menubar->Append(menu, wxGetStockLabel(wxID_HELP));

        SetMenuBar(menubar);
        CreateStatusBar();
    #ifdef _WX_PERSIST_H_
        if (!wxPersistentRegisterAndRestore(this, wxT("Main")))
    #endif
        {
            wxFrame_SetInitialPosition(this);
        }
        this->Connect(wxEVT_CLOSE_WINDOW,
                      wxCloseEventHandler(MainFrame::OnCloseWindow));
    }
    return ok;
}

void MainFrame::OnFullscreen(wxCommandEvent&)
{
    long style = wxFULLSCREEN_ALL;
    
    ShowFullScreen(!IsFullScreen(), style);
}

void MainFrame::OnUpdateFullscreen(wxUpdateUIEvent& event)
{
   event.Check(IsFullScreen());
}

void MainFrame::OnAbout(wxCommandEvent&)
{
    ::wxSTEditorAboutDialog(this);
}

wxView* wxDocument_GetCurrentView(const wxDocument* doc)
{
   wxView* view = doc->GetDocumentManager()->GetCurrentView();

   return (view && (view->GetDocument() == doc)) ? view : doc->GetFirstView();
}

void wxDocument_Info(const wxDocument* doc, wxArrayString* as)
{
   const wxString fmt = wxT("%s:\t%s");
   wxView* active_view = wxDocument_GetCurrentView(doc);
   wxView* view  = active_view ? active_view : doc->GetFirstView();
   wxWindow* frame = view ? view->GetFrame() : NULL;;

   as->Add(wxString::Format(fmt, wxT("Doc class"), doc->GetClassInfo()->GetClassName()));
   if (view)
   {
      as->Add(wxString::Format(fmt, wxT("View class"), view->GetClassInfo()->GetClassName()));
   }
   as->Add(wxString::Format(fmt, wxT("Frame class"), frame ? frame->GetClassInfo()->GetClassName() : wxEmptyString));
   if (frame)
   {
      as->Add(wxString::Format(fmt, wxT("Frame label"), frame->GetLabel().wx_str()));
   }
   as->Add(wxString::Format(fmt, wxT("GetFilename"), doc->GetFilename().wx_str()));
   as->Add(wxString::Format(fmt, wxT("GetTitle"), doc->GetTitle().wx_str()));
   as->Add(wxString::Format(fmt, wxT("GetUserReadableName"), doc->GetUserReadableName().wx_str()));
   as->Add(wxString::Format(fmt, wxT("IsModified"), doc->IsModified() ? _("Yes") : _("No")));
   as->Add(wxString::Format(fmt, wxT("GetDocumentSaved"), doc->GetDocumentSaved() ? _("Yes") : _("No")));
   as->Add(wxString::Format(fmt, wxT("GetDocumentName"), doc->GetDocumentName().wx_str()));
}

void MainFrame::OnProperties(wxCommandEvent&)
{
   const wxDocument* doc = GetDocumentManager()->GetCurrentDocument();
   wxArrayString as;

   ::wxDocument_Info(doc, &as);
   ::wxMessageBox(::wxJoin(as, wxT('\n')), wxMessageBoxCaption, wxOK | wxCENTRE, this);
}

bool MainFrame::ProcessEvent(wxEvent& event)
{
    bool processed = false;//wxMDIParentFrame::ProcessEvent(event); // skip m_docManager->ProcessEvent

    if (!processed)
    {
        processed = wxDocMDIParentFrame::ProcessEvent(event); // try m_docManager->ProcessEvent
    }
    return processed;
}

void MainFrame::OnCloseWindow(wxCloseEvent& event)
{
    wxConfigBase* config = wxConfig::Get();
    wxFileHistory open_windows;
    const wxList& list = m_docManager->GetDocuments();

    config->SetPath(wxT("/OpenWindows"));
    size_t j = 0;
    for (wxList::const_reverse_iterator it = list.rbegin(); it != list.rend(); it++)
    {
        wxDocument* doc = wxStaticCast(*it, wxDocument);

        if (doc->GetDocumentSaved())
        {
            open_windows.AddFileToHistory(doc->GetFilename());
        #ifdef _WX_PERSIST_H_
            // asserts in wxPersistenceManager::Save(), "not registered"
            //wxPersistentSave(wxStaticCast(doc->GetFirstView()->GetFrame(), wxMDIChildFrame), wxString::Format(wxT("Pos%d"), j));
        #endif
            j++;
        }
    }
    open_windows.Save(*config);
    config->SetPath(wxT("/"));

    event.Skip();
}
