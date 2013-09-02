///////////////////////////////////////////////////////////////////////////////
// Name:        app.h
// Purpose:     MDI wxSTEditor app
// Author:      Troels K
// Modified by:
// Created:     2012-01-19
// Copyright:   (c) Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// Define a new application
class App : public wxApp
{
protected:
    wxLocale m_locale;
public:
    App();

    void OpenDocuments(wxDocManager*);

    // override some wxApp virtual methods
    virtual bool OnInit();
    virtual int OnExit();
    virtual void OnInitCmdLine(wxCmdLineParser&);
    virtual bool OnCmdLineParsed(wxCmdLineParser&);

    class CmdLine
    {
        public:
            FileNameArray m_fileNames;
            enum wxLanguage m_lang;
    } m_cmdLine;

#if (wxVERSION_NUMBER < 2900)
    wxString GetAppDisplayName() const
    {
        return GetAppName();
    }
#endif

private:
    wxDocManager* CreateDocManager();
    wxDocument* CreateNewDefaultDocument();

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(App)

class MainFrame : public wxDocMDIParentFrame
{
public:
    MainFrame() : wxDocMDIParentFrame()
    {
    }

    bool Create(wxDocManager* docManager, const wxString& title);

    virtual bool ProcessEvent(wxEvent&);

protected:
    void OnCloseWindow(wxCloseEvent&);
    void OnFullscreen(wxCommandEvent&);
    void OnUpdateFullscreen(wxUpdateUIEvent&);
    void OnProperties(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
    DECLARE_EVENT_TABLE()
};
