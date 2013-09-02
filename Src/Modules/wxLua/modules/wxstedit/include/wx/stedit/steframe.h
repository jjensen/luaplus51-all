///////////////////////////////////////////////////////////////////////////////
// Name:        steframe.h
// Purpose:     wxSTEditorFrame
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file steframe.h
/// @brief wxSTEditorFrame, a wxFrame for an editor or notebook.

#ifndef _STEFRAME_H_
#define _STEFRAME_H_

#include <wx/frame.h>
#include <wx/dnd.h>
#include <wx/treectrl.h>

class WXDLLIMPEXP_FWD_CORE wxFrame;
class WXDLLIMPEXP_FWD_CORE wxSplitterWindow;
class WXDLLIMPEXP_FWD_CORE wxNotebook;
class WXDLLIMPEXP_FWD_CORE wxNotebookEvent;
class WXDLLIMPEXP_FWD_CORE wxMenu;
class WXDLLIMPEXP_FWD_CORE wxKeyEvent;
class WXDLLIMPEXP_FWD_CORE wxToolBar;
class WXDLLIMPEXP_FWD_BASE wxConfigBase;
class WXDLLIMPEXP_FWD_CORE wxFileHistory;
class WXDLLIMPEXP_FWD_CORE wxGenericDirCtrl;

#include "wx/stedit/stedefs.h"

//---------------------------------------------------------------------------
/** @class wxSTEditorFrame
    @brief A wxFrame to contain either a single editor or editors in a wxSTEditorNotebook.

    A wxFrame to contain either a single editor or editors in a wxSTEditorNotebook.
    You can optionally have a splitter that will contain a wxNotebook in the
    left panel with a wxListBox containing a list of the pages in the notebook.
    See the CreateOptions function.

    In order to make this as generic as possible all requests for windows call
    the virtual functions GetEditor, GetEditorSplitter, GetEditorNotebook,
    GetSideSplitter etc. You can also just create the windows any way you like
    and set the member pointers appropriately, even to NULL.
    This way you can still allow HandleMenuEvent to do some work for you,
    but you can make a more complicated window layout
    so long as you return the appropriate windows. The functions to get the
    windows can return NULL and any action from the HandleMenuEvent function
    will be silently ignored.

@verbatim
  |--------------------------------------------------------|
  | wxSTEditorFrame, menu & toolbar                        |
  |--------------------------------------------------------|
  |                |                                       |
  |                |         wxSTEditorNotebook            |
  |                |                or                     |
  |  SideNotebook  |         wxSTEditorSplitter            |
  |                |                                       |
  |                |---------------------------------------| < MainSplitter
  |                |    Some user added window (unused)    |
  |--------------------------------------------------------|
                   ^
              SideSplitter
@endverbatim

*/ //------------------------------------------------------------------------

class WXDLLIMPEXP_STEDIT wxSTEditorFrame : public wxFrame
{
public:
    /// Default constructor, call Create(...) to actually make the frame.
    wxSTEditorFrame() : wxFrame() { Init(); }
    /// Create a wxSTEditorFrame.
    wxSTEditorFrame(wxWindow *parent, wxWindowID id = wxID_ANY,
                    const wxString& title = STE_APPDISPLAYNAME,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxDEFAULT_FRAME_STYLE,
                    const wxString& name = wxT("wxSTEditorFrame")) : wxFrame()
    {
        Init();
        Create(parent, id, title, pos, size, style, name);
    }

    virtual ~wxSTEditorFrame();

    virtual bool Destroy(); ///< overrides wxFrame::Destroy()

