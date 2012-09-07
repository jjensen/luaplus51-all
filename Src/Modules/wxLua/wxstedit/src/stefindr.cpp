///////////////////////////////////////////////////////////////////////////////
// Name:        stefindr.cpp
// Purpose:     wxSTEditorFindReplaceData
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// RCS-ID:
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "wx/stedit/stefindr.h"
#include "wx/stedit/stedit.h"
#include "wx/stedit/steart.h"
#include "stedlgs_wdr.h"
#include "wxext.h"

//-----------------------------------------------------------------------------
// Static functions for prepending strings to wxArrayString and wxComboBoxes
//-----------------------------------------------------------------------------

bool wxSTEPrependArrayString(const wxString &str, wxArrayString &strArray, int max_count)
{
    const int idx = strArray.Index(str);
    if (idx == 0)
        return false;
    if (idx != wxNOT_FOUND)
        strArray.RemoveAt(idx);

    strArray.Insert(str, 0);
    if ((max_count > 0) && ((int)strArray.GetCount() > max_count))
        strArray.RemoveAt(max_count, strArray.GetCount()-max_count);

    return true;
}

bool wxSTEPrependComboBoxString(const wxString &str, wxComboBox *combo, int max_strings)
{
    wxCHECK_MSG(combo, false, wxT("Invalid combobox in wxSTEPrependComboBoxString"));

    int pos = combo->FindString(str);
    if (pos == 0)
        return false;
    if (pos != wxNOT_FOUND)
        combo->Delete(pos);

    combo->Insert(str, 0);
    combo->SetSelection(0);

    if (max_strings > 0)
    {
        while ((int)combo->GetCount() > max_strings)
            combo->Delete(combo->GetCount()-1);
    }
    return true;
}

void wxSTEInitComboBoxStrings(const wxArrayString& values, wxComboBox* combo)
{
    wxCHECK_RET(combo, wxT("Invalid combobox in wxSTEInitComboBoxStrings"));

    combo->Clear();

    for (size_t n = 0; n < values.GetCount(); n++)
        combo->Append(values[n]);

    if (combo->GetCount() > 0)
        combo->SetSelection(0);
}

void wxSTEInitMenuStrings(const wxArrayString& values, wxMenu* menu, int start_win_id, int max_count)
{
    wxCHECK_RET(menu, wxT("Invalid wxMenu in wxSTEInitMenuStrings"));

    int value_count = values.GetCount();

    for (int n = 0; n < max_count; n++)
    {
        int win_id = n + start_win_id;
        wxMenuItem* menuItem = menu->FindItem(win_id);

        if (n >= value_count)
        {
            if (menuItem != NULL)
                menu->Remove(win_id);
        }
        else if (menuItem != NULL)
        {
            menuItem->SetItemLabel(values[n]);
        }
        else
        {
            menu->Append(win_id, values[n]);
        }
    }
}

//-----------------------------------------------------------------------------
// wxSTEditorFindReplaceData
//-----------------------------------------------------------------------------

// static
wxSTEditorFindReplaceData wxSTEditorFindReplaceData::sm_findReplaceData(wxFR_DOWN|STE_FR_WRAPAROUND);

wxSTEditorFindReplaceData::wxSTEditorFindReplaceData(wxUint32 flags)
                          :wxFindReplaceData(),
                           m_max_strings(10),
                           m_loaded_config(false),
                           m_dialogSize(wxDefaultSize)
{
    SetFlags(flags);
}

// static
int wxSTEditorFindReplaceData::STEToScintillaFindFlags(int ste_flags)
{
    int sci_flags = 0;
    if (STE_HASBIT(ste_flags, STE_FR_MATCHCASE)) sci_flags |= wxSTC_FIND_MATCHCASE;
    if (STE_HASBIT(ste_flags, STE_FR_WHOLEWORD)) sci_flags |= wxSTC_FIND_WHOLEWORD;
    if (STE_HASBIT(ste_flags, STE_FR_WORDSTART)) sci_flags |= wxSTC_FIND_WORDSTART;
    if (STE_HASBIT(ste_flags, STE_FR_REGEXP   )) sci_flags |= wxSTC_FIND_REGEXP;
    if (STE_HASBIT(ste_flags, STE_FR_POSIX    )) sci_flags |= wxSTC_FIND_POSIX;
    return sci_flags;
}

// static
int wxSTEditorFindReplaceData::ScintillaToSTEFindFlags(int sci_flags)
{
    int ste_flags = 0;
    if (STE_HASBIT(sci_flags, wxSTC_FIND_MATCHCASE)) ste_flags |= STE_FR_MATCHCASE;
    if (STE_HASBIT(sci_flags, wxSTC_FIND_WHOLEWORD)) ste_flags |= STE_FR_WHOLEWORD;
    if (STE_HASBIT(sci_flags, wxSTC_FIND_WORDSTART)) ste_flags |= STE_FR_WORDSTART;
    if (STE_HASBIT(sci_flags, wxSTC_FIND_REGEXP   )) ste_flags |= STE_FR_REGEXP;
    if (STE_HASBIT(sci_flags, wxSTC_FIND_POSIX    )) ste_flags |= STE_FR_POSIX;
    return ste_flags;
}

// static
wxString wxSTEditorFindReplaceData::CreateFindAllString(const wxString& fileName,
                                                        int line_number,      int line_start_pos,
                                                        int string_start_pos, int string_length,
                                                        const wxString& lineText)
{
    return wxString::Format(wxT("%s|%d|%d|%d|%d>"),
                                fileName.wx_str(),
                                line_number, line_start_pos,
                                string_start_pos, string_length) + lineText;
}

// static
bool wxSTEditorFindReplaceData::ParseFindAllString(const wxString& findAllString,
                                                    wxString& fileName,
                                                    int& line_number_,      int& line_start_pos_,
                                                    int& string_start_pos_, int& string_length_,
                                                    wxString& lineText)
{
    wxString s(findAllString);
    long line_number      = 0;
    long line_start_pos   = 0;
    long string_start_pos = 0;
    long string_length    = 0;

    fileName = s.BeforeFirst(wxT('|'));
    s = s.AfterFirst(wxT('|'));

    if (s.BeforeFirst(wxT('|')).ToLong(&line_number))
    {
        line_number_ = (int)line_number;
        s = s.AfterFirst(wxT('|'));
    }
    else
        return false;

    if (s.BeforeFirst(wxT('|')).ToLong(&line_start_pos))
    {
        line_start_pos_ = (int)line_start_pos;
        s = s.AfterFirst(wxT('|'));
    }
    else
        return false;

    if (s.BeforeFirst(wxT('|')).ToLong(&string_start_pos))
    {
        string_start_pos_ = (int)string_start_pos;
        s = s.AfterFirst(wxT('|'));
    }
    else
        return false;

    if (s.BeforeFirst(wxT('>')).ToLong(&string_length))
    {
        string_length_ = (int)string_length;
        s = s.AfterFirst(wxT('>'));
    }
    else
        return false;

    lineText = s;

    return true;
}

