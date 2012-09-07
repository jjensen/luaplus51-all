///////////////////////////////////////////////////////////////////////////////
// Name:        stefindr.h
// Purpose:     wxSTEditorFindReplaceData
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file stefindr.h
/// @brief Find/replace functions and dialog.

#ifndef _STEFINDR_H_
#define _STEFINDR_H_

#include <wx/fdrepdlg.h>

#include "wx/stedit/stedefs.h"
#include "wx/stedit/steopts.h"
#include "wx/stedit/stedit.h"

class WXDLLIMPEXP_FWD_BASE wxConfigBase;
class WXDLLIMPEXP_FWD_CORE wxRadioButton;
class WXDLLIMPEXP_FWD_CORE wxComboBox;

//-----------------------------------------------------------------------------
/// @name Static functions for prepending strings to wxArrayString, wxComboBoxes, wxMenus.
//-----------------------------------------------------------------------------
/// @{

/// Add a string to the array at the top and remove any to keep the max_count.
/// If max_count <= 0 then don't remove any.
WXDLLIMPEXP_STEDIT bool wxSTEPrependArrayString(const wxString &str,
                                                wxArrayString &strArray,
                                                int max_count);

/// Prepend a string to a wxComboBox, removing any copies of it appearing after.
/// If max_strings > 0 then ensure that there are only max_strings in the combo.
WXDLLIMPEXP_STEDIT bool wxSTEPrependComboBoxString(const wxString &str,
                                                   wxComboBox *combo,
                                                   int max_strings);

/// Initialize the combo to have these strings and select first.
WXDLLIMPEXP_STEDIT void wxSTEInitComboBoxStrings(const wxArrayString& values,
                                                 wxComboBox* combo);

/// Initialize the menu to have these strings up to max_count number.
WXDLLIMPEXP_STEDIT void wxSTEInitMenuStrings(const wxArrayString& values,
                                             wxMenu* menu,
                                             int start_win_id, int max_count);
/// @}

//-----------------------------------------------------------------------------
/// STEFindReplaceFlags Flags for wxSTEditorFindReplaceData find/replace behavior.
//-----------------------------------------------------------------------------

enum STEFindReplaceFlags
{
    STE_FR_DOWN          = 0x001, ///< wxFR_DOWN       = 1,
    STE_FR_WHOLEWORD     = 0x002, ///< wxFR_WHOLEWORD  = 2,
    STE_FR_MATCHCASE     = 0x004, ///< wxFR_MATCHCASE  = 4

    STE_FR_WORDSTART     = 0x010, ///< find if string is whole or start of word
    STE_FR_WRAPAROUND    = 0x020, ///< wrap around the doc if not found
    STE_FR_REGEXP        = 0x040, ///< use wxSTC regexp
    STE_FR_POSIX         = 0x080, ///< use wxSTC regexp posix () not \(\) to tagged
    STE_FR_FINDALL       = 0x100, ///< Find all occurances in document
    STE_FR_BOOKMARKALL   = 0x200, ///< Bookmark all occurances in document
    // Choose only one of these
    STE_FR_WHOLEDOC      = 0x1000, ///< search the whole doc starting from top
    STE_FR_FROMCURSOR    = 0x2000, ///< search starting at cursor
    STE_FR_ALLDOCS       = 0x4000, ///< for notebook, starts at current page and goes forward

    STE_FR_SEARCH_MASK   = (STE_FR_WHOLEDOC|STE_FR_FROMCURSOR|STE_FR_ALLDOCS) ///< Mask bits of how to search.
};

//-----------------------------------------------------------------------------
/// STEFindReplaceDialogStyles Flags to specify in the wxFindReplaceDialog ctor or Create().
//-----------------------------------------------------------------------------
enum STEFindReplaceDialogStyles
{
    STE_FR_REPLACEDIALOG = 0x001, ///< wxFR_REPLACEDIALOG = 1, replace dialog (otherwise find dialog).
    STE_FR_NOUPDOWN      = 0x002, ///< wxFR_NOUPDOWN      = 2, don't allow changing the search direction.
    STE_FR_NOMATCHCASE   = 0x004, ///< wxFR_NOMATCHCASE   = 4, don't allow case sensitive searching.
    STE_FR_NOWHOLEWORD   = 0x008, ///< wxFR_NOWHOLEWORD   = 8, don't allow whole word searching.
    STE_FR_NOWORDSTART   = 0x010, ///< Don't allow word start searching.
    STE_FR_NOWRAPAROUND  = 0x020, ///< Don't allow wrapping around.
    STE_FR_NOREGEXP      = 0x040, ///< Don't allow regexp searching.
    STE_FR_NOALLDOCS     = 0x080, ///< Don't allow search all docs option.  (for no editor notebook).
    STE_FR_NOFINDALL     = 0x100, ///< Don't allow finding all strings.     (for find results editor)
    STE_FR_NOBOOKMARKALL = 0x200  ///< Don't allow bookmarking all strings. (for find results editor)
};

