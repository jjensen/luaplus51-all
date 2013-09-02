///////////////////////////////////////////////////////////////////////////////
// Name:        wxstedit.h
// Purpose:     Simple wxSTEditor app
// Author:      John Labenski, Troels K
// Modified by:
// Created:     04/01/98
// Copyright:   (c) John Labenski, Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes <wx/wx.h".
#include <wx/wxprec.h>

#include <wx/app.h>

// ----------------------------------------------------------------------------
// wxStEditApp - the application class
// ----------------------------------------------------------------------------

class wxStEditApp : public wxApp
{
public:
    wxStEditApp();

    // wxApp overridden virtual functions
    virtual void OnInitCmdLine(wxCmdLineParser& parser);   // wxApp::OnInit() calls these
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
    virtual bool OnInit();
    virtual int  OnExit();

protected:
    void CreateShell();
    wxSTEditorFrame* CreateMainFrame();
    wxFrame* CreateHelpFrame(const wxString& caption, const char* text);
    void OnMenuEvent(wxCommandEvent& event);
    void OnSTEShellEvent(wxSTEditorEvent& event);

protected:
    wxSTEditorFrame* m_frame;

    wxSTEditorOptions m_steOptions;
    wxArrayFileName  m_fileNames;      // command line filenames
    bool m_recurse_dirs;               // command line option to recurse dirs
    enum wxLanguage m_lang;            // command line language
    wxLocale m_locale;
};