// static
bool wxSTEditorFindReplaceData::GotoFindAllString(const wxString& findAllString,
                                                  wxSTEditor* editor)
{
    wxCHECK_MSG(editor, false, wxT("Invalid wxSTEditor to goto line in."));

    wxString fileName;
    int line_number      = 0;
    int line_start_pos   = 0;
    int string_start_pos = 0;
    int string_length    = 0;
    wxString lineText;

    bool ok = wxSTEditorFindReplaceData::ParseFindAllString(findAllString,
                                                            fileName,
                                                            line_number, line_start_pos,
                                                            string_start_pos, string_length,
                                                            lineText);

    // sanity check, maybe just go to the end if the doc if now shorter?
    if (ok && (wxFileName(fileName) == editor->GetFileName()))
    {
        if (string_start_pos+string_length <= editor->GetLength())
        {
            editor->GotoPos(string_start_pos);
            editor->SetSelection(string_start_pos, string_start_pos+string_length);
        }
        else
            editor->GotoPos(editor->GetLength()); // move the cursor, hopefully they'll remember that they changed the file.

        return true; // we at least moved the cursor
    }

    return false;
}

bool wxSTEditorFindReplaceData::LoadConfig(wxConfigBase &config,
                                           const wxString &configPath)
{
    m_loaded_config = true; // maybe it failed, but we tried at least once

    wxString key(wxSTEditorOptions::FixConfigPath(configPath, false));
    long val = 0;

    if (config.Read(key + wxT("/FindFlags"), &val))
    {
        SetFlags(int(val));
        return true;
    }

    return false;
}

void wxSTEditorFindReplaceData::SaveConfig(wxConfigBase &config,
                                           const wxString &configPath) const
{
    wxString key(wxSTEditorOptions::FixConfigPath(configPath, false));
    config.Write(key + wxT("/FindFlags"), GetFlags());
}

//-----------------------------------------------------------------------------
// wxSTEditorFindResultsEditor
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxSTEditorFindResultsEditor, wxSTEditor)

BEGIN_EVENT_TABLE(wxSTEditorFindResultsEditor, wxSTEditor)
    EVT_STC_MARGINCLICK      (wxID_ANY, wxSTEditorFindResultsEditor::OnMarginClick)
    EVT_STEDITOR_MARGINDCLICK(wxID_ANY, wxSTEditorFindResultsEditor::OnMarginClick)
    EVT_STC_DOUBLECLICK      (wxID_ANY, wxSTEditorFindResultsEditor::OnMarginClick)
END_EVENT_TABLE()

void wxSTEditorFindResultsEditor::Init()
{
    m_targetWin = NULL;
}

bool wxSTEditorFindResultsEditor::Create(wxWindow *parent, wxWindowID winid,
                                         const wxPoint& pos, const wxSize& size,
                                         long style, const wxString& name)
{
    if (!wxSTEditor::Create(parent, winid, pos, size, style, name))
        return false;

    SetStyleBits(5); // want to show indicators

    //SetMarginType(STE_MARGIN_NUMBER, wxSTC_MARGIN_NUMBER);
    //SetMarginWidth(STE_MARGIN_NUMBER, TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_9999")));
    //SetMarginSensitive(STE_MARGIN_NUMBER, true); // don't select line

    SetMarginWidth(STE_MARGIN_MARKER, 16);
    SetMarginSensitive(STE_MARGIN_MARKER, true); // don't select line

    //SetMarginWidth(STE_MARGIN_FOLD, 16);
    //SetMarginSensitive(STE_MARGIN_FOLD, true); // don't select line

    // edge colour
    //SetEdgeMode(wxSTC_EDGE_LINE);
    //SetEdgeColumn(7);

    SetReadOnly(true);

    SetLanguage(STE_LANG_NULL);

    return true;
}

wxSTEditorFindResultsEditor::~wxSTEditorFindResultsEditor()
{
    if (wxSTEditorFindReplacePanel::GetFindResultsEditor() == this)
        wxSTEditorFindReplacePanel::SetFindResultsEditor(NULL);
}

void wxSTEditorFindResultsEditor::CreateOptions(const wxSTEditorOptions& options)
{
    wxSTEditor::CreateOptions(options);
}

void wxSTEditorFindResultsEditor::CreateOptionsFromEditorOptions(const wxSTEditorOptions& editorOptions)
{
    wxSTEditorOptions options;

    options.SetEditorStyles(editorOptions.GetEditorStyles());
    options.SetEditorLangs(editorOptions.GetEditorLangs());
    options.SetFindReplaceData(editorOptions.GetFindReplaceData(), true);

    // Nahhh, probaby best to use the simple default menu
    //options.SetEditorOptions(STE_CREATE_POPUPMENU|STE_CREATE_ACCELTABLE);
    //wxSTEditorMenuManager* steMM = new wxSTEditorMenuManager(STE_MENU_READONLY);
    //options.SetMenuManager(steMM, false);

    CreateOptions(options);
}