//-----------------------------------------------------------------------------
/// @class wxSTEditorFindReplaceData
/// @brief Extended find/replace data class.
//-----------------------------------------------------------------------------
class WXDLLIMPEXP_STEDIT wxSTEditorFindReplaceData : public wxFindReplaceData
{
public:
    wxSTEditorFindReplaceData(wxUint32 flags = wxFR_DOWN|STE_FR_WRAPAROUND);

    virtual ~wxSTEditorFindReplaceData() {}

    // These are in wxFindReplaceData
    //int   GetFlags() const
    //void  SetFlags(wxUint32 flags)
    //const wxString& GetFindString() { return m_FindWhat; }
    //const wxString& GetReplaceString() { return m_ReplaceWith; }
    //void  SetFindString(const wxString& str) { m_FindWhat = str; }
    //void  SetReplaceString(const wxString& str) { m_ReplaceWith = str; }

    bool HasFlag(int flag) const { return (GetFlags() & flag) != 0; }

    void SetFlag(wxUint32 flag, bool enable) { SetFlags( STE_SETBIT(GetFlags(), flag, enable) ); }

    /// Convert the STE flags to Scintilla flags.
    static int STEToScintillaFindFlags(int ste_flags);
    /// Convert the Scintilla flags to STE flags.
    static int ScintillaToSTEFindFlags(int sci_flags);

    /// Add find strings at top of list removing old ones if > GetMaxStrings().
    void AddFindString(const wxString& str) { wxSTEPrependArrayString(str, m_findStrings, m_max_strings); }
    /// Add replace strings at top of list removing old ones if > GetMaxStrings().
    void AddReplaceString(const wxString& str) { wxSTEPrependArrayString(str, m_replaceStrings, m_max_strings); }

    const wxArrayString& GetFindStrings()    const { return m_findStrings; }
    const wxArrayString& GetReplaceStrings() const { return m_replaceStrings; }

    /// Get max number of search strings to save.
    int GetMaxStrings() const { return m_max_strings; }
    /// Set max number of search strings to save.
    void SetMaxStrings(int count) { m_max_strings = count; }

    /// Get the find all strings, set from a previous call to find all.
    /// Format is ("%ld|%s|%d|%s", &editor, filename, line#, line text).
    const wxArrayString& GetFindAllStrings() const { return m_findAllStrings; }
          wxArrayString& GetFindAllStrings()       { return m_findAllStrings; }

    /// Create a "find all" string with the information coded into it.
    static wxString CreateFindAllString(const wxString& fileName,
                                        int line_number,      int line_start_pos,
                                        int string_start_pos, int string_length,
                                        const wxString& lineText);
    /// Parse a "find all" string with the information coded into it, returning success.
    static bool ParseFindAllString(const wxString& findAllString,
                                   wxString& fileName,
                                   int& line_number,      int& line_start_pos,
                                   int& string_start_pos, int& string_length,
                                   wxString& lineText);
    /// Goto and select the text in the "find all" string created by CreateFindAllString().
    static bool GotoFindAllString(const wxString& findAllString,
                                  wxSTEditor* editor);

    /// Compare the strings with flags = -1 for internal flags or use own flags.
    /// Only compares the strings with or without case, returns true if the same.
    bool StringCmp(const wxString& a, const wxString& b, int flags = -1) const
    {
        if (flags == -1) flags = GetFlags();
        return ((flags & wxFR_MATCHCASE) != 0) ? (a.Cmp(b) == 0) : (a.CmpNoCase(b) == 0);
    }

    /// Get/Set the size of the dialog.
    /// This size is only set if the user expands it, otherwise it's equal to wxDefaultSize.
    /// Also, it does not size the current dialog and only sets the size
    ///   of the dialog when first created.
    wxSize GetDialogSize() const           { return m_dialogSize; }
    void SetDialogSize(const wxSize& size) { m_dialogSize = size; }