    /// Create a wxSTEditorFrame, call only if you've used the default constructor.
    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
                const wxString& title = STE_APPDISPLAYNAME,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxT("wxSTEditorFrame"));

    // ----------------------------------------------------------------------
    /// @name Create the frame from the wxSTEditorOptions
    ///  @{

    /// Create and set the wxSTEditorOptions, call this after creation
    ///  or just create the child windows yourself.
    virtual void CreateOptions(const wxSTEditorOptions& options);
    /// GetOptions, use this to change editor option values
    const wxSTEditorOptions& GetOptions() const { return m_options; }
          wxSTEditorOptions& GetOptions()       { return m_options; }
    /// Set the options, the options will now be refed copies of the ones you set.
    /// Call this to detach the options for a particular frame from the others.
    void SetOptions(const wxSTEditorOptions& options) { m_options = options; }

    /// @}

    /// Enable/disable sending wxSTEditor events from children editors
    void SetSendSTEEvents(bool send);

    // ----------------------------------------------------------------------
    /// @name Get child windows
    /// @{

    /// Get either the single editor for STF_SINGLEPAGE or
    /// the editor at page, if page = -1 get the current editor.
    /// Override this in your frame for more complex scenarios where you may have
    /// different or more layers of windows.
    virtual wxSTEditor *GetEditor(int page = -1) const;
    /// Get either the single editor splitter for STF_SINGLEPAGE or
    /// the editor splitter at page, if page = -1 get the current editor splitter.
    /// Override this in your frame for more complex scenarios where you may have
    /// different or more layers of windows.
    virtual wxSTEditorSplitter *GetEditorSplitter(int page = -1) const;
    /// Get the notebook containing the editor, returns NULL if style STF_SINGLEPAGE.
    virtual wxSTEditorNotebook *GetEditorNotebook() const { return m_steNotebook; }

    /// Get the horizontal splitter between editor (notebook) and some user set window.
    virtual wxSplitterWindow* GetMainSplitter() const { return m_mainSplitter; }

    /// Split or unsplit the GetSideSplitter().
    void ShowSidebar(bool show_left_side);
    /// Get the vertical splitter between sidebar notebook and editors, NULL if not style STF_SIDEBAR.
    virtual wxSplitterWindow* GetSideSplitter() const { return m_sideSplitter; }
    /// Get the sidebar notebook, NULL if not style STF_SIDEBAR.
    virtual wxNotebook* GetSideNotebook() const { return m_sideNotebook; }
    /// Get the file treectrl in the sidebar notebook, NULL if not style STF_SIDEBAR.
    virtual wxSTEditorTreeCtrl* GetFileTreeCtrl() const { return m_steTreeCtrl; }
    /// Get the file treectrl in the sidebar notebook, NULL if not style STF_SIDEBAR.
    virtual wxGenericDirCtrl* GetDirCtrl() const { return m_dirCtrl; }

    /// @}

    /// Load a file into either the notebook or single editor.
    /// Returns success and optionally shows a wxMessageDialog with a error message on failure.
    bool LoadFile(const wxFileName& fileName, bool show_error_dialog_on_error);

    /// Update all the menu/tool items in the wxSTEditorOptions.
    virtual void UpdateAllItems();
    /// Update popupmenu, menubar, toolbar if any.
    virtual void UpdateItems(wxMenu *menu=NULL, wxMenuBar *menuBar=NULL, wxToolBar *toolBar=NULL);

    /// Get the base of the title, default is the title set in constructor, @sa MakeTitle().
    wxString GetTitleBase() const { return m_titleBase; }
    /// Set the base of the title, default is the title set in constructor, @sa MakeTitle().
    void SetTitleBase( const wxString& titleBase ) { m_titleBase = titleBase; }
    /// Title is "GetTitleBase() - [*] editor fileName", * used to denote modified.
    wxString MakeTitle(const wxSTEditor*) const;

    // ----------------------------------------------------------------------
    /// @name wxConfig saving and loading
    /// @{
    /// Get wxConfigBase::Get(false), override to return a custom wxConfig.
    virtual wxConfigBase* GetConfigBase();
    /// Load the config for showing the sidebar and frame size.
    ///   @see wxSTEditorOptions for paths and internal saving config.
    void LoadConfig( wxConfigBase &config,
                     const wxString &configPath = wxT("/wxSTEditor/Frame") );
    /// Save the config for showing the sidebar and the frame size.
    ///   @see wxSTEditorOptions for paths and internal saving config.
    void SaveConfig( wxConfigBase &config,
                     const wxString &configPath = wxT("/wxSTEditor/Frame") );
    /// @}

    // -----------------------------------------------------------------------
    /// @name implementation
    /// @{

    void OnNotebookPageChanged(wxNotebookEvent &event);
    void OnFindAllResults(wxCommandEvent& event);
    void OnDirCtrlItemActivation(wxTreeEvent &event);

    void OnSTECreated(wxCommandEvent &event);
    void OnSTEState(wxSTEditorEvent &event);
    void OnSTEPopupMenu(wxSTEditorEvent &event);
    void OnSTCUpdateUI(wxStyledTextEvent &event);

    void OnMenuOpen(wxMenuEvent &event);
    void OnMenu(wxCommandEvent &event);
    /// Handle all known menu events, returns true if event was handled.
    virtual bool HandleMenuEvent(wxCommandEvent &event);

    void OnClose(wxCloseEvent &event);

    /// @}