void wxSTEditorFindResultsEditor::SetResults(const wxSTEditorFindReplaceData& findReplaceData)
{
    m_findReplaceData = findReplaceData;

    const wxArrayString& findAllStrings = m_findReplaceData.GetFindAllStrings();
    size_t n, count = findAllStrings.GetCount();

    m_lineArrayMap.Clear();
    Clear();
    ClearAllIndicators();

    if (count < 1)
    {
        SetReadOnly(false);
        SetText(wxEmptyString);
        SetReadOnly(true);
        return;
    }

    IndicatorSetStyle(wxSTC_INDIC0_MASK, wxSTC_INDIC_ROUNDBOX);
    IndicatorSetForeground(wxSTC_INDIC0_MASK, *wxRED);

    wxSTEditorStyles::GetGlobalEditorStyles().SetEditorStyle( 3, STE_STYLE_STRING, this, false);
    wxSTEditorStyles::GetGlobalEditorStyles().SetEditorStyle( 4, STE_STYLE_NUMBER, this, false);

    wxString fileName;
    int line_number      = 0;
    int line_start_pos   = 0;
    int string_start_pos = 0;
    int string_length    = 0;
    wxString lineText;

    int pos = 0;
    wxString lastFileName;
    wxString str;

    SetReadOnly(false);

    for (n = 0; n < count; n++)
    {
        bool parsed = wxSTEditorFindReplaceData::ParseFindAllString(findAllStrings.Item(n),
                                                                    fileName,
                                                                    line_number, line_start_pos,
                                                                    string_start_pos, string_length,
                                                                    lineText);
        if (!parsed)
            continue;

        if (fileName != lastFileName)
        {
            lastFileName = fileName;

            pos = GetLength();
            SetFoldLevel(LineFromPosition(pos), 0);

            m_lineArrayMap.Add(-1);
            AppendText(fileName + wxT("\n"));
            StartStyling(pos, 31);
            SetStyling(fileName.Length(), 3);
        }

        m_lineArrayMap.Add(n);

        pos = GetLength();
        SetFoldLevel(LineFromPosition(pos), 1);

        wxString lineString(wxString::Format(wxT("%5d"), line_number));
        AppendText(lineString);
        StartStyling(pos, 31);
        SetStyling(lineString.Length(), 4);

        pos = GetLength();
        AppendText(wxT(" : ") + lineText);

        SetIndicator(pos + 3 + (string_start_pos-line_start_pos),
                     string_length,
                     wxSTC_INDIC2_MASK);

    }

    SetReadOnly(true);
    ColouriseDocument();

    //IndicateAllStrings(m_findReplaceData.GetFindString(),
    //                   m_findReplaceData.GetFlags(),
    //                   wxSTC_INDIC0_MASK);

    // Tell our parents that we have new results in case we're hidden
    if (GetLength() > 0)
    {
        wxCommandEvent event(wxEVT_STEFIND_RESULTS_NEED_SHOWN, GetId());
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxSTEditorFindResultsEditor::OnMarginClick( wxStyledTextEvent &event )
{
    //if (!m_created) return; // set after editor is fully created

    if (event.GetEventType() == wxEVT_STEDITOR_MARGINDCLICK)
        return;

    STE_TextPos pos = event.GetPosition();

    if (event.GetEventType() == wxEVT_STC_DOUBLECLICK) // event pos not set correctly
        pos = GetCurrentPos();

    int line = LineFromPosition(pos);

    if (GetLine(line).Strip(wxString::both).IsEmpty())
        return;

    MarkerDeleteAll(STE_MARKER_BOOKMARK);

    if ((line < 0) || (line >= (int)m_lineArrayMap.GetCount()) || (m_lineArrayMap[line] < 0))
        return;

    int findall_index = m_lineArrayMap[line];

    MarkerAdd(line, STE_MARKER_BOOKMARK);

    wxFindDialogEvent findEvent(wxEVT_STEFIND_GOTO, GetId());
    findEvent.SetEventObject(this);
    findEvent.SetFindString(m_findReplaceData.GetFindAllStrings()[findall_index]);
    findEvent.SetFlags(m_findReplaceData.GetFlags());
    findEvent.SetExtraLong(findall_index);
    //Send(findEvent);

    if (m_targetWin)
        m_targetWin->GetEventHandler()->ProcessEvent(findEvent);
    else
        GetParent()->GetEventHandler()->ProcessEvent(findEvent);
}

//-----------------------------------------------------------------------------
// wxSTEditorFindReplacePanel
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxSTEditorFindReplacePanel, wxPanel)

wxSTEditorFindResultsEditor* wxSTEditorFindReplacePanel::sm_findResultsEditor = NULL;

BEGIN_EVENT_TABLE(wxSTEditorFindReplacePanel, wxPanel)
    EVT_TEXT        (ID_STEDLG_FIND_COMBO,    wxSTEditorFindReplacePanel::OnFindComboText)
    EVT_TEXT        (ID_STEDLG_REPLACE_COMBO, wxSTEditorFindReplacePanel::OnFindComboText)
    EVT_CHECKBOX    (wxID_ANY,                wxSTEditorFindReplacePanel::OnCheckBox)
    EVT_RADIOBOX    (wxID_ANY,                wxSTEditorFindReplacePanel::OnCheckBox)
    EVT_RADIOBUTTON (wxID_ANY,                wxSTEditorFindReplacePanel::OnCheckBox)
    EVT_BUTTON      (wxID_ANY,                wxSTEditorFindReplacePanel::OnButton)
    EVT_MENU        (wxID_ANY,                wxSTEditorFindReplacePanel::OnMenu)

#ifdef __WXMSW__
    EVT_IDLE        (wxSTEditorFindReplacePanel::OnIdle)
#endif

    //EVT_ACTIVATE  (wxSTEditorFindReplacePanel::OnActivate)
END_EVENT_TABLE()

wxSTEditorFindReplacePanel::~wxSTEditorFindReplacePanel()
{
    m_findCombo    = NULL;
    m_replaceCombo = NULL;

    delete m_insertMenu;
}

void wxSTEditorFindReplacePanel::Init()
{
    m_created            = false;
    m_ignore_activation  = false;

    m_targetWin          = NULL;

    m_flags              = 0;
    m_findReplaceData    = NULL;

    m_find_insert_pos    = 0;
    m_replace_insert_pos = 0;

    m_findCombo          = NULL;
    m_replaceCombo       = NULL;
    m_wholewordCheckBox  = NULL;
    m_matchcaseCheckBox  = NULL;
    m_backwardsCheckBox  = NULL;
    m_wordstartCheckBox  = NULL;
    m_regexpFindCheckBox = NULL;
    m_wraparoundCheckBox = NULL;
    m_findallCheckBox    = NULL;
    m_bookmarkallCheckBox = NULL;

    m_scopewholeRadioButton   = NULL;
    m_scopecursorRadioButton  = NULL;
    m_scopealldocsRadioButton = NULL;

    m_findButton        = NULL;
    m_replaceButton     = NULL;
    m_replaceFindButton = NULL;
    m_replaceAllButton  = NULL;

    m_insertMenu        = NULL;

    m_resultEditor      = NULL;
}

wxSizer *FindSizerSizer(wxSizer *sizer, wxSizer *topSizer)
{
    wxSizerItemList &sizerList = topSizer->GetChildren();

    for (wxSizerItemList::iterator it = sizerList.begin();
         it != sizerList.end();
         it++)
    {
        wxSizerItem *item = *it;

        if (item->IsSizer())
        {
            if (item->GetSizer() == sizer)
                return topSizer;
            else
            {
                wxSizer *foundSizer = FindSizerSizer(sizer, item->GetSizer());
                if (foundSizer)
                    return foundSizer;
            }
        }
    }
    return NULL;
}

wxSizer *FindSizerWindow(wxWindow *win, wxSizer *topSizer)
{
    wxSizerItemList &sizerList = topSizer->GetChildren();

    for (wxSizerItemList::iterator it = sizerList.begin();
         it != sizerList.end();
         it++)
    {
        wxSizerItem *item = *it;

        if (item->IsWindow() && (item->GetWindow() == win))
            return topSizer;
        else if (item->IsSizer())
        {
            wxSizer *foundSizer = FindSizerWindow(win, item->GetSizer());
            if (foundSizer)
                return foundSizer;
        }
    }
    return NULL;
}

bool wxSTEditorFindReplacePanel::Create(wxWindow *parent, wxWindowID winid,
                                        wxSTEditorFindReplaceData *data,
                                        const wxPoint& pos, const wxSize& size,
                                        long style, const wxString& name)
{
    if ( !wxPanel::Create(parent, winid, pos, size, style, name) )
        return false;

    wxSizer* frSizer = wxSTEditorFindReplaceSizer(this, false, false);

    m_findCombo    = wxStaticCast(FindWindow(ID_STEDLG_FIND_COMBO   ), wxComboBox);
    m_replaceCombo = wxStaticCast(FindWindow(ID_STEDLG_REPLACE_COMBO), wxComboBox);

    m_wholewordCheckBox   = wxStaticCast(FindWindow(ID_STEDLG_WHOLEWORD_CHECKBOX  ), wxCheckBox);
    m_matchcaseCheckBox   = wxStaticCast(FindWindow(ID_STEDLG_MATCHCASE_CHECKBOX  ), wxCheckBox);
    m_backwardsCheckBox   = wxStaticCast(FindWindow(ID_STEDLG_BACKWARDS_CHECKBOX  ), wxCheckBox);
    m_wordstartCheckBox   = wxStaticCast(FindWindow(ID_STEDLG_WORDSTART_CHECKBOX  ), wxCheckBox);
    m_regexpFindCheckBox  = wxStaticCast(FindWindow(ID_STEDLG_REGEXP_FIND_CHECKBOX), wxCheckBox);
    m_wraparoundCheckBox  = wxStaticCast(FindWindow(ID_STEDLG_WRAP_CHECKBOX       ), wxCheckBox);
    m_findallCheckBox     = wxStaticCast(FindWindow(ID_STEDLG_FINDALL_CHECKBOX    ), wxCheckBox);
    m_bookmarkallCheckBox = wxStaticCast(FindWindow(ID_STEDLG_BOOKMARKALL_CHECKBOX), wxCheckBox);

    m_scopewholeRadioButton   = wxStaticCast(FindWindow(ID_STEDLG_SCOPEWHOLE_RADIOBUTTON  ), wxRadioButton);
    m_scopecursorRadioButton  = wxStaticCast(FindWindow(ID_STEDLG_SCOPECURSOR_RADIOBUTTON ), wxRadioButton);
    m_scopealldocsRadioButton = wxStaticCast(FindWindow(ID_STEDLG_SCOPEALLDOCS_RADIOBUTTON), wxRadioButton);

    m_findButton        = wxStaticCast(FindWindow(ID_STEDLG_FIND_BUTTON       ), wxButton);
    m_replaceButton     = wxStaticCast(FindWindow(ID_STEDLG_REPLACE_BUTTON    ), wxButton);
    m_replaceFindButton = wxStaticCast(FindWindow(ID_STEDLG_REPLACEFIND_BUTTON), wxButton);
    m_replaceAllButton  = wxStaticCast(FindWindow(ID_STEDLG_REPLACEALL_BUTTON ), wxButton);

    m_insertMenu = wxSTEditorMenuManager::CreateInsertCharsMenu(NULL,
        STE_MENU_INSERTCHARS_CHARS|STE_MENU_INSERTCHARS_REGEXP);

    if (!data)
    {
        Enable(false);
        return false;
    }

    // Set the data and update the button state based on its values
    SetData(data);

    if (HasFlag(STE_FR_NOUPDOWN))
    {
        m_backwardsCheckBox->SetValue(false);
        FindSizerWindow(m_backwardsCheckBox, frSizer)->Show(m_backwardsCheckBox, false);
    }

    if (HasFlag(STE_FR_NOMATCHCASE))
    {
        m_matchcaseCheckBox->SetValue(true);
        FindSizerWindow(m_matchcaseCheckBox, frSizer)->Show(m_matchcaseCheckBox, false);
    }

    if (HasFlag(STE_FR_NOWHOLEWORD))
    {
        m_wholewordCheckBox->SetValue(false);
        FindSizerWindow(m_wholewordCheckBox, frSizer)->Show(m_wholewordCheckBox, false);
    }

    if (HasFlag(STE_FR_NOWORDSTART))
    {
        m_wordstartCheckBox->SetValue(false);
        FindSizerWindow(m_wordstartCheckBox, frSizer)->Show(m_wordstartCheckBox, false);
    }

    if (HasFlag(STE_FR_NOWRAPAROUND))
    {
        m_wraparoundCheckBox->SetValue(false);
        FindSizerWindow(m_wraparoundCheckBox, frSizer)->Show(m_wraparoundCheckBox, false);
    }

    if (HasFlag(STE_FR_NOREGEXP))
    {
        m_regexpFindCheckBox->SetValue(false);
        FindSizerWindow(m_regexpFindCheckBox, frSizer)->Show(m_regexpFindCheckBox, false);
    }

    if (HasFlag(STE_FR_NOALLDOCS))
    {
        m_scopealldocsRadioButton->Show(false);
        // you can't find in all docs, remove that flag, set find from cursor
        if ( m_findReplaceData->HasFlag(STE_FR_ALLDOCS) ||
            (!m_findReplaceData->HasFlag(STE_FR_WHOLEDOC) &&
             !m_findReplaceData->HasFlag(STE_FR_FROMCURSOR)) )
            m_findReplaceData->SetFlags((m_findReplaceData->GetFlags() & ~STE_FR_SEARCH_MASK) | STE_FR_FROMCURSOR);
    }

    if (HasFlag(STE_FR_NOFINDALL))
    {
        m_findallCheckBox->SetValue(false);
        m_findallCheckBox->Show(false);

    }

    if (HasFlag(STE_FR_NOBOOKMARKALL))
    {
        m_bookmarkallCheckBox->SetValue(false);
        m_bookmarkallCheckBox->Show(false);
    }

    if (!HasFlag(wxFR_REPLACEDIALOG))
    {
        wxSizer *sizer = FindSizerWindow(m_replaceCombo, frSizer);
        if (sizer)
        {
            sizer->Show(FindWindow(ID_STEDLG_REPLACE_TEXT), false);
            sizer->Show(m_replaceCombo, false);
            sizer->Show(FindWindow(ID_STEDLG_REPLACE_BITMAPBUTTON), false);
        }

        wxSizer *replaceSizer = FindSizerWindow(m_replaceButton, frSizer);
        sizer = FindSizerSizer(replaceSizer, frSizer);
        if (sizer)
            sizer->Hide(replaceSizer);
    }

    wxFlexGridSizer *rootSizer = new wxFlexGridSizer( 1, 0, 0 );
    rootSizer->AddGrowableCol( 0 );
    rootSizer->AddGrowableRow( 1 );
    rootSizer->Add(frSizer, 0, wxGROW, 0);

    m_resultEditor = new wxSTEditorFindResultsEditor(this, wxID_ANY);
    m_resultEditor->Show(false);

    rootSizer->Add(m_resultEditor, 1, wxGROW, 0);
    //rootSizer->Show(m_resultEditor, m_findReplaceData->HasFlag(STE_FR_FINDALL));

    SetSizer(rootSizer);
    rootSizer->Layout();
    Layout();
    rootSizer->SetSizeHints( this );
    //rootSizer->Fit( this );

    m_created = true;

    FindWindow(wxID_CANCEL)->SetLabel(wxGetStockLabel(wxID_CLOSE, wxSTOCK_NOFLAGS));
    UpdateFindFlags();
    UpdateButtons();
    m_findCombo->SetFocus();

    return true;
}

void wxSTEditorFindReplacePanel::SetData(wxSTEditorFindReplaceData *data)
{
    wxCHECK_RET(data, wxT("Invalid find replace data in wxSTEditorFindReplaceDialog::SetData"));
    m_findReplaceData = data;

    // setup the find/replace comboboxes
    wxSTEInitComboBoxStrings(m_findReplaceData->GetFindStrings(),    m_findCombo);
    wxSTEInitComboBoxStrings(m_findReplaceData->GetReplaceStrings(), m_replaceCombo);

    // setup the options checkboxes
    int flags = m_findReplaceData->GetFlags();

    m_wholewordCheckBox->SetValue(STE_HASBIT(flags, wxFR_WHOLEWORD));
    m_matchcaseCheckBox->SetValue(STE_HASBIT(flags, wxFR_MATCHCASE));
    m_backwardsCheckBox->SetValue(!STE_HASBIT(flags, wxFR_DOWN));

    m_wordstartCheckBox->SetValue(STE_HASBIT(flags, STE_FR_WORDSTART));
    m_regexpFindCheckBox->SetValue(STE_HASBIT(flags, STE_FR_REGEXP));
    m_wraparoundCheckBox->SetValue(STE_HASBIT(flags, STE_FR_WRAPAROUND));

    m_findallCheckBox->SetValue(STE_HASBIT(flags, STE_FR_FINDALL));
    m_bookmarkallCheckBox->SetValue(STE_HASBIT(flags, STE_FR_BOOKMARKALL));

    // setup the scope radio buttons
    if (STE_HASBIT(flags, STE_FR_FROMCURSOR))
        m_scopecursorRadioButton->SetValue(true);
    else if (STE_HASBIT(flags, STE_FR_ALLDOCS))
        m_scopealldocsRadioButton->SetValue(true);
    else
        m_scopewholeRadioButton->SetValue(true);
}

wxWindow* wxSTEditorFindReplacePanel::GetTargetWindow() const
{
    return m_targetWin ? m_targetWin : GetParent();
}

wxSTEditor* wxSTEditorFindReplacePanel::GetEditor() const
{
    wxWindow* targetWindow = GetTargetWindow();
    wxSTEditor* edit = NULL;

    if (targetWindow)
    {
        if (wxDynamicCast(targetWindow, wxSTEditorNotebook))
            edit = wxDynamicCast(targetWindow, wxSTEditorNotebook)->GetEditor();
        else if (wxDynamicCast(targetWindow, wxSTEditorSplitter))
            edit = wxDynamicCast(targetWindow, wxSTEditorSplitter)->GetEditor();
        else if (wxDynamicCast(targetWindow, wxSTEditor))
            edit = wxDynamicCast(targetWindow, wxSTEditor);
    }

    return edit;
}

void wxSTEditorFindReplacePanel::SendEvent(const wxEventType& evtType)
{
    wxFindDialogEvent event(evtType, GetId());
    event.SetEventObject(this);
    event.SetFindString(m_findCombo->GetValue());
    event.SetFlags(GetFindFlags());
    event.SetExtraLong(-1);

    if (evtType != wxEVT_COMMAND_FIND_CLOSE)
        wxSTEPrependComboBoxString(m_findCombo->GetValue(), m_findCombo, m_findReplaceData->GetMaxStrings());

    if ( HasFlag(wxFR_REPLACEDIALOG) )
    {
        wxSTEPrependComboBoxString(m_replaceCombo->GetValue(), m_replaceCombo, m_findReplaceData->GetMaxStrings());
        event.SetReplaceString(m_replaceCombo->GetValue());
    }

    Send(event);
}

void wxSTEditorFindReplacePanel::Send(wxFindDialogEvent& event)
{
    // we copy the data to dialog->GetData() as well
    m_findReplaceData->SetFlags(event.GetFlags());

    m_findReplaceData->SetFindString(event.GetFindString());
    if (event.GetFindString().Length())
        m_findReplaceData->AddFindString(event.GetFindString());

    if ( HasFlag(wxFR_REPLACEDIALOG) &&
         (event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE ||
          event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE_ALL) )
    {
        m_findReplaceData->SetReplaceString(event.GetReplaceString());
        m_findReplaceData->AddReplaceString(event.GetReplaceString());
    }

    // translate wxEVT_COMMAND_FIND_NEXT to wxEVT_COMMAND_FIND if needed
    if ( event.GetEventType() == wxEVT_COMMAND_FIND_NEXT )
    {
        if ( m_findReplaceData->GetFindString() != m_lastSearch )
        {
            event.SetEventType(wxEVT_COMMAND_FIND);
            m_lastSearch = m_findReplaceData->GetFindString();
        }
    }

    wxSTEditorFindResultsEditor* resultsEditor = GetFindResultsEditor() ? GetFindResultsEditor() : m_resultEditor;

    if (m_findReplaceData->HasFlag(STE_FR_FINDALL) && resultsEditor &&
        ((event.GetEventType() == wxEVT_COMMAND_FIND) ||
         (event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)))
    {
        m_findReplaceData->GetFindAllStrings().Clear();
        resultsEditor->SetResults(*m_findReplaceData);
    }

    wxWindow *target = GetTargetWindow();

    // first send event to ourselves then to the target
    if ( !GetEventHandler()->ProcessEvent(event) && target )
    {
        // the event is not propagated upwards to the parent automatically
        // because the dialog is a top level window, so do it manually as
        // in 9 cases of 10 the message must be processed by the dialog
        // owner and not the dialog itself
        target->GetEventHandler()->ProcessEvent(event);
    }

    if (m_findReplaceData->HasFlag(STE_FR_FINDALL) && resultsEditor &&
        ((event.GetEventType() == wxEVT_COMMAND_FIND) ||
         (event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)))
    {
        resultsEditor->SetTargetWindow(GetTargetWindow());
        resultsEditor->SetResults(*m_findReplaceData);
    }

    wxWindow* focusWin = FindFocus();

    // restore the focus to the text editor, not the find results editor
    if (resultsEditor && (resultsEditor == focusWin) && (GetTargetWindow() != NULL))
    {
        wxSTEditorNotebook* steNotebook = wxDynamicCast(GetTargetWindow(), wxSTEditorNotebook);

        if (steNotebook && steNotebook->GetEditor())
            steNotebook->GetEditor()->SetFocus();
        else
            GetTargetWindow()->SetFocus();
    }

    UpdateButtons();
}

void wxSTEditorFindReplacePanel::OnButton(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case ID_STEDLG_FIND_BITMAPBUTTON  :
        {
            // set the clientdata of the menu to the combo it's for, see OnMenu
            wxRect r = ((wxWindow*)event.GetEventObject())->GetRect();
            m_insertMenu->SetClientData((void*)m_findCombo);
            m_insertMenu->Enable(ID_STEDLG_MENU_INSERTMENURE, m_regexpFindCheckBox->IsChecked());
            PopupMenu(m_insertMenu, r.GetRight(), r.GetTop());
            break;
        }
        case ID_STEDLG_REPLACE_BITMAPBUTTON  :
        {
            wxRect r = ((wxWindow*)event.GetEventObject())->GetRect();
            m_insertMenu->SetClientData((void*)m_replaceCombo);
            m_insertMenu->Enable(ID_STEDLG_MENU_INSERTMENURE, m_regexpFindCheckBox->IsChecked());
            PopupMenu(m_insertMenu, r.GetRight(), r.GetTop());
            break;
        }
        case ID_STEDLG_FIND_BUTTON        : SendEvent(wxEVT_COMMAND_FIND_NEXT); break;
        case ID_STEDLG_REPLACE_BUTTON     : SendEvent(wxEVT_COMMAND_FIND_REPLACE); break;
        case ID_STEDLG_REPLACEFIND_BUTTON : SendEvent(wxEVT_COMMAND_FIND_REPLACE);
                                            SendEvent(wxEVT_COMMAND_FIND_NEXT); break;
        case ID_STEDLG_REPLACEALL_BUTTON  : SendEvent(wxEVT_COMMAND_FIND_REPLACE_ALL); break;
        case wxID_CANCEL                  : SendEvent(wxEVT_COMMAND_FIND_CLOSE); event.Skip(); break;
        default : break;
    }
}