    /// Load/Save config for find flags.
    /// See also wxSTEditorOptions for paths and internal saving config.
    bool LoadConfig(wxConfigBase &config,
                    const wxString &configPath = wxT("/wxSTEditor/FindReplace/"));
    void SaveConfig(wxConfigBase &config,
                    const wxString &configPath = wxT("/wxSTEditor/FindReplace/")) const;

    /// Returns true after calling LoadConfig().
    bool HasLoadedConfig() const      { return m_loaded_config; }
    void SetLoadedConfig(bool loaded) { m_loaded_config = loaded; }

    /// all editors (can and should probably) share the same find/replace data.
    static wxSTEditorFindReplaceData sm_findReplaceData;

protected:
    int           m_max_strings;
    bool          m_loaded_config;
    wxArrayString m_findStrings;
    wxArrayString m_replaceStrings;
    wxArrayString m_findAllStrings;
    wxSize        m_dialogSize;
};

//-----------------------------------------------------------------------------
/// @class wxSTEditorFindResultsEditor
/// @brief A wxTreeCtrl that can display the results of "find all".
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_STEDIT wxSTEditorFindResultsEditor : public wxSTEditor
{
public:
    wxSTEditorFindResultsEditor() : wxSTEditor() { Init(); }

    wxSTEditorFindResultsEditor(wxWindow *parent, wxWindowID winid,
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxDefaultSize,
                                long style = 0,
                                const wxString& name = wxT("wxSTEditorFindResultsEditor"))
    {
        Init();
        Create(parent, winid, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID winid,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxT("wxSTEditorFindResultsEditor"));

    virtual ~wxSTEditorFindResultsEditor();

    virtual void CreateOptions(const wxSTEditorOptions& options);
    /// Given options for an editor, make a copy of them preserving the styles and langs,
    /// but setup the prefs to make sense for displaying find results.
    virtual void CreateOptionsFromEditorOptions(const wxSTEditorOptions& editorOptions);

    void SetResults(const wxSTEditorFindReplaceData& findReplaceData);

    /// Set the window to send the events to, if NULL then send to the parent.
    wxWindow* GetTargetWindow() const     { return m_targetWin; }
    void SetTargetWindow( wxWindow* win ) { m_targetWin = win; }

protected:

    void OnMarginClick(wxStyledTextEvent& event);

    wxSTEditorOptions         m_options;
    wxSTEditorFindReplaceData m_findReplaceData;
    wxArrayInt                m_lineArrayMap;

    wxWindow*                 m_targetWin;

private:
    void Init();
    DECLARE_DYNAMIC_CLASS(wxSTEditorFindResultsEditor)
    DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------------
/// @class wxSTEditorFindReplacePanel
/// @brief Enhanced wxFindReplaceDialog panel.
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_STEDIT wxSTEditorFindReplacePanel : public wxPanel
{
public:
    wxSTEditorFindReplacePanel() : wxPanel() { Init(); }

    wxSTEditorFindReplacePanel(wxWindow *parent, wxWindowID winid,
                               wxSTEditorFindReplaceData *data,
                               const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxDefaultSize,
                               long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                               const wxString& name = wxT("wxSTEditorFindReplacePanel"))
    {
        Init();
        Create(parent, winid, data, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID winid,
                wxSTEditorFindReplaceData *data,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                const wxString& name = wxT("wxSTEditorFindReplacePanel"));

    virtual ~wxSTEditorFindReplacePanel();

    /// @name Get/Set/Create an editor to show the find results in.
    /// @{

    /// Get the global editor to show "find all" string in, may be NULL.
    static wxSTEditorFindResultsEditor* GetFindResultsEditor() { return sm_findResultsEditor; }
    /// Set a global editor to show "find all" strings in.
    /// If no editor is set then this find panel will be expanded and the
    /// results shown there.
    static void SetFindResultsEditor(wxSTEditorFindResultsEditor* findResultsEditor) { sm_findResultsEditor = findResultsEditor; }

    /// @}

    /// Find dialog data access, data should never be NULL.
    wxSTEditorFindReplaceData *GetData() { return m_findReplaceData; }
    void  SetData(wxSTEditorFindReplaceData *data);

    /// Set the window to send the events to, if NULL then send to the parent.
    wxWindow* GetTargetWindow() const;
    void SetTargetWindow( wxWindow* win ) { m_targetWin = win; }

    /// Try to get an editor for this dialog.
    /// It uses the target window and checks if it is a wxSTEditor, wxSTEditorSplitter,
    ///   or wxSTEditorNotebook and gets their editor
    wxSTEditor* GetEditor() const;

    // implementation
    void UpdateButtons();            // enable/disable buttons as appropriate
    void SelectFindString();         // select the find string in the combo
    int  GetFindFlags() const { return m_flags; }
    void UpdateFindFlags();

    // -----------------------------------------------------------------------
    // implementation
    void SendEvent(const wxEventType& evtType);
    void Send(wxFindDialogEvent& event);

    void OnButton(wxCommandEvent& event);
    void OnMenu(wxCommandEvent& event);
    void OnFindComboText(wxCommandEvent &event);
    void OnCheckBox(wxCommandEvent &event);
    void OnActivate(wxActivateEvent &event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnIdle(wxIdleEvent& event);

//protected:
    wxSTEditorFindReplaceData *m_findReplaceData;

    bool m_created;
    bool m_ignore_activation;

    wxWindow *m_targetWin;

    int m_flags;
    wxString m_lastSearch;

    int m_find_insert_pos;
    int m_replace_insert_pos;

    wxComboBox *m_findCombo;
    wxComboBox *m_replaceCombo;

    wxCheckBox *m_wholewordCheckBox;
    wxCheckBox *m_matchcaseCheckBox;
    wxCheckBox *m_backwardsCheckBox;
    wxCheckBox *m_wordstartCheckBox;
    wxCheckBox *m_regexpFindCheckBox;
    wxCheckBox *m_wraparoundCheckBox;
    wxCheckBox *m_findallCheckBox;
    wxCheckBox *m_bookmarkallCheckBox;

    wxRadioButton *m_scopewholeRadioButton;
    wxRadioButton *m_scopecursorRadioButton;
    wxRadioButton *m_scopealldocsRadioButton;

    wxButton *m_findButton;
    wxButton *m_replaceButton;
    wxButton *m_replaceFindButton;
    wxButton *m_replaceAllButton;

    wxMenu   *m_insertMenu;

    wxSTEditorFindResultsEditor *m_resultEditor;

    static wxSTEditorFindResultsEditor *sm_findResultsEditor;

private:
    void Init();
    DECLARE_DYNAMIC_CLASS(wxSTEditorFindReplacePanel)
    DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------------
/// @class wxSTEditorFindReplaceDialog
/// @brief Enhanced wxFindReplaceDialog.
//-----------------------------------------------------------------------------

/// The name of the wxSTEditorFindReplaceDialog is used to search though the
/// wxWidgets wxWindow list to find it so that we don't have to save a
/// pointer to it.
WXDLLIMPEXP_DATA_STEDIT(extern const wxString) wxSTEditorFindReplaceDialogNameStr;

class WXDLLIMPEXP_STEDIT wxSTEditorFindReplaceDialog : public wxDialog
{
public:
    wxSTEditorFindReplaceDialog() : wxDialog() { Init(); }

    wxSTEditorFindReplaceDialog( wxWindow *parent,
                                 wxSTEditorFindReplaceData *data,
                                 const wxString& title,
                                 int style = 0,
                                 const wxString &name = wxSTEditorFindReplaceDialogNameStr)
    {
        Init();
        Create(parent, data, title, style, name);
    }

    bool Create( wxWindow *parent,
                 wxSTEditorFindReplaceData *data,
                 const wxString& title,
                 int style = 0,
                 const wxString &name = wxSTEditorFindReplaceDialogNameStr );

    virtual ~wxSTEditorFindReplaceDialog();

    /// Get/Set values for this dialog.
    wxSTEditorFindReplacePanel* GetFindReplacePanel() const { return m_findReplacePanel; }

    // -----------------------------------------------------------------------
    // implementation
    void OnButton(wxCommandEvent& event);
    void OnSize(wxSizeEvent &event);
    void OnActivate(wxActivateEvent &event);
    void OnCloseWindow(wxCloseEvent& event);

protected:
    wxSTEditorFindReplacePanel *m_findReplacePanel;

private:
    void Init();
    DECLARE_DYNAMIC_CLASS(wxSTEditorFindReplaceDialog)
    DECLARE_EVENT_TABLE()
};

#endif  // _STEFINDR_H_