protected:
    wxSTEditorOptions m_options;
    wxString          m_titleBase;

    wxSplitterWindow   *m_sideSplitter;     ///< Vertical splitter for editor and main splitter.
    wxWindow           *m_sideSplitterWin1; ///< Left page of the side splitter, may be NULL.
    wxWindow           *m_sideSplitterWin2; ///< Right page of the side splitter.
    int                 m_sideSplitter_pos;

    wxNotebook         *m_sideNotebook;     ///< Left page of the side splitter (if created) to hold the wxSTEditorTreeCtrl, may be NULL.
    wxSTEditorTreeCtrl *m_steTreeCtrl;      ///< Child of the m_sideNotebook that displays the notebook pages, may be NULL.
    wxGenericDirCtrl   *m_dirCtrl;          ///< Child of the m_sideNotebook to load new files, may be NULL.

    wxSplitterWindow   *m_mainSplitter;     ///< Horizontal splitter for notebook/editor and bottom result notebook.
    wxWindow           *m_mainSplitterWin1; ///< Left page of the main splitter, may be NULL.
    wxWindow           *m_mainSplitterWin2; ///< Right page of the main splitter.
    int                 m_mainSplitter_pos;

    wxSTEditorNotebook *m_steNotebook;      ///< Top page of main splitter, a notebook for editors (not single editor), may be NULL.
    wxSTEditorSplitter *m_steSplitter;      ///< Top page of main splitter, a single editor (not notebook), may be NULL.

    wxNotebook         *m_resultsNotebook;  ///< Bottom page of the main splitter (if created) to hold a wxSTEditorFindResultsEditor, may be NULL.
    wxSTEditorFindResultsEditor* m_findResultsEditor; ///< Editor for results of find all, may be NULL.

    wxSTERecursionGuardFlag m_rGuard_OnMenu;
    wxSTERecursionGuardFlag m_rGuard_HandleMenuEvent;

private:
    void Init();
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(wxSTEditorFrame)
};

//---------------------------------------------------------------------------
/** @class wxSTEditorFileDropTarget
    @brief A wxFileDropTarget for the wxSTEditorFrame, wxSTEditorNotebook, wxSTEditorSplitter, and wxSTEditor.
    Note that by default the wxStyledTextCtrl handles text dropping so we
    do not override it or allow dropping files on it.
*/ //------------------------------------------------------------------------

#if wxUSE_DRAG_AND_DROP

class WXDLLIMPEXP_STEDIT wxSTEditorFileDropTarget : public wxFileDropTarget
{
public:
    /// Create this for any window and it will traverse up the parents 
    /// until a suitable frame, notebook, splitter, editor to 
    /// open the file is found.
    wxSTEditorFileDropTarget(wxWindow *owner) : m_owner(owner) {}

    /// Overrides wxFileDropTarget::OnDropFiles() to open the file either in the
    /// editor notebook or single page splitter depending on what window is found.
    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);

    wxWindow *m_owner;
};

#endif //wxUSE_DRAG_AND_DROP

#endif  // _STEFRAME_H_