void wxSTEditorFindReplacePanel::OnMenu(wxCommandEvent& event)
{
    wxString c;
    int ipos = 0;

    switch (event.GetId())
    {
        case ID_STEDLG_INSERTMENU_TAB : c = wxT("\t"); break;
        case ID_STEDLG_INSERTMENU_CR  : c = wxT("\r"); break;
        case ID_STEDLG_INSERTMENU_LF  : c = wxT("\n"); break;

        case ID_STEDLG_INSERTMENURE_ANYCHAR   : c = wxT("."); break;
        case ID_STEDLG_INSERTMENURE_RANGE     : c = wxT("[]");  ipos = -1; break;
        case ID_STEDLG_INSERTMENURE_NOTRANGE  : c = wxT("[^]"); ipos = -1; break;
        case ID_STEDLG_INSERTMENURE_BEGINLINE : c = wxT("^"); break;
        case ID_STEDLG_INSERTMENURE_ENDLINE   : c = wxT("$"); break;
        case ID_STEDLG_INSERTMENURE_TAGEXPR   :
        {
            if (!STE_HASBIT(GetFindFlags(), STE_FR_POSIX))
                { c = wxT("\\(\\)");  ipos = -2; }
            else
                { c = wxT("()");  ipos = -1; }
            break;
        }
        case ID_STEDLG_INSERTMENURE_0MATCHES  : c = wxT("*"); break;
        case ID_STEDLG_INSERTMENURE_1MATCHES  : c = wxT("+"); break;
        case ID_STEDLG_INSERTMENURE_01MATCHES : c = wxT("?"); break;

        case ID_STEDLG_INSERTMENURE_ALPHANUM  : c = wxT("[a-zA-Z0-9]"); break;
        case ID_STEDLG_INSERTMENURE_ALPHA     : c = wxT("[a-zA-Z]"); break;
        case ID_STEDLG_INSERTMENURE_NUMERIC   : c = wxT("[0-9]"); break;
        case ID_STEDLG_INSERTMENURE_TAB       : c = wxT("\\t"); break;
        default : break;
    }

    if (c.Length()) // this must have been for the m_insertMenu
    {
        wxComboBox* cBox = wxStaticCast(m_insertMenu->GetClientData(), wxComboBox);
        wxCHECK_RET(cBox, wxT("Unexpected missing control"));
#ifdef __WXMSW__
        // See comment in OnIdle(), MSW forgets insertion point after losing focus
        wxTextPos pos = (cBox == m_findCombo) ? m_find_insert_pos : m_replace_insert_pos;
#else
        wxTextPos pos = cBox->GetInsertionPoint();
#endif

        wxString s = cBox->GetValue();

        if (pos >= int(s.Length()))
            s += c;
        else if (pos == 0)
            s = c + s;
        else
            s = s.Mid(0, pos) + c + s.Mid(pos);

        cBox->SetValue(s);
        cBox->SetFocus();
        cBox->SetInsertionPoint(pos + (int)c.Length() + ipos);
        m_ignore_activation = true;
    }
}

void wxSTEditorFindReplacePanel::OnActivate(wxActivateEvent &event)
{
    event.Skip();

    if (event.GetActive())
    {
        if (!m_ignore_activation)
            SelectFindString();

        UpdateButtons();
    }

    m_ignore_activation = false;
}

void wxSTEditorFindReplacePanel::OnIdle(wxIdleEvent &event)
{
    if (IsShown())
    {
        // This is a really ugly hack because the combo forgets its insertion
        //   point in MSW whenever it loses focus
        wxWindow* focus = FindFocus();
        if (m_findCombo && (focus == m_findCombo))
            m_find_insert_pos = m_findCombo->GetInsertionPoint();
        if (m_replaceCombo && (focus == m_replaceCombo))
            m_replace_insert_pos = m_replaceCombo->GetInsertionPoint();
    }

    event.Skip();
}

void wxSTEditorFindReplacePanel::UpdateFindFlags()
{
    m_flags = 0;

    if (m_matchcaseCheckBox->GetValue())   m_flags |= wxFR_MATCHCASE;
    if (m_wholewordCheckBox->GetValue())   m_flags |= wxFR_WHOLEWORD;
    if (!m_backwardsCheckBox->GetValue())  m_flags |= wxFR_DOWN;

    if (m_wordstartCheckBox->GetValue())   m_flags |= STE_FR_WORDSTART;
    if (m_regexpFindCheckBox->GetValue())  m_flags |= STE_FR_REGEXP;
    if (m_wraparoundCheckBox->GetValue())  m_flags |= STE_FR_WRAPAROUND;

    if (m_findallCheckBox->GetValue())     m_flags |= STE_FR_FINDALL;
    if (m_bookmarkallCheckBox->GetValue()) m_flags |= STE_FR_BOOKMARKALL;

    if (m_scopewholeRadioButton->GetValue())        m_flags |= STE_FR_WHOLEDOC;
    else if (m_scopecursorRadioButton->GetValue())  m_flags |= STE_FR_FROMCURSOR;
    else if (m_scopealldocsRadioButton->GetValue()) m_flags |= STE_FR_ALLDOCS;

    if (!GetFindResultsEditor() && m_resultEditor &&
        (m_resultEditor->IsShown() != STE_HASBIT(m_flags, STE_FR_FINDALL)))
    {
        InvalidateBestSize();
        SetMinSize(wxSize(10, 10));
        GetSizer()->SetMinSize(wxSize(10, 10));
        GetSizer()->Show(m_resultEditor, STE_HASBIT(m_flags, STE_FR_FINDALL));
        GetSizer()->Layout();
        GetSizer()->SetSizeHints(this);
    }
}

void wxSTEditorFindReplacePanel::SelectFindString()
{
    wxString value = m_findCombo->GetValue();
    if (value.Len() > 0u)
        m_findCombo->SetSelection(0, (int)value.Len());
}

void wxSTEditorFindReplacePanel::OnFindComboText(wxCommandEvent& WXUNUSED(event))
{
    UpdateButtons();
}

void wxSTEditorFindReplacePanel::OnCheckBox(wxCommandEvent &event)
{
    UpdateFindFlags();
    UpdateButtons();
    event.Skip();
}

// FIXME - This is a hack for a bug in GTK (not wxWidgets) where if you enable
// a button you cannot click on it without capturing and releasing the mouse.
void wxSTE_WIN_ENABLE(wxWindow* win, bool enable)
{
    if (win && (win->IsEnabled() != enable))
    {
        win->Enable(enable);

#ifdef __WXGTK__
        if (enable && win->IsShown())
        {
            if (!win->HasCapture())
                win->CaptureMouse();
            if (win->HasCapture())
                win->ReleaseMouse();
        }
#endif // __WXGTK__
    }
}

void wxSTEditorFindReplacePanel::UpdateButtons()
{
    if (!m_created) return; // skip initial events sent from combobox in GTK

    // Can't search backwards when using regexp
    if (m_regexpFindCheckBox->GetValue() && m_backwardsCheckBox->IsEnabled())
    {
        m_backwardsCheckBox->SetValue(false);
        m_backwardsCheckBox->Enable(false);
    }
    else if (!m_regexpFindCheckBox->GetValue() && !m_backwardsCheckBox->IsEnabled())
    {
        m_backwardsCheckBox->Enable(true);
    }

    // update the find/replace button state
    const wxString findStr = m_findCombo->GetValue();
    bool enable = findStr.Length() > 0u;

    wxSTEditor *edit = GetEditor();

    int flags = GetFindFlags();

    if (enable)
    {
        bool changed = edit ? ((edit->GetFindString() != findStr)||(edit->GetFindFlags() != flags)) : true;
        enable &= ((edit && edit->CanFind()) ? true : changed);
    }

    wxSTE_WIN_ENABLE(m_findButton, enable);

    if (HasFlag(wxFR_REPLACEDIALOG))
    {
        // Don't want recursive find
        if (m_findReplaceData->StringCmp(findStr, m_replaceCombo->GetValue(), flags))
            enable = false;

        wxSTE_WIN_ENABLE(m_replaceAllButton, enable);

        wxString selText = edit ? edit->GetSelectedText() : wxString(wxEmptyString);

        // can only replace if already selecting the "find" text
        if (enable && edit && !edit->SelectionIsFindString(findStr, flags))
            enable = false;
        else if (!m_regexpFindCheckBox->IsChecked() && !m_findReplaceData->StringCmp(findStr, selText, flags))
            enable = false;

        wxSTE_WIN_ENABLE(m_replaceButton, enable);
        wxSTE_WIN_ENABLE(m_replaceFindButton, enable);
    }
}

//-----------------------------------------------------------------------------
// wxSTEditorFindReplaceDialog
//-----------------------------------------------------------------------------
const wxString wxSTEditorFindReplaceDialogNameStr = wxT("wxSTEditorFindReplaceDialogNameStr");

IMPLEMENT_DYNAMIC_CLASS(wxSTEditorFindReplaceDialog, wxDialog)

BEGIN_EVENT_TABLE(wxSTEditorFindReplaceDialog, wxDialog)
    EVT_BUTTON   (wxID_ANY, wxSTEditorFindReplaceDialog::OnButton)
    EVT_CHECKBOX (wxID_ANY, wxSTEditorFindReplaceDialog::OnButton)

    EVT_SIZE     (wxSTEditorFindReplaceDialog::OnSize)
    EVT_ACTIVATE (wxSTEditorFindReplaceDialog::OnActivate)
    EVT_CLOSE    (wxSTEditorFindReplaceDialog::OnCloseWindow)
END_EVENT_TABLE()

wxSTEditorFindReplaceDialog::~wxSTEditorFindReplaceDialog() {}

void wxSTEditorFindReplaceDialog::Init()
{
    m_findReplacePanel = NULL;
}

bool wxSTEditorFindReplaceDialog::Create(wxWindow *parent,
                                         wxSTEditorFindReplaceData *data,
                                         const wxString& title,
                                         int style, const wxString &name)
{
    if (!wxDialog::Create(parent, ID_STE_FINDREPLACE_DIALOG, title,
                           wxDefaultPosition, wxDefaultSize,
                           wxDEFAULT_DIALOG_STYLE_RESIZE | wxFRAME_FLOAT_ON_PARENT | style,
                           name))
                           //wxDEFAULT_FRAME_STYLE | wxRESIZE_BORDER | wxFRAME_FLOAT_ON_PARENT | style,
    {
        return false;
    }

    m_findReplacePanel = new wxSTEditorFindReplacePanel(this, wxID_ANY, data, wxDefaultPosition,
                                   wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER | style);
    m_findReplacePanel->SetTargetWindow(parent); // assume this, they can override later

    // use sizer since child file replace panel will use it to resize us
    //wxFlexGridSizer* rootSizer = new wxFlexGridSizer(1, 0, 0);
    //rootSizer->AddGrowableCol( 0 );
    //rootSizer->AddGrowableRow( 0 );
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(m_findReplacePanel, 1, wxGROW);
    SetSizer(rootSizer);
    rootSizer->SetSizeHints(this);

    // set the last user set size, but only if it's bigger than the size we
    // are already
    wxSize dialogSize = data ? data->GetDialogSize() : wxDefaultSize;
    wxSize size = GetSize();

    if (m_findReplacePanel->m_resultEditor && m_findReplacePanel->m_resultEditor->IsShown() &&
        (dialogSize != wxDefaultSize) &&
        ((dialogSize.x > size.x) || (dialogSize.y > size.y)))
    {
        SetSize(wxMax(dialogSize.x, size.x), wxMax(dialogSize.y, size.y));
    }
    Centre();
    SetIcon(wxArtProvider::GetIcon((style & wxFR_REPLACEDIALOG) ? wxART_STEDIT_REPLACE : wxART_STEDIT_FIND, wxART_FRAME_ICON));
    return true;
}

void wxSTEditorFindReplaceDialog::OnCloseWindow(wxCloseEvent &event)
{
    if (m_findReplacePanel)
        m_findReplacePanel->SendEvent(wxEVT_COMMAND_FIND_CLOSE);

    event.Skip();
}

void wxSTEditorFindReplaceDialog::OnActivate(wxActivateEvent &event)
{
    event.Skip();

    if (event.GetActive() && m_findReplacePanel)
        m_findReplacePanel->OnActivate(event);
}

void wxSTEditorFindReplaceDialog::OnButton(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case ID_STEDLG_FINDALL_CHECKBOX :
        {
            // wxWidgets needs help resizing the shown/hidden results editor
            //   This ugly hack works in any case
            //wxSize s = GetSize();
            //wxSize minSize = m_findReplacePanel->GetSize();
            //wxPrintf(wxT("DLG %d %d    %d %d\n"), s.GetWidth(), s.GetHeight(), minSize.GetWidth(), minSize.GetHeight());

            InvalidateBestSize();
            SetMinSize(wxSize(10,10));
            GetSizer()->SetMinSize(wxSize(10,10));
            m_findReplacePanel->GetSizer()->SetSizeHints(this);

            break;
        }
        case wxID_CANCEL : Destroy();
        default : event.Skip();
    }
}

void wxSTEditorFindReplaceDialog::OnSize(wxSizeEvent &event)
{
/*
    if (GetSize() != m_findReplacePanel->GetSizer()->CalcMin())
    {
        InvalidateBestSize();
        SetMinSize(wxSize(10,10));
        GetSizer()->SetMinSize(wxSize(10,10));
        m_findReplacePanel->GetSizer()->SetMinSize(wxSize(10,10));
        m_findReplacePanel->GetSizer()->SetSizeHints(m_findReplacePanel);
        GetSizer()->SetSizeHints(this);
        //SetClientSize(GetSizer()->CalcMin());
    }
    else
*/

    // remember the size of the find dialog for find all
    if (m_findReplacePanel && m_findReplacePanel->GetData() &&
        m_findReplacePanel->m_resultEditor && m_findReplacePanel->m_resultEditor->IsShown())
    {
        m_findReplacePanel->GetData()->SetDialogSize(GetSize());
    }


        event.Skip();

    //wxPrintf(wxT("wxSTEditorFindReplaceDialog::OnSize %d %d   %d %d\n"), GetSize().x, GetSize().y, event.GetSize().x, event.GetSize().y);
}
