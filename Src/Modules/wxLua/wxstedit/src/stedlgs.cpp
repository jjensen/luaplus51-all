///////////////////////////////////////////////////////////////////////////////
// File:        stedlgs.cpp
// Purpose:     Preferences dialog
// Maintainer:
// Created:     2003-04-28
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "wx/stedit/stedit.h"
#include "wx/stedit/steart.h"

#include <wx/fontenum.h>   // font support
#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include <wx/listbook.h>
#include <wx/mimetype.h>
#include <wx/valgen.h>
#include <wx/imaglist.h>
#include <wx/aboutdlg.h>

#include "wxext.h"
#include "wxtrunk.h"
#include "stedlgs_wdr.h"

//-----------------------------------------------------------------------------
// wxSTEditorPrefPageData_RefData - data shared by multiple pages shown at once
//-----------------------------------------------------------------------------

#define STEPPD_REFDATA ((wxSTEditorPrefPageData_RefData*)m_refData)

wxSTEditorPrefPageData::wxSTEditorPrefPageData()
{
    m_refData = new wxSTEditorPrefPageData_RefData();
}

wxSTEditorPrefPageData::wxSTEditorPrefPageData(const wxSTEditorPrefs& prefs,
                                               const wxSTEditorStyles& styles,
                                               const wxSTEditorLangs& langs,
                                               int languageId,
                                               wxSTEditor* editor,
                                               int options)
{
    m_refData = new wxSTEditorPrefPageData_RefData();

    STEPPD_REFDATA->m_prefs      = prefs;
    STEPPD_REFDATA->m_styles     = styles;
    STEPPD_REFDATA->m_langs      = langs;
    STEPPD_REFDATA->m_languageId = languageId;
    STEPPD_REFDATA->m_editor     = editor;
    STEPPD_REFDATA->m_options    = options;
}

wxSTEditorPrefs&  wxSTEditorPrefPageData::GetPrefs() const
{
    return STEPPD_REFDATA->m_prefs;
}
wxSTEditorStyles& wxSTEditorPrefPageData::GetStyles() const
{
    return STEPPD_REFDATA->m_styles;
}
wxSTEditorLangs&  wxSTEditorPrefPageData::GetLangs() const
{
    return STEPPD_REFDATA->m_langs;
}
int wxSTEditorPrefPageData::GetLanguageId() const
{
    return STEPPD_REFDATA->m_languageId;
}
void wxSTEditorPrefPageData::SetLanguageId(int lang_id)
{
    STEPPD_REFDATA->m_languageId = lang_id;
}
wxSTEditor* wxSTEditorPrefPageData::GetEditor() const
{
    return STEPPD_REFDATA->m_editor;
}
void wxSTEditorPrefPageData::SetEditor(wxSTEditor* editor)
{
    STEPPD_REFDATA->m_editor = editor;
}
int wxSTEditorPrefPageData::GetOptions() const
{
    return STEPPD_REFDATA->m_options;
}
void wxSTEditorPrefPageData::SetOptions(int options)
{
    STEPPD_REFDATA->m_options = options;
}

//----------------------------------------------------------------------------
// wxSTEditorPrefDialogPageBase
//----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorPrefDialogPageBase, wxPanel);

BEGIN_EVENT_TABLE(wxSTEditorPrefDialogPageBase, wxPanel)
    EVT_BUTTON( wxID_APPLY, wxSTEditorPrefDialogPageBase::OnApply )
    EVT_BUTTON( wxID_OK,    wxSTEditorPrefDialogPageBase::OnApply )
    EVT_BUTTON( wxID_RESET, wxSTEditorPrefDialogPageBase::OnReset )
END_EVENT_TABLE()

//----------------------------------------------------------------------------
// wxSTEditorPrefDialogPagePrefs
//----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorPrefDialogPagePrefs, wxSTEditorPrefDialogPageBase);

BEGIN_EVENT_TABLE(wxSTEditorPrefDialogPagePrefs, wxSTEditorPrefDialogPageBase)
END_EVENT_TABLE()

wxSTEditorPrefDialogPagePrefs::wxSTEditorPrefDialogPagePrefs( const wxSTEditorPrefPageData& editorPrefData,
                                                              const wxSTEditorPrefPageData& prefData,
                                                              wxWindow *parent,
                                                              wxWindowID winid )
                              : wxSTEditorPrefDialogPageBase(editorPrefData, prefData, parent, winid)
{
    wxCHECK_RET(editorPrefData.GetPrefs().IsOk(), wxT("Invalid preferences"));

    // The IDs are from wxDesigner, may be out of order, use lookup table
    m_prefsToIds.Alloc(STE_PREF__MAX);
    m_prefsToIds.Add(-1, STE_PREF__MAX);       // may be -1 for not handled

    m_prefsToIds[STE_PREF_HIGHLIGHT_SYNTAX]    = ID_STEDLG_HIGHLIGHT_SYNTAX_CHECKBOX;
    m_prefsToIds[STE_PREF_HIGHLIGHT_PREPROC]   = ID_STEDLG_HIGHLIGHT_PREPROC_CHECKBOX;
    m_prefsToIds[STE_PREF_HIGHLIGHT_BRACES]    = ID_STEDLG_HIGHLIGHT_BRACES_CHECKBOX;
    m_prefsToIds[STE_PREF_LOAD_INIT_LANG]      = ID_STEDLG_LOAD_INIT_LANG_CHECKBOX;
    m_prefsToIds[STE_PREF_LOAD_UNICODE]        = ID_STEDLG_LOAD_UNICODE_CHOICE;
    m_prefsToIds[STE_PREF_WRAP_MODE]           = ID_STEDLG_WRAP_MODE_CHECKBOX;
    m_prefsToIds[STE_PREF_WRAP_VISUALFLAGS]    = ID_STEDLG_WRAP_VISUALFLAGS_CHOICE;
    m_prefsToIds[STE_PREF_WRAP_VISUALFLAGSLOC] = ID_STEDLG_WRAP_VISUALFLAGSLOC_CHOICE;
    m_prefsToIds[STE_PREF_WRAP_STARTINDENT]    = ID_STEDLG_WRAP_STARTINDENT_SPINCTRL;
    m_prefsToIds[STE_PREF_ZOOM]                = ID_STEDLG_ZOOM_SPINCTRL;
    m_prefsToIds[STE_PREF_VIEW_EOL]            = ID_STEDLG_VIEW_EOL_CHECKBOX;
    m_prefsToIds[STE_PREF_VIEW_WHITESPACE]     = ID_STEDLG_VIEW_WHITESPACE_CHECKBOX;
    m_prefsToIds[STE_PREF_INDENT_GUIDES]       = ID_STEDLG_INDENT_GUIDES_CHECKBOX;
    m_prefsToIds[STE_PREF_EDGE_MODE]           = ID_STEDLG_EDGE_MODE_CHOICE;
    m_prefsToIds[STE_PREF_EDGE_COLUMN]         = ID_STEDLG_EDGE_COLUMN_SPINCTRL;
    m_prefsToIds[STE_PREF_VIEW_LINEMARGIN]     = ID_STEDLG_VIEW_LINEMARGIN_CHECKBOX;
    m_prefsToIds[STE_PREF_VIEW_MARKERMARGIN]   = ID_STEDLG_VIEW_MARKERMARGIN_CHECKBOX;
    m_prefsToIds[STE_PREF_VIEW_FOLDMARGIN]     = ID_STEDLG_VIEW_FOLDMARGIN_CHECKBOX;
    m_prefsToIds[STE_PREF_USE_TABS]            = ID_STEDLG_USE_TABS_CHECKBOX;
    m_prefsToIds[STE_PREF_TAB_INDENTS]         = ID_STEDLG_TAB_INDENTS_CHECKBOX;
    m_prefsToIds[STE_PREF_TAB_WIDTH]           = ID_STEDLG_TAB_WIDTH_SPINCTRL;
    m_prefsToIds[STE_PREF_INDENT_WIDTH]        = ID_STEDLG_INDENT_WIDTH_SPINCTRL;
    m_prefsToIds[STE_PREF_BACKSPACE_UNINDENTS] = ID_STEDLG_BACKSPACE_UNINDENTS_CHECKBOX;
    m_prefsToIds[STE_PREF_AUTOINDENT]          = ID_STEDLG_AUTOINDENT_CHECKBOX;
    m_prefsToIds[STE_PREF_CARET_LINE_VISIBLE]  = ID_STEDLG_CARET_LINE_VISIBLE_CHECKBOX;
    m_prefsToIds[STE_PREF_CARET_WIDTH]         = ID_STEDLG_CARET_WIDTH_SPINCTRL;
    m_prefsToIds[STE_PREF_CARET_PERIOD]        = ID_STEDLG_CARET_PERIOD_SPINCTRL;
    m_prefsToIds[STE_PREF_EOL_MODE]            = ID_STEDLG_EOL_MODE_CHOICE;
    m_prefsToIds[STE_PREF_PRINT_MAGNIFICATION] = ID_STEDLG_PRINT_MAGNIFICATION_SPINCTRL;
    m_prefsToIds[STE_PREF_PRINT_COLOURMODE]    = ID_STEDLG_PRINT_COLOURMODE_CHOICE;
    m_prefsToIds[STE_PREF_PRINT_WRAPMODE]      = ID_STEDLG_PRINT_WRAPMODE_CHECKBOX;
    m_prefsToIds[STE_PREF_PRINT_LINENUMBERS]   = ID_STEDLG_PRINT_LINENUMBERS_CHOICE;
    m_prefsToIds[STE_PREF_FOLD_STYLES]         = ID_STEDLG_FOLD_STYLES_LISTBOX; // req special code
    m_prefsToIds[STE_PREF_FOLDMARGIN_STYLE]    = ID_STEDLG_FOLDMARGIN_STYLE_CHOICE;
    m_prefsToIds[STE_PREF_SAVE_REMOVE_WHITESP] = ID_STEDLG_SAVE_WHITESPACE_CHECKBOX;
    m_prefsToIds[STE_PREF_SAVE_CONVERT_EOL]    = ID_STEDLG_SAVE_LINEENDINGS_CHECKBOX;

    // Add controls to this...
    //wxSTEditorPrefSizer(this, true, true);
    //SetControlValues();
}

void wxSTEditorPrefDialogPagePrefs::GetControlValues()
{
    wxSTEditorPrefs stePrefs(GetPrefData().GetPrefs());
    size_t n, count = m_prefsToIds.GetCount();

    for (n = 0; n < count; n++)
    {
        int win_id = m_prefsToIds[n];
        if (win_id < 0) // may be -1 for not handled
            continue;

        wxWindow* win = FindWindow(win_id);
        if (win == NULL)
        {
            m_prefsToIds[n] = -1; // reset it to not handled and continue
            continue;
        }

        // handle special cases first

        if ((win_id == ID_STEDLG_FOLD_STYLES_LISTBOX) && wxDynamicCast(win, wxCheckListBox))
        {
            wxCheckListBox *listBox = wxDynamicCast(win, wxCheckListBox);
            int value = 0, i, lcount = listBox->GetCount();
            for (i = 0; i < lcount; i++)
            {
                if (listBox->IsChecked(i))
                    value |= 1<<i;
            }

            stePrefs.SetPrefInt(STE_PREF_FOLD_STYLES, value);
        }
        else if (wxDynamicCast(win, wxCheckBox))
            stePrefs.SetPrefBool(n, ((wxCheckBox*)win)->GetValue());
        else if (wxDynamicCast(win, wxSpinCtrl))
            stePrefs.SetPrefInt(n, ((wxSpinCtrl*)win)->GetValue());
        else if (wxDynamicCast(win, wxChoice))
            stePrefs.SetPrefInt(n, ((wxChoice*)win)->GetSelection());
        else if (wxDynamicCast(win, wxComboBox))
            stePrefs.SetPrefInt(n, ((wxComboBox*)win)->GetSelection());
        else if (wxDynamicCast(win, wxListBox))
            stePrefs.SetPrefInt(n, ((wxListBox*)win)->GetSelection());
        else
            wxFAIL_MSG(wxT("Unknown control type in wxSTEditorPrefDialogPagePrefs::GetControlValues"));
    }
}

void wxSTEditorPrefDialogPagePrefs::SetControlValues()
{
    wxSTEditorPrefs stePrefs(GetPrefData().GetPrefs());
    size_t n, count = m_prefsToIds.GetCount();

    for (n = 0; n < count; n++)
    {
        int win_id = m_prefsToIds[n];
        if (win_id < 0) // may be -1 for not handled
            continue;

        wxWindow* win = FindWindow(win_id);
        if (win == NULL)
        {
            m_prefsToIds[n] = -1; // reset it to not handled and continue
            continue;
        }

        if ((win_id == ID_STEDLG_FOLD_STYLES_LISTBOX) && wxDynamicCast(win, wxCheckListBox))
        {
            wxCheckListBox *listBox = wxDynamicCast(win, wxCheckListBox);
            unsigned int i, lcount = listBox->GetCount();
            int value = stePrefs.GetPrefInt(STE_PREF_FOLD_STYLES);
            for (i = 0; i < lcount; i++)
                listBox->Check(i, (value & (1<<i)) != 0);
        }
        else if (wxDynamicCast(win, wxCheckBox))
            ((wxCheckBox*)win)->SetValue(stePrefs.GetPrefBool(n));
        else if (wxDynamicCast(win, wxSpinCtrl))
            ((wxSpinCtrl*)win)->SetValue(stePrefs.GetPrefInt(n));
        else if (wxDynamicCast(win, wxChoice))
            ((wxChoice*)win)->SetSelection(stePrefs.GetPrefInt(n));
        else if (wxDynamicCast(win, wxComboBox))
            ((wxComboBox*)win)->SetSelection(stePrefs.GetPrefInt(n));
        else if (wxDynamicCast(win, wxListBox))
            ((wxListBox*)win)->SetSelection(stePrefs.GetPrefInt(n));
        else
            wxFAIL_MSG(wxT("Unknown control type in wxSTEditorPrefDialogPagePrefs::SetControlValues"));
    }
}

void wxSTEditorPrefDialogPagePrefs::Apply()
{
    GetControlValues();
    wxSTEditorPrefs editorPrefs(GetEditorPrefData().GetPrefs());
    wxSTEditorPrefs prefs(GetPrefData().GetPrefs());
    size_t n, count = m_prefsToIds.GetCount();

    for (n = 0; n < count; n++)
    {
        // may be -1 for not handled or null, apply only the ones we have
        if ((m_prefsToIds[n] >= 0) && (FindWindow(m_prefsToIds[n]) != NULL))
            editorPrefs.SetPrefInt(n, prefs.GetPrefInt(n), false);
    }
}

void wxSTEditorPrefDialogPagePrefs::Reset()
{
    wxSTEditorPrefs prefs(GetPrefData().GetPrefs());
    wxSTEditorPrefs defaultPrefs(true); // defaults
    size_t n, count = m_prefsToIds.GetCount();

    for (n = 0; n < count; n++)
    {
        // may be -1 for not handled or null, reset only the ones we have
        if ((m_prefsToIds[n] >= 0) && (FindWindow(m_prefsToIds[n]) != NULL))
            prefs.SetPrefInt(n, defaultPrefs.GetPrefInt(n), false);
    }

    SetControlValues();
}

bool wxSTEditorPrefDialogPagePrefs::IsModified()
{
    GetControlValues();
    return !GetPrefData().GetPrefs().IsEqualTo(GetEditorPrefData().GetPrefs());
}

//----------------------------------------------------------------------------
// wxSTEditorPrefDialogPageStyles
//----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorPrefDialogPageStyles, wxSTEditorPrefDialogPageBase);

BEGIN_EVENT_TABLE(wxSTEditorPrefDialogPageStyles, wxSTEditorPrefDialogPageBase)
    EVT_BUTTON                (wxID_ANY, wxSTEditorPrefDialogPageStyles::OnEvent)
    EVT_CHOICE                (wxID_ANY, wxSTEditorPrefDialogPageStyles::OnEvent)
    EVT_CHECKBOX              (wxID_ANY, wxSTEditorPrefDialogPageStyles::OnEvent)
    EVT_SPINCTRL              (wxID_ANY, wxSTEditorPrefDialogPageStyles::OnSpinEvent)
    EVT_NOTEBOOK_PAGE_CHANGED (wxID_ANY, wxSTEditorPrefDialogPageStyles::OnPageChanged)

    EVT_STC_MARGINCLICK      (ID_STEDLG_STYLE_COLOUR_EDITOR, wxSTEditorPrefDialogPageStyles::OnMarginClick)
    EVT_STEDITOR_MARGINDCLICK(ID_STEDLG_STYLE_COLOUR_EDITOR, wxSTEditorPrefDialogPageStyles::OnMarginClick)
    EVT_STC_DOUBLECLICK      (ID_STEDLG_STYLE_COLOUR_EDITOR, wxSTEditorPrefDialogPageStyles::OnMarginClick)
    EVT_STC_MARGINCLICK      (ID_STEDLG_STYLE_STYLE_EDITOR,  wxSTEditorPrefDialogPageStyles::OnMarginClick)
    EVT_STEDITOR_MARGINDCLICK(ID_STEDLG_STYLE_STYLE_EDITOR,  wxSTEditorPrefDialogPageStyles::OnMarginClick)
    EVT_STC_DOUBLECLICK      (ID_STEDLG_STYLE_STYLE_EDITOR,  wxSTEditorPrefDialogPageStyles::OnMarginClick)
END_EVENT_TABLE()

#define STYLE_SAMPLE_TEXT(n) wxString(GetPrefData().GetStyles().GetStyleName(m_styleArray[n]))

wxString wxSTEditorPrefDialogPageStyles::sm_helpString(
    wxT("Help On Setting Styles\n\n")

    wxT("This editor supports syntax highlighting for a number of different programming languages. ")
    wxT("In order to make it easy to create your own color and style theme, a single generic list of styles is used. ")
    wxT("This is because some languages have rather esoteric idioms and if this editor catered to each and every one there would be hundreds of different styles to have to adjust. ")
    wxT("The list tries to encompass most of the common styles for all the language lexers (code parsers). ")
    wxT("Therefore, the names given to the styles are meant only as hints as to what the style will be used for and you can map any editor style to any lexer style. ")
    wxT("For more information you should examine the \"Languages\" tab where you can change the mapping of the editor and lexer styles.\n\n")

    wxT("Select the style you want to change by clicking on the margin or double clicking the line. ")
    wxT("You can change the individual attributes for a style by checking the checkboxes to the left or ")
    wxT("uncheck them to use the attribute value from the \"Default Style\" in the \"Lexer Styles\". ")
    wxT("It is a good idea to set the \"Default Style\" first so that you only set the values for the other styles that you want to be different. ")
    wxT("Attributes that do not apply for specific styles are disabled.\n\n")

    wxT("The \"Editor Colors\" page allows you to change the overall appearance of the editor for any language.\n\n")

    wxT("The \"Lexer Styles\" page shows a list of styles to allow you to adjust them all at once to easily create a theme that'll look good for any language. ")
    wxT("You can also select particular languages to see what styles they use and get a feel for what they would look like. ")
    wxT("The styles that are used for the currently set lexer in the \"Languages\" tab are shown with a white cross in the margin.")
);

void wxSTEditorPrefDialogPageStyles::Init()
{
    m_style_max_len     = 10;
    m_current_style     = 0;
    m_last_language_ID  = -1;
    m_colourData        = NULL;

    m_styleNotebook     = NULL;
    m_colourEditor      = NULL;
    m_styleEditor       = NULL;
    m_helpEditor        = NULL;
    m_colour_editor_marker_handle = 0;
    m_style_editor_marker_handle  = 0;

    m_langChoice        = NULL;

    m_fontCheckBox      = NULL;
    m_fontButton        = NULL;
    m_fontChoice        = NULL;
    m_fontSizeCheckBox  = NULL;
    m_fontSizeSpin      = NULL;
    m_attribCheckBox    = NULL;
    m_boldCheckBox      = NULL;
    m_italicsCheckBox   = NULL;
    m_underlineCheckBox = NULL;
    m_eolFillCheckBox   = NULL;
    m_fontForeCheckBox  = NULL;
    m_fontForeButton    = NULL;
    m_fontBackCheckBox  = NULL;
    m_fontBackButton    = NULL;
}

wxSTEditorPrefDialogPageStyles::wxSTEditorPrefDialogPageStyles(const wxSTEditorPrefPageData& editorPrefData,
                                                               const wxSTEditorPrefPageData& prefData,
                                                               wxWindow *parent,
                                                               wxWindowID winid )
                               :wxSTEditorPrefDialogPageBase(editorPrefData, prefData, parent, winid)
{
    Init();
    wxSTEditorStyles steStyles(GetPrefData().GetStyles());
    wxCHECK_RET(steStyles.IsOk(), wxT("Invalid styles")); // langs may be NULL

    wxSTERecursionGuard guard(m_rGuard_setting_style);

    // fill colour data with rainbowish colors
    m_colourData = new wxColourData;
    m_colourData->SetChooseFull(true);
    m_colourData->SetCustomColour(0,  wxColour(0,     0,   0));
    m_colourData->SetCustomColour(1,  wxColour(64,   64,  64));
    m_colourData->SetCustomColour(2,  wxColour(128, 128, 128));
    m_colourData->SetCustomColour(3,  wxColour(192, 192, 192));
    m_colourData->SetCustomColour(4,  wxColour(255, 255, 255));
    m_colourData->SetCustomColour(5,  wxColour(255,   0,   0));
    m_colourData->SetCustomColour(6,  wxColour(255,   0, 128));
    m_colourData->SetCustomColour(7,  wxColour(255,   0, 255));
    m_colourData->SetCustomColour(8,  wxColour(128,   0, 255));
    m_colourData->SetCustomColour(9,  wxColour(  0,   0, 255));
    m_colourData->SetCustomColour(10, wxColour(  0, 128, 255));
    m_colourData->SetCustomColour(11, wxColour(  0, 255, 255));
    m_colourData->SetCustomColour(12, wxColour(  0, 255, 128));
    m_colourData->SetCustomColour(13, wxColour(  0, 255,   0));
    m_colourData->SetCustomColour(14, wxColour(128, 255,   0));
    m_colourData->SetCustomColour(15, wxColour(255, 255,   0));

    wxSTEditorStyleSizer( this, true, true );

    m_styleNotebook = wxStaticCast(FindWindow(ID_STEDLG_STYLE_NOTEBOOK), wxNotebook);
    m_colourEditor  = new wxSTEditor(m_styleNotebook, ID_STEDLG_STYLE_COLOUR_EDITOR);
    m_styleEditor   = new wxSTEditor(m_styleNotebook, ID_STEDLG_STYLE_STYLE_EDITOR);
    m_helpEditor    = new wxSTEditor(m_styleNotebook, wxID_ANY);

    m_styleNotebook->AddPage(m_colourEditor, _("Editor Colors"), true);
    m_styleNotebook->AddPage(m_styleEditor,  _("Lexer Styles"),  false);
    m_styleNotebook->AddPage(m_helpEditor,   _("Help"),          false);

    m_helpEditor->SetWrapMode(wxSTC_WRAP_WORD);
    m_helpEditor->SetText(sm_helpString);
    m_helpEditor->SetReadOnly(true);

    SetupEditor(m_colourEditor);
    SetupEditor(m_styleEditor);

    m_colourEditor->SetStyleBits(5); // want to show indicators

    // add markers to editors
    m_colour_editor_marker_handle = m_colourEditor->MarkerAdd(0, 0);
    m_style_editor_marker_handle  = m_styleEditor->MarkerAdd(0, 0);

    m_langChoice        = wxStaticCast(FindWindow(ID_STEDLG_STYLELANG_CHOICE   ), wxChoice  );
    m_fontCheckBox      = wxStaticCast(FindWindow(ID_STEDLG_FONT_CHECKBOX      ), wxCheckBox);
    m_fontButton        = wxStaticCast(FindWindow(ID_STEDLG_FONT_BUTTON        ), wxButton  );
    m_fontChoice        = wxStaticCast(FindWindow(ID_STEDLG_FONT_CHOICE        ), wxChoice  );
    m_fontSizeCheckBox  = wxStaticCast(FindWindow(ID_STEDLG_FONTSIZE_CHECKBOX  ), wxCheckBox);
    m_fontSizeSpin      = wxStaticCast(FindWindow(ID_STEDLG_FONTSIZE_SPINCTRL  ), wxSpinCtrl);
    m_attribCheckBox    = wxStaticCast(FindWindow(ID_STEDLG_ATTRIBUTES_CHECKBOX), wxCheckBox);
    m_boldCheckBox      = wxStaticCast(FindWindow(ID_STEDLG_BOLD_CHECKBOX      ), wxCheckBox);
    m_italicsCheckBox   = wxStaticCast(FindWindow(ID_STEDLG_ITALICS_CHECKBOX   ), wxCheckBox);
    m_underlineCheckBox = wxStaticCast(FindWindow(ID_STEDLG_UNDERLINE_CHECKBOX ), wxCheckBox);
    m_eolFillCheckBox   = wxStaticCast(FindWindow(ID_STEDLG_EOLFILL_CHECKBOX   ), wxCheckBox);
    m_fontForeCheckBox  = wxStaticCast(FindWindow(ID_STEDLG_FONTFORE_CHECKBOX  ), wxCheckBox);
    m_fontForeButton    = wxStaticCast(FindWindow(ID_STEDLG_FONTFORE_BUTTON    ), wxButton  );
    m_fontBackCheckBox  = wxStaticCast(FindWindow(ID_STEDLG_FONTBACK_CHECKBOX  ), wxCheckBox);
    m_fontBackButton    = wxStaticCast(FindWindow(ID_STEDLG_FONTBACK_BUTTON    ), wxButton  );

    m_langChoice->Clear();
    if (GetPrefData().GetLangs().IsOk())
    {
        wxSTEditorLangs steLangs(GetPrefData().GetLangs());
        m_langChoice->Append(_("Show all styles"));
        size_t n = 0, count = steLangs.GetCount();
        for (n = 0; n < count; n++)
        {
            if (steLangs.GetUseLanguage(n))
                m_langChoice->Append(steLangs.GetName(n), (void*)n);
        }

        m_langChoice->SetSelection(0);
    }
    else
    {
        m_langChoice->Hide();
    }

    m_styleArray = steStyles.GetStylesArray(true);
    size_t n = 0, count = m_styleArray.GetCount();

    // find max length of all style names (for appending extra text)
    for (n = 0; n < count; n++)
    {
        int len = (int)steStyles.GetStyleName(m_styleArray[n]).Len() + 2;

        m_style_max_len = wxMax(m_style_max_len, len);
    }

    FillStyleEditor(m_styleEditor);

    for (n = 0; n < count; n++)
    {
        if (m_styleArray[n] > STE_STYLE_LANG__MAX)
        {
            m_colourEditor->AddText(STYLE_SAMPLE_TEXT(n));
            m_colourLineArray.Add((int)n);

            // add the markers
            int marker_num = m_styleArray[n] - STE_STYLE_MARKER__FIRST;
            if ((marker_num >= wxSTC_MARKNUM_FOLDEREND) &&
                (marker_num <= wxSTC_MARKNUM_FOLDEROPEN))
            {
                m_colourEditor->MarkerAdd((int)m_colourLineArray.GetCount()-1, marker_num);
            }

            if (n < count-1)
                m_colourEditor->AddText(wxT("\n"));
        }
    }

    m_current_style = m_styleArray[m_colourLineArray[0]];
    m_colourEditor->SetReadOnly(true);
    m_styleEditor->SetReadOnly(true);

    m_fontChoice->Clear();
    wxFontEnumerator fontEnum;
    fontEnum.EnumerateFacenames(wxFONTENCODING_SYSTEM, true);
    wxArrayString faceNames = fontEnum.GetFacenames();

    faceNames.Sort();
    count = faceNames.GetCount();
    for (n = 0; n < count; n++)
        m_fontChoice->Append(wxT("*") + faceNames.Item(n));

    UpdateEditor(m_colourEditor, m_colourLineArray);
    UpdateEditor(m_styleEditor, m_styleLineArray);
    GetControlValues();
    SetControlValues();
}

wxSTEditorPrefDialogPageStyles::~wxSTEditorPrefDialogPageStyles()
{
    delete m_colourData;
}

void wxSTEditorPrefDialogPageStyles::FillStyleEditor(wxSTEditor* editor)
{
    editor->SetReadOnly(false);
    editor->ClearAll();
    wxSTEditorLangs steLangs(GetPrefData().GetLangs());
    wxSTEditorStyles steStyles(GetPrefData().GetStyles());

    size_t n = 0, count = 0;
    wxArrayInt langStyles;
    int lang_n = -1;

    if (steLangs.IsOk() && m_langChoice->IsShown() && (m_langChoice->GetSelection() != 0))
    {
        lang_n = (long)m_langChoice->GetClientData(m_langChoice->GetSelection());
        count = steLangs.GetStyleCount(lang_n);
        for (n = 0; n < count; n++)
        {
            //if (langStyles.Index(steLangs.GetSTEStyle(lang_n, n)) == wxNOT_FOUND)
                langStyles.Add(steLangs.GetSTEStyle(lang_n, n));
        }
    }
    else
        langStyles = m_styleArray;

    m_styleLineArray.Clear();
    count = langStyles.GetCount();
    wxString text;

    for (n = 0; n < count; n++)
    {
        // The m_styleEditor contains all the lexer styles
        if (langStyles[n] < STE_STYLE_LANG__MAX)
        {
            wxString text = steStyles.GetStyleName(langStyles[n]);

            if (steLangs.IsOk() && (lang_n >= 0))
            {
                text += wxString(wxT(' '), m_style_max_len - text.Len());
                text += steLangs.GetStyleDescription(lang_n, n);
            }

            if ((n+1 < count) && (langStyles[n+1] < STE_STYLE_LANG__MAX))
                text += wxT("\n");

            m_styleEditor->AddText(text);
            m_styleLineArray.Add(langStyles[n]);
        }
        else
            break;
    }

    editor->SetReadOnly(true);
}

void wxSTEditorPrefDialogPageStyles::SetupEditor(wxSTEditor* editor)
{
    //m_styleEditor->SetModEventMask(0);
    editor->RegisterStyles(GetPrefData().GetStyles());

    // setup styles and dummy lexer
    editor->SetStyleBits(7); // FIXME
    editor->SetLexer(wxSTC_LEX_NULL);
    // line numbers
    editor->SetMarginType(STE_MARGIN_NUMBER, wxSTC_MARGIN_NUMBER);
    editor->SetMarginWidth(STE_MARGIN_NUMBER, editor->TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_199")));
    editor->SetMarginSensitive(STE_MARGIN_NUMBER, true); // don't select line
    // marker magin
    editor->SetMarginType(STE_MARGIN_MARKER, wxSTC_MARGIN_SYMBOL);
    editor->SetMarginWidth(STE_MARGIN_MARKER, 16);
    editor->MarkerDefine(0, wxSTC_MARK_CIRCLE, *wxBLACK, *wxRED);
    editor->MarkerDefine(1, wxSTC_MARK_PLUS, *wxBLACK, *wxWHITE);
    editor->SetMarginSensitive(STE_MARGIN_MARKER, true);
    // fold margin
    editor->SetMarginType(STE_MARGIN_FOLD, wxSTC_MARGIN_SYMBOL);
    editor->SetMarginMask(STE_MARGIN_FOLD, wxSTC_MASK_FOLDERS);
    editor->SetMarginWidth(STE_MARGIN_FOLD, 16);
    editor->SetMarginSensitive(STE_MARGIN_FOLD, true); // don't select line
    // edge colour
    editor->SetEdgeMode(wxSTC_EDGE_LINE);
    editor->SetEdgeColumn(80);
}

void wxSTEditorPrefDialogPageStyles::OnEvent(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_setting_style);
    if (guard.IsInside()) return;

    wxSTEditorStyles steStyles(GetPrefData().GetStyles());

    switch (event.GetId())
    {
        case ID_STEDLG_STYLELANG_CHOICE :
        {
            m_styleEditor->MarkerDeleteHandle(m_style_editor_marker_handle);
            FillStyleEditor(m_styleEditor);
            m_style_editor_marker_handle = m_styleEditor->MarkerAdd(0, 0);
            break;
        }

        // Font facename ------------------------------------------------------
        case ID_STEDLG_FONT_CHECKBOX :
        {
            steStyles.SetUseDefault(m_current_style, STE_STYLE_USEDEFAULT_FACENAME, !event.IsChecked());
            break;
        }
        case ID_STEDLG_FONT_BUTTON :
        {
            wxFontData data;
            data.EnableEffects(false);
            data.SetAllowSymbols(false);
            data.SetInitialFont(steStyles.GetFont(m_current_style));
            wxFontDialog dialog(this, data);
            if (dialog.ShowModal() == wxID_OK)
            {
                steStyles.SetFont(m_current_style, dialog.GetFontData().GetChosenFont());
            }
            break;
        }
        case ID_STEDLG_FONT_CHOICE :
        {
            steStyles.SetFaceName(m_current_style, m_fontChoice->GetStringSelection().AfterLast(wxT('*')));
            break;
        }
        // Font size  ---------------------------------------------------------
        case ID_STEDLG_FONTSIZE_CHECKBOX :
        {
            steStyles.SetUseDefault(m_current_style, STE_STYLE_USEDEFAULT_FONTSIZE, !event.IsChecked());
            break;
        }
        case ID_STEDLG_FONTSIZE_SPINCTRL :
        {
            steStyles.SetSize(m_current_style, event.GetInt());
            break;
        }
        // Font attributes ----------------------------------------------------
        case ID_STEDLG_ATTRIBUTES_CHECKBOX :
        {
            steStyles.SetUseDefault(m_current_style, STE_STYLE_USEDEFAULT_FONTSTYLE, !event.IsChecked());
            break;
        }
        case ID_STEDLG_BOLD_CHECKBOX :
        {
            steStyles.SetBold(m_current_style, event.IsChecked());
            break;
        }
        case ID_STEDLG_ITALICS_CHECKBOX :
        {
            steStyles.SetItalic(m_current_style, event.IsChecked());
            break;
        }
        case ID_STEDLG_UNDERLINE_CHECKBOX :
        {
            steStyles.SetUnderlined(m_current_style, event.IsChecked());
            break;
        }
        case ID_STEDLG_EOLFILL_CHECKBOX :
        {
            steStyles.SetEOLFilled(m_current_style, event.IsChecked());
            break;
        }
        // Font colours -------------------------------------------------------
        case ID_STEDLG_FONTFORE_CHECKBOX :
        {
            steStyles.SetUseDefault(m_current_style, STE_STYLE_USEDEFAULT_FORECOLOUR, !event.IsChecked());
            break;
        }
        case ID_STEDLG_FONTFORE_BUTTON :
        {
            m_colourData->SetColour(steStyles.GetForegroundColour(m_current_style));
            wxColourDialog dialog(this, m_colourData);
            dialog.SetTitle(_("Choose the font's foreground color"));
            if (dialog.ShowModal() == wxID_OK)
            {
                *m_colourData = dialog.GetColourData();
                steStyles.SetForegroundColour(m_current_style, m_colourData->GetColour());
            }
            break;
        }
        case ID_STEDLG_FONTBACK_CHECKBOX :
        {
            steStyles.SetUseDefault(m_current_style, STE_STYLE_USEDEFAULT_BACKCOLOUR, !event.IsChecked());
            break;
        }
        case ID_STEDLG_FONTBACK_BUTTON :
        {
            m_colourData->SetColour(steStyles.GetBackgroundColour(m_current_style));
            wxColourDialog dialog(this, m_colourData);
            dialog.SetTitle(_("Choose the font's background color"));
            if (dialog.ShowModal() == wxID_OK)
            {
                *m_colourData = dialog.GetColourData();
                steStyles.SetBackgroundColour(m_current_style, m_colourData->GetColour());
            }
            break;
        }

        default : event.Skip(); break;
    }

    GetControlValues();
    SetControlValues();
}

void wxSTEditorPrefDialogPageStyles::OnSpinEvent( wxSpinEvent &event )
{
    OnEvent( *((wxCommandEvent*)&event) ); // forward it
}

void wxSTEditorPrefDialogPageStyles::OnPageChanged( wxNotebookEvent &event )
{
    OnEvent( *((wxCommandEvent*)&event) ); // forward it
}

void wxSTEditorPrefDialogPageStyles::OnMarginClick( wxStyledTextEvent &event )
{
    wxSTERecursionGuard guard(m_rGuard_setting_style);
    if (guard.IsInside()) return;

    if (!m_fontBackButton) return; // set after editor is fully created

    if (event.GetEventType() == wxEVT_STEDITOR_MARGINDCLICK)
        return;

    wxSTEditor *editor = wxStaticCast(event.GetEventObject(), wxSTEditor);
    STE_TextPos pos = event.GetPosition();

    if (event.GetEventType() == wxEVT_STC_DOUBLECLICK) // event pos not set correctly
        pos = editor->GetCurrentPos();

    int line = editor->LineFromPosition(pos);

    if (editor->GetLine(line).Strip(wxString::both).IsEmpty())
        return;

    if ((editor == m_colourEditor) &&
        (m_colourEditor->MarkerLineFromHandle(m_colour_editor_marker_handle) != line))
    {
        m_colourEditor->MarkerDeleteHandle(m_colour_editor_marker_handle);
        m_colour_editor_marker_handle = m_colourEditor->MarkerAdd(line, 0);
    }
    else if ((editor == m_styleEditor) &&
        (m_styleEditor->MarkerLineFromHandle(m_style_editor_marker_handle) != line))
    {
        m_styleEditor->MarkerDeleteHandle(m_style_editor_marker_handle);
        m_style_editor_marker_handle = m_styleEditor->MarkerAdd(line, 0);
    }
    else
        return;

    GetControlValues();
    SetControlValues();
}

void wxSTEditorPrefDialogPageStyles::UpdateEditor(wxSTEditor* editor, wxArrayInt& lineArray)
{
    wxCHECK_RET(editor, wxT("Invalid editor"));
    wxSTERecursionGuard guard(m_rGuard_setting_style);
    //if (guard.IsInside()) return;

    wxSTEditorStyles steStyles(GetPrefData().GetStyles());
    //m_styleEditor->ClearAllIndicators(); see below

    editor->SetReadOnly(false);

    steStyles.UpdateEditor(editor);

    size_t n, count = lineArray.GetCount();
    for (n = 0; n < count; n++)
    {
        int style_index = lineArray[n];
        int ste_style = m_styleArray[style_index];
        // skip over default preset styles of scintilla
        size_t sci_style = n;
        if (n >= wxSTC_STYLE_DEFAULT)
            sci_style += (wxSTC_STYLE_INDENTGUIDE - wxSTC_STYLE_DEFAULT);

        steStyles.SetEditorStyle((int)sci_style, ste_style, editor);

        int line = (int)n;
        wxString lineStr = editor->GetLine(line);

        STE_TextPos pos = editor->PositionFromLine(line);
        size_t line_len = lineStr.Length(); // with \n at end for right length

        if (line_len < 2) // skip empty lines
            continue;

        editor->StartStyling(pos, 0xff);
        editor->SetStyling((int)line_len, (int)sci_style);

        // Note: can't do this since using 7 style bits
        if ( (ste_style >= STE_STYLE_INDIC_0) && (ste_style <= STE_STYLE_INDIC_2))
        {
            editor->SetIndicator(pos, (int)line_len, steStyles.GetIndicatorMask(ste_style-STE_STYLE_INDIC_0));
        }
    }

    editor->SetReadOnly(true);
}

void wxSTEditorPrefDialogPageStyles::GetControlValues()
{
    if (m_styleNotebook->GetSelection() == 0)
        m_current_style = m_styleArray[m_colourLineArray[m_colourEditor->MarkerLineFromHandle(m_colour_editor_marker_handle)]];
    else if (m_styleNotebook->GetSelection() == 1)
        m_current_style = m_styleArray[m_styleLineArray[m_styleEditor->MarkerLineFromHandle(m_style_editor_marker_handle)]];
}

void wxSTEditorPrefDialogPageStyles::SetControlValues()
{
    wxSTERecursionGuard guard(m_rGuard_setting_style); // just block others

    wxSTEditorStyles steStyles(GetPrefData().GetStyles());

    // enable/disable unused items
    int  use_info = steStyles.GetStyleUsage(m_current_style);
    bool use_font = (use_info & STE_STYLE_USES_FONT      ) != 0;
    bool use_fore = (use_info & STE_STYLE_USES_FORECOLOUR) != 0;
    bool use_back = (use_info & STE_STYLE_USES_BACKCOLOUR) != 0;

    // disable items for help page
    if (m_styleNotebook->GetSelection() == 2)
        use_info = use_font = use_fore = use_back = false;

    // font name controls -----------------------------------------------------
    bool def_font = steStyles.GetUsesDefault(m_current_style, STE_STYLE_USEDEFAULT_FACENAME)
                      && (m_current_style != 0);
    m_fontCheckBox->SetValue(!def_font && use_font);
    if (use_font)
    {
        int font_sel = m_fontChoice->FindString(steStyles.GetFaceName(m_current_style));
        if (font_sel == wxNOT_FOUND)
            font_sel = m_fontChoice->FindString(wxT("*")+steStyles.GetFaceName(m_current_style));
        if ((font_sel >= 0) && (font_sel != m_fontChoice->GetSelection()))
            m_fontChoice->SetSelection(font_sel);
        else if (font_sel == wxNOT_FOUND)
        {
            m_fontChoice->Append(steStyles.GetFaceName(m_current_style));
            m_fontChoice->SetSelection(m_fontChoice->GetCount()-1);
        }
    }
    m_fontCheckBox->Enable((m_current_style != 0) && use_font);
    m_fontButton->Enable(!def_font && use_font);
    m_fontChoice->Enable(!def_font && use_font);

    // font size controls -----------------------------------------------------
    bool def_fontsize = steStyles.GetUsesDefault(m_current_style, STE_STYLE_USEDEFAULT_FONTSIZE)
                        && (m_current_style != 0);
    m_fontSizeCheckBox->SetValue(!def_fontsize && use_font);
    if (use_font) m_fontSizeSpin->SetValue(steStyles.GetSize(m_current_style));
    m_fontSizeCheckBox->Enable((m_current_style != 0) && use_font);
    m_fontSizeSpin->Enable(!def_fontsize && use_font);

    // font attributes controls -----------------------------------------------
    bool def_attr = steStyles.GetUsesDefault(m_current_style, STE_STYLE_USEDEFAULT_FONTSTYLE)
                    && (m_current_style != 0);
    m_attribCheckBox->SetValue(!def_attr && use_font);
    m_boldCheckBox->SetValue(steStyles.GetBold(m_current_style) && use_font);
    m_italicsCheckBox->SetValue(steStyles.GetItalic(m_current_style)  && use_font);
    m_underlineCheckBox->SetValue(steStyles.GetUnderlined(m_current_style) && use_font);
    m_eolFillCheckBox->SetValue(steStyles.GetEOLFilled(m_current_style) && use_font);
    m_attribCheckBox->Enable((m_current_style != 0) && use_font);
    m_boldCheckBox->Enable(!def_attr && use_font);
    m_italicsCheckBox->Enable(!def_attr && use_font);
    m_underlineCheckBox->Enable(!def_attr && use_font);
    m_eolFillCheckBox->Enable(!def_attr && use_font);

    // font foreground controls -----------------------------------------------
    bool def_fore = steStyles.GetUsesDefault(m_current_style, STE_STYLE_USEDEFAULT_FORECOLOUR)
                    && (m_current_style != 0);
    m_fontForeCheckBox->SetValue(!def_fore && use_fore);
    m_fontForeButton->SetForegroundColour(steStyles.GetBackgroundColour(m_current_style));
    m_fontForeButton->SetBackgroundColour(steStyles.GetForegroundColour(m_current_style));
    m_fontForeCheckBox->Enable((m_current_style != 0) && use_fore);
    m_fontForeButton->Enable(!def_fore && use_fore);

    // font background controls -----------------------------------------------
    bool def_back = steStyles.GetUsesDefault(m_current_style, STE_STYLE_USEDEFAULT_BACKCOLOUR)
                    && (m_current_style != 0);
    m_fontBackCheckBox->SetValue(!def_back && use_back);
    m_fontBackButton->SetForegroundColour(steStyles.GetForegroundColour(m_current_style));
    m_fontBackButton->SetBackgroundColour(steStyles.GetBackgroundColour(m_current_style));
    m_fontBackCheckBox->Enable((m_current_style != 0) && use_back);
    m_fontBackButton->Enable(!def_back && use_back);

    m_styleEditor->SetReadOnly(false);

    if (m_styleNotebook->GetSelection() == 0)
        UpdateEditor(m_colourEditor, m_colourLineArray);
    else if (m_styleNotebook->GetSelection() == 1)
        UpdateEditor(m_styleEditor, m_styleLineArray);

    m_langChoice->Enable(m_styleNotebook->GetSelection() == 1);

    // show what styles are used for this language
    //   note: could use own wxSTEditorLangs, but if editor doesn't have one
    //         the user can't change it anyway so pretend not to know what styles are used.
    wxSTEditorLangs steLangs(GetPrefData().GetLangs());
    if (steLangs.IsOk() && (m_last_language_ID != GetPrefData().GetLanguageId()))
    {
        m_last_language_ID = GetPrefData().GetLanguageId();
        m_styleEditor->MarkerDeleteAll(1);

        size_t style_count = steLangs.GetStyleCount(m_last_language_ID);

        for (size_t s_n = 0; s_n < style_count; s_n++)
        {
            int s = steLangs.GetSTEStyle(m_last_language_ID, s_n);
            if (s >= 0)
                m_styleEditor->MarkerAdd(s, 1);
        }
    }

    // caret
    //if (m_prefs)
    //    m_styleEditor->SetCaretLineVisible(m_prefs->GetCaretLineVisible());

    m_styleEditor->SetReadOnly(true);
}

void wxSTEditorPrefDialogPageStyles::Apply()
{
    GetControlValues();
    GetEditorPrefData().GetStyles().Copy(GetPrefData().GetStyles());
}
void wxSTEditorPrefDialogPageStyles::Reset()
{
    GetPrefData().GetStyles().Reset();
    SetControlValues();
}

bool wxSTEditorPrefDialogPageStyles::IsModified()
{
    GetControlValues();
    return !GetPrefData().GetStyles().IsEqualTo(GetEditorPrefData().GetStyles());
}

//----------------------------------------------------------------------------
// wxSTEditorPrefDialogPageLangs
//----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorPrefDialogPageLangs, wxSTEditorPrefDialogPageBase);

BEGIN_EVENT_TABLE(wxSTEditorPrefDialogPageLangs, wxSTEditorPrefDialogPageBase)
    EVT_CHOICE          (wxID_ANY,                    wxSTEditorPrefDialogPageLangs::OnChoice)

    EVT_STC_MARGINCLICK      (ID_STEDLG_LANG_STYLE_EDITOR, wxSTEditorPrefDialogPageLangs::OnMarginClick)
    EVT_STEDITOR_MARGINDCLICK(ID_STEDLG_LANG_STYLE_EDITOR, wxSTEditorPrefDialogPageLangs::OnMarginClick)
    EVT_STC_DOUBLECLICK      (ID_STEDLG_LANG_STYLE_EDITOR, wxSTEditorPrefDialogPageLangs::OnMarginClick)
END_EVENT_TABLE()

wxString wxSTEditorPrefDialogPageLangs::sm_helpString(
    wxT("Help On Language Lexers (code parsers)\n\n")

    wxT("This editor supports syntax highlighting for a number of different programming languages. ")
    wxT("Choose the language lexer you want to use for the current document using the \"Language\" choice control. ")
    wxT("The file patterns are a semicolon ';' separated list of file patterns to detect what language to use for different file types. ")
    wxT("For example; C/C++ has the patterns '*.c;*.cpp;*.h' which means that files ending with .c, .cpp, .h will be lexed using C/C++ programming language.\n\n")

    wxT("The \"Styles\" tab allows you to change the mapping between the editor styles on the left and the lexer styles on the right. ")
    wxT("See the overall \"Styles\" tab to adjust the look of the editor styles and to learn about the names given to the editor styles. ")
    wxT("Use the \"Choose style\" choice control to select a different editor style, note that the names of the editor styles are just hints for what they're most used for and can be arbitrarily mapped to any lexer style. ")
    wxT("Select a different lexer style to modify by clicking on the margin or double clicking on the appropriate line.\n\n")

    wxT("The \"Keywords\" tab allows you to view the special keywords for the language. ")
    wxT("Different languages may have different numbers of keyword sets and each one can be given their own styling. ")
    wxT("You may add your own additional keywords by entering them as a space separated list into the \"Additional keywords\" text control. ")
);

void wxSTEditorPrefDialogPageLangs::Init()
{
    m_languageChoice       = NULL;
    m_filepatternTextCtrl  = NULL;
    m_notebook             = NULL;
    m_styleChoice          = NULL;
    m_styleEditor          = NULL;
    m_keywordsChoice       = NULL;
    m_keywordsTextCtrl     = NULL;
    m_userKeywordsTextCtrl = NULL;
    m_helpEditor           = NULL;

    m_style_editor_marker_handle = 0;
    m_current_lang               = 0;
    m_current_style_n            = 0;
    m_keyword_n                  = 0;
    m_max_stylename_length       = 0;
}

wxSTEditorPrefDialogPageLangs::wxSTEditorPrefDialogPageLangs(const wxSTEditorPrefPageData& editorPrefData,
                                                             const wxSTEditorPrefPageData& prefData,
                                                             wxWindow *parent,
                                                             wxWindowID winid )
                              :wxSTEditorPrefDialogPageBase(editorPrefData, prefData, parent, winid)
{
    wxCHECK_RET(GetPrefData().GetLangs().IsOk(), wxT("Invalid languages"));
    wxCHECK_RET(GetPrefData().GetStyles().IsOk(), wxT("Invalid styles"));

    wxSTEditorLangs  steLangs(GetPrefData().GetLangs());
    wxSTEditorStyles steStyles(GetPrefData().GetStyles());

    size_t n = 0, count;
    m_style_editor_marker_handle = 0;
    m_current_lang         = GetPrefData().GetLanguageId();
    m_current_style_n      = 0;
    m_keyword_n            = -1;
    m_max_stylename_length = 20;

    wxSTEditorLangSizer(this, true, true);

    m_languageChoice      = wxStaticCast(FindWindow(ID_STEDLG_LANG_CHOICE),wxChoice);
    m_filepatternTextCtrl = wxStaticCast(FindWindow(ID_STEDLG_FILEPATTERN_TEXTCTRL), wxTextCtrl);
    m_notebook            = wxStaticCast(FindWindow(ID_STEDLG_LANG_NOTEBOOK), wxNotebook);

    // Create and add the style panel
    wxPanel *stylePanel = new wxPanel(m_notebook);
    m_styleEditor = new wxSTEditor(stylePanel, ID_STEDLG_LANG_STYLE_EDITOR,
                                         wxDefaultPosition, wxDefaultSize);
    m_styleEditor->RegisterStyles(steStyles);
    // setup styles and dummy lexer
    m_styleEditor->SetStyleBits(7); // FIXME
    m_styleEditor->SetLexer(wxSTC_LEX_NULL);
    // marker magin
    m_styleEditor->SetMarginType(STE_MARGIN_MARKER, wxSTC_MARGIN_SYMBOL);
    m_styleEditor->SetMarginWidth(STE_MARGIN_MARKER, 16);
    m_styleEditor->MarkerDefine(0, wxSTC_MARK_CIRCLE, *wxBLACK, *wxRED);
    m_styleEditor->MarkerDefine(1, wxSTC_MARK_PLUS, *wxBLACK, *wxWHITE);
    m_styleEditor->SetMarginSensitive(STE_MARGIN_MARKER, true);
    // add marker
    m_style_editor_marker_handle = m_styleEditor->MarkerAdd(0, 0);

    wxSTEditorLangStyleSizer(stylePanel, true, true);
    m_notebook->AddPage(stylePanel, _("Styles"), true);

    // add the styles to the choice
    m_styleChoice = wxStaticCast(FindWindow(ID_STEDLG_LANG_STYLE_CHOICE), wxChoice);
    m_styleChoice->Clear();
    wxArrayInt styleArray = steStyles.GetStylesArray(false);
    count = styleArray.GetCount();
    for (n = 0; n < count; n++)
    {
        wxString styleName = steStyles.GetStyleName(styleArray[n]);
        m_styleChoice->Append(styleName, (void*)(long)styleArray[n]);
        size_t len = styleName.Length() + 1;
        if (m_max_stylename_length < len)
            m_max_stylename_length = len;
    }

    // Create and add the keyword panel
    wxPanel *keywordPanel = new wxPanel(m_notebook);
    wxSTEditorLangKeywordSizer(keywordPanel, true, true);
    m_notebook->AddPage(keywordPanel, _("Keywords"), false);

    m_keywordsChoice       = wxStaticCast(FindWindow(ID_STEDLG_LANG_KEYWORD_CHOICE      ), wxChoice  );
    m_keywordsTextCtrl     = wxStaticCast(FindWindow(ID_STEDLG_LANG_KEYWORD_TEXTCTRL    ), wxTextCtrl);
    m_userKeywordsTextCtrl = wxStaticCast(FindWindow(ID_STEDLG_LANG_USERKEYWORD_TEXTCTRL), wxTextCtrl);

    m_helpEditor = new wxSTEditor(m_notebook);
    m_helpEditor->SetWrapMode(wxSTC_WRAP_WORD);
    m_helpEditor->SetText(sm_helpString);
    m_helpEditor->SetReadOnly(true);
    m_notebook->AddPage(m_helpEditor, _("Help"), false);

    // Set the values of the language combo
    m_languageChoice->Clear();
    count = steLangs.GetCount();
    for (n = 0; n < count; n++)
    {
        if (!steLangs.GetUseLanguage(n)) continue;

        m_usedLangs.Add((int)n);
        m_languageChoice->Append(steLangs.GetName(n));
    }

    if (m_usedLangs.Index(m_current_lang) == -1)
    {
        Enable(false);
        wxFAIL_MSG(wxT("Invalid language"));
        return;
    }

    m_languageChoice->SetSelection(m_usedLangs.Index(m_current_lang));

    SetControlValues();
}

void wxSTEditorPrefDialogPageLangs::OnChoice(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case ID_STEDLG_LANG_CHOICE       :
        case ID_STEDLG_LANG_STYLE_CHOICE :
        {
            GetControlValues();
            SetControlValues();
            break;
        }
        case ID_STEDLG_LANG_KEYWORD_CHOICE :
        {
            GetControlValues();
            SetKeywordTextCtrl();
            break;
        }
        default : break;
    }
}

void wxSTEditorPrefDialogPageLangs::GetControlValues()
{
    if (!IsEnabled()) return;

    wxSTEditorLangs steLangs(GetPrefData().GetLangs());
    steLangs.SetUserFilePattern(GetPrefData().GetLanguageId(), m_filepatternTextCtrl->GetValue());

    int ste_style = (long)m_styleChoice->GetClientData(m_styleChoice->GetSelection());
    steLangs.SetUserSTEStyle(m_current_lang, m_current_style_n, ste_style);

    if ((m_keyword_n >= 0) && (m_keyword_n < (int)steLangs.GetKeyWordsCount(m_current_lang)))
        steLangs.SetUserKeyWords(m_current_lang, m_keyword_n, m_userKeywordsTextCtrl->GetValue());

    // Get these after saving the above values
    m_current_lang = m_usedLangs[m_languageChoice->GetSelection()];
    GetPrefData().SetLanguageId(m_current_lang);

    m_keyword_n = m_keywordsChoice->GetSelection();
}

void wxSTEditorPrefDialogPageLangs::SetControlValues()
{
    if (!IsEnabled()) return;

    wxSTEditorLangs  steLangs(GetPrefData().GetLangs());
    wxSTEditorStyles steStyles(GetPrefData().GetStyles());

    m_current_lang = m_usedLangs[m_languageChoice->GetSelection()];
    m_filepatternTextCtrl->SetValue(steLangs.GetFilePattern(m_current_lang));

    m_styleEditor->SetReadOnly(false);
    m_styleEditor->ClearAll();

    // Refill the styles used for this lang
    size_t n, style_count = steLangs.GetStyleCount(m_current_lang);
    for (n = 0; n < style_count; n++)
    {
        int ste_style = steLangs.GetSTEStyle(m_current_lang, n);
        if (ste_style >= 0)
        {
            wxString styleName = steStyles.GetStyleName(ste_style);
            //int word_number = ste_style - STE_STYLE_KEYWORD1;
            //if ((word_number >= 0) && (word_number < steLangs.GetKeyWordsCount(m_current_lang)))
            //    styleName += wxT(" (*)");

            styleName += wxString(wxT(' '), wxMax(1UL, m_max_stylename_length - styleName.Length()));
            styleName += steLangs.GetStyleDescription(m_current_lang, n);

            m_styleEditor->AppendText(styleName + wxT("\n"));
        }
    }

    // style each line
    steStyles.UpdateEditor(m_styleEditor);
    for (n = 0; n < style_count; n++)
    {
        int ste_style = steLangs.GetSTEStyle(m_current_lang, n);
        int sci_style = steLangs.GetSciStyle(m_current_lang, n);
        if ((ste_style < 0) || (sci_style < 0)) continue;

        steStyles.SetEditorStyle(sci_style, ste_style, m_styleEditor);

        int line = (int)n;
        wxString lineStr = m_styleEditor->GetLine(line);

        STE_TextPos pos = m_styleEditor->PositionFromLine(line);
        size_t line_len = lineStr.Length();

        if (line_len <= 2)
            continue;

        m_styleEditor->StartStyling(pos, 0xff);
        m_styleEditor->SetStyling((int)line_len, sci_style);
    }

    m_styleEditor->SetReadOnly(true);

    // delete marker, but try to put it back in same place if possible
    m_styleEditor->MarkerDeleteHandle(m_style_editor_marker_handle);
    m_styleEditor->MarkerDeleteAll(0);
    if (m_current_style_n > (int)style_count) m_current_style_n = 0;
    m_style_editor_marker_handle = m_styleEditor->MarkerAdd(m_current_style_n, 0);

    // put cursor at start of line with the selected style
    m_styleEditor->GotoLine(m_current_style_n);

    // Setup the keyword listbox
    m_keywordsChoice->Clear();
    size_t keywords_count = steLangs.GetKeyWordsCount(m_current_lang);
    for (n = 0; n < keywords_count; n++)
        m_keywordsChoice->Append(wxString::Format(wxT("%d"), (int)(n+1)));

    if (m_keywordsChoice->GetCount() != 0)
        m_keywordsChoice->SetSelection(0);

    SetStylesChoice();
    SetKeywordTextCtrl();
}

void wxSTEditorPrefDialogPageLangs::OnMarginClick(wxStyledTextEvent &event)
{
    if (event.GetEventType() == wxEVT_STEDITOR_MARGINDCLICK)
        return;

    STE_TextPos pos = event.GetPosition();

    if (event.GetEventType() == wxEVT_STC_DOUBLECLICK) // event pos not set correctly
        pos = m_styleEditor->GetCurrentPos();

    int line = m_styleEditor->LineFromPosition(pos);

    if (m_styleEditor->GetLine(line).Strip(wxString::both).IsEmpty())
        return;

    if (m_styleEditor->MarkerLineFromHandle(m_style_editor_marker_handle) != line)
    {
        if ((line < 0) || (line >= (int)GetPrefData().GetLangs().GetStyleCount(m_current_lang)))
            return;

        m_styleEditor->MarkerDeleteAll(0);
        m_style_editor_marker_handle = m_styleEditor->MarkerAdd(line, 0);

        m_current_style_n = line;

        SetStylesChoice();
    }
}

void wxSTEditorPrefDialogPageLangs::SetStylesChoice()
{
    int ste_style = GetPrefData().GetLangs().GetSTEStyle(m_current_lang, m_current_style_n);
    int n, count = m_styleChoice->GetCount();
    for (n = 0; n < count; n++)
    {
        if (long(m_styleChoice->GetClientData(n)) == ste_style)
        {
            m_styleChoice->SetSelection(n);
            break;
        }
    }
}

void wxSTEditorPrefDialogPageLangs::SetKeywordTextCtrl()
{
    m_current_lang = m_usedLangs[m_languageChoice->GetSelection()];

    wxSTEditorLangs steLangs(GetPrefData().GetLangs());

    size_t word_n = m_keywordsChoice->GetSelection();
    if ((word_n < 0) || (word_n >= steLangs.GetKeyWordsCount(m_current_lang)))
    {
        m_keywordsTextCtrl->SetValue(wxEmptyString);
        m_userKeywordsTextCtrl->SetValue(wxEmptyString);
        m_keywordsTextCtrl->Enable(false);
        m_userKeywordsTextCtrl->Enable(false);
        return;
    }

    m_keywordsTextCtrl->Enable(true);
    m_userKeywordsTextCtrl->Enable(true);
    m_keywordsTextCtrl->SetValue(steLangs.GetKeyWords(m_current_lang, word_n, true));
    m_userKeywordsTextCtrl->SetValue(steLangs.GetUserKeyWords(m_current_lang, word_n));
}

void wxSTEditorPrefDialogPageLangs::Apply()
{
    if (!IsEnabled()) return;

    GetControlValues();
    GetEditorPrefData().SetLanguageId(GetPrefData().GetLanguageId());
    GetEditorPrefData().GetLangs().Copy(GetPrefData().GetLangs());
}
void wxSTEditorPrefDialogPageLangs::Reset()
{
    if (!IsEnabled()) return;

    GetPrefData().GetLangs().Reset();
    SetControlValues();
}

bool wxSTEditorPrefDialogPageLangs::IsModified()
{
    GetControlValues();
    return !GetPrefData().GetLangs().IsEqualTo(GetEditorPrefData().GetLangs());
}

//----------------------------------------------------------------------------
// wxSTEditorPrefDialog
//----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorPrefDialog, wxDialog)

BEGIN_EVENT_TABLE(wxSTEditorPrefDialog, wxDialog)
    EVT_BUTTON(wxID_APPLY,  wxSTEditorPrefDialog::OnApply)
    EVT_BUTTON(wxID_CANCEL, wxSTEditorPrefDialog::OnCancel)
    EVT_BUTTON(wxID_OK,     wxSTEditorPrefDialog::OnOk)
    EVT_BUTTON(wxID_RESET,  wxSTEditorPrefDialog::OnReset)

    EVT_NOTEBOOK_PAGE_CHANGED (ID_STEDLG_PREF_NOTEBOOK, wxSTEditorPrefDialog::OnNotebookPageChanged)
    //EVT_UPDATE_UI(wxID_APPLY, wxSTEditorPrefDialog::OnUpdateUIApply)
END_EVENT_TABLE()

extern void LangConfig(); // FIXME test code to dump languages (100Kb!, maybe not so good)

/*static*/ int wxSTEditorPrefDialog::ms_currentpage = 0;

void wxSTEditorPrefDialog::Init()
{
    m_noteBook = NULL;
    m_imageList = new wxImageList(wxSTEIconSize.x, wxSTEIconSize.y);
}

bool wxSTEditorPrefDialog::Create( const wxSTEditorPrefPageData& editorPrefData,
                                   wxWindow *parent, wxWindowID win_id,
                                   long style, const wxString& name )
{
    if (!wxDialog::Create(parent, win_id, _("Editor Preferences"), wxDefaultPosition, wxDefaultSize, style, name))
        return false;

    //LangConfig();
    m_editorPrefData = editorPrefData;
    m_prefData.SetLanguageId(m_editorPrefData.GetLanguageId());
    m_prefData.SetOptions(m_editorPrefData.GetOptions());

    wxCHECK_MSG(m_editorPrefData.GetPrefs().IsOk() ||
                m_editorPrefData.GetStyles().IsOk() ||
                m_editorPrefData.GetLangs().IsOk(), false, wxT("At least one page must be added."));

    if (m_editorPrefData.GetPrefs().IsOk())
        m_prefData.GetPrefs().Copy(m_editorPrefData.GetPrefs());
    if (m_editorPrefData.GetStyles().IsOk())
        m_prefData.GetStyles().Copy(m_editorPrefData.GetStyles());
    if (m_editorPrefData.GetLangs().IsOk())
        m_prefData.GetLangs().Copy(m_editorPrefData.GetLangs());

    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_VIEW,      wxART_TOOLBAR, wxSTEIconSize));
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_TABSEOL,   wxART_TOOLBAR, wxSTEIconSize));
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_FOLDWRAP,  wxART_TOOLBAR, wxSTEIconSize));
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_PRINT,     wxART_TOOLBAR, wxSTEIconSize));
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_LOADSAVE,  wxART_TOOLBAR, wxSTEIconSize));
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_HIGHLIGHT, wxART_TOOLBAR, wxSTEIconSize));
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_STYLES,    wxART_TOOLBAR, wxSTEIconSize));
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_LANGS,     wxART_TOOLBAR, wxSTEIconSize));

    wxBitmap b1 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_VIEW,      wxART_TOOLBAR, wxSTEIconSize);
    wxBitmap b2 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_TABSEOL,   wxART_TOOLBAR, wxSTEIconSize);
    wxBitmap b3 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_FOLDWRAP,  wxART_TOOLBAR, wxSTEIconSize);
    wxBitmap b4 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_PRINT,     wxART_TOOLBAR, wxSTEIconSize);
    wxBitmap b5 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_LOADSAVE,  wxART_TOOLBAR, wxSTEIconSize);
    wxBitmap b6 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_HIGHLIGHT, wxART_TOOLBAR, wxSTEIconSize);
    wxBitmap b7 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_STYLES,    wxART_TOOLBAR, wxSTEIconSize);
    wxBitmap b8 = wxArtProvider::GetBitmap(wxART_STEDIT_PREFDLG_LANGS,     wxART_TOOLBAR, wxSTEIconSize);


    wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxNO_BORDER);
    m_noteBook = new wxListbook(panel, ID_STEDLG_PREF_NOTEBOOK,
                                wxDefaultPosition, wxDefaultSize,
                                0); //wxNB_MULTILINE);
    m_noteBook->SetImageList(m_imageList);

    if (GetPrefData().GetPrefs().IsOk())
    {
        wxSTEditorPrefDialogPagePrefs *page = NULL;

        if (GetPrefData().HasOption(STE_PREF_PAGE_SHOW_VIEW))
        {
            page = new wxSTEditorPrefDialogPagePrefs(GetEditorPrefData(), GetPrefData(), m_noteBook);
            wxSTEditorViewPrefsSizer(page, true, true);
            page->SetControlValues();
            m_noteBook->AddPage(page, _("View"), false, 0);
        }
        if (GetPrefData().HasOption(STE_PREF_PAGE_SHOW_TABSEOL))
        {
            page = new wxSTEditorPrefDialogPagePrefs(GetEditorPrefData(), GetPrefData(), m_noteBook);
            wxSTEditorTabsPrefsSizer(page, true, true);
            page->SetControlValues();
            m_noteBook->AddPage(page, _("Tabs / EOL"), false, 1);
        }
        if (GetPrefData().HasOption(STE_PREF_PAGE_SHOW_FOLDWRAP))
        {
            page = new wxSTEditorPrefDialogPagePrefs(GetEditorPrefData(), GetPrefData(), m_noteBook);
            wxSTEditorFoldPrefsSizer(page, true, true);
            page->SetControlValues();
            m_noteBook->AddPage(page, _("Fold / Wrap"), false, 2);
        }
        if (GetPrefData().HasOption(STE_PREF_PAGE_SHOW_PRINT))
        {
            page = new wxSTEditorPrefDialogPagePrefs(GetEditorPrefData(), GetPrefData(), m_noteBook);
            wxSTEditorPrintPrefsSizer(page, true, true);
            page->SetControlValues();
            m_noteBook->AddPage(page, _("Printing"), false, 3);
        }
        if (GetPrefData().HasOption(STE_PREF_PAGE_SHOW_LOADSAVE))
        {
            page = new wxSTEditorPrefDialogPagePrefs(GetEditorPrefData(), GetPrefData(), m_noteBook);
            wxSTEditorLoadSavePrefsSizer(page, true, true);
            page->SetControlValues();
            m_noteBook->AddPage(page, _("Load / Save"), false, 4);
        }
        if (GetPrefData().HasOption(STE_PREF_PAGE_SHOW_HIGHLIGHT))
        {
            page = new wxSTEditorPrefDialogPagePrefs(GetEditorPrefData(), GetPrefData(), m_noteBook);
            wxSTEditorHighlightingPrefsSizer(page, true, true);
            page->SetControlValues();
            m_noteBook->AddPage(page, _("Highlighting"), false, 5);
        }
    }

    if (GetPrefData().GetStyles().IsOk() && GetPrefData().HasOption(STE_PREF_PAGE_SHOW_STYLES))
    {
        m_noteBook->AddPage(
            new wxSTEditorPrefDialogPageStyles(GetEditorPrefData(), GetPrefData(), m_noteBook),
                            _("Styles"), false, 6);
    }

    if (GetPrefData().GetLangs().IsOk() && GetPrefData().GetStyles().IsOk() &&
        GetPrefData().HasOption(STE_PREF_PAGE_SHOW_LANGS))
    {
        m_noteBook->AddPage(
            new wxSTEditorPrefDialogPageLangs(GetEditorPrefData(), GetPrefData(), m_noteBook),
                            _("Languages"), false, 7);
    }

    m_noteBook->SetSelection(ms_currentpage);
#ifdef __WXMSW__
    // the list is scrolled down a bit initially
    m_noteBook->GetListView()->EnsureVisible(ms_currentpage);
#endif

    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_noteBook, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);

    wxStdDialogButtonSizer* buttonpane = wxSTEditorStdDialogButtonSizer(panel, wxOK | wxCANCEL | wxAPPLY);
    wxButton *resetButton = new wxButton(panel, wxID_RESET, _("Default"));
#if wxUSE_TOOLTIPS
    resetButton->SetToolTip(_("Reset this page's values to their default"));
#endif //wxUSE_TOOLTIPS
    buttonpane->Insert(0, resetButton, 0, wxEXPAND);

    sizer->SetSizeHints(this);
    Centre();
    SetIcons(wxSTEditorArtProvider::GetDialogIconBundle());
    return true;
}

wxSTEditorPrefDialog::~wxSTEditorPrefDialog()
{
    ms_currentpage = m_noteBook->GetSelection();
    delete m_imageList;
}

void wxSTEditorPrefDialog::OnApply(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_inapply);
    if (guard.IsInside()) return;

    size_t n, count = m_noteBook->GetPageCount();
    for (n = 0; n < count; n++)
        m_noteBook->GetPage(n)->GetEventHandler()->ProcessEvent(event);

    // set the language without update since will do it later
    if (GetEditorPrefData().GetEditor())
        GetEditorPrefData().GetEditor()->GetSTERefData()->SetLanguage(GetEditorPrefData().GetLanguageId());
        //GetEditorPrefData().GetEditor()->SetLanguage(GetEditorPrefData().GetLanguageId());

    if (GetEditorPrefData().GetStyles().IsOk())
        GetEditorPrefData().GetStyles().UpdateAllEditors();
    if (GetEditorPrefData().GetPrefs().IsOk())
        GetEditorPrefData().GetPrefs().UpdateAllEditors();
    if (GetEditorPrefData().GetLangs().IsOk())
        GetEditorPrefData().GetLangs().UpdateAllEditors();
}

void wxSTEditorPrefDialog::OnCancel(wxCommandEvent &WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void wxSTEditorPrefDialog::OnOk(wxCommandEvent &event)
{
    OnApply(event);
    EndModal(wxID_OK);
}

void wxSTEditorPrefDialog::OnReset(wxCommandEvent &event)
{
    static wxSTERecursionGuardFlag s_flag;
    wxSTERecursionGuard guard(s_flag);
    if (guard.IsInside())
    {
        event.Skip();
        return;
    }

    int n = m_noteBook->GetSelection();
    m_noteBook->GetPage(n)->GetEventHandler()->ProcessEvent(event);
}

void wxSTEditorPrefDialog::OnNotebookPageChanged(wxNotebookEvent &)
{
    wxWindow *win = m_noteBook->GetPage(m_noteBook->GetSelection());

    // give the styles a kick in case the languageID changed
    if (win && wxDynamicCast(win, wxSTEditorPrefDialogPageStyles))
        wxDynamicCast(win, wxSTEditorPrefDialogPageStyles)->SetControlValues();
    // give the langs a kick in case the styles have changed
    if (win && wxDynamicCast(win, wxSTEditorPrefDialogPageLangs))
        wxDynamicCast(win, wxSTEditorPrefDialogPageLangs)->SetControlValues();
}

void wxSTEditorPrefDialog::OnUpdateUIApply(wxUpdateUIEvent& event)
{
    wxWindow *win = m_noteBook->GetPage(m_noteBook->GetSelection());

    bool modified = true;

    //static long i = 0;
    //wxPrintf(wxT("UpdateUI %d\n"), i++); fflush(stdout);

    if (win && wxDynamicCast(win, wxSTEditorPrefDialogPagePrefs))
        modified = wxDynamicCast(win, wxSTEditorPrefDialogPagePrefs)->IsModified();
    else if (win && wxDynamicCast(win, wxSTEditorPrefDialogPageStyles))
        modified = wxDynamicCast(win, wxSTEditorPrefDialogPageStyles)->IsModified();
    else if (win && wxDynamicCast(win, wxSTEditorPrefDialogPageLangs))
        modified = wxDynamicCast(win, wxSTEditorPrefDialogPageLangs)->IsModified();

    event.Enable(modified);
}

//-----------------------------------------------------------------------------
// wxSTEditorPropertiesDialog - display info about editor session
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorPropertiesDialog, wxDialog);

#define SET_STATTEXT(win_id,val) wxStaticCast(FindWindow(win_id),wxStaticText)->SetLabel(val)

BEGIN_EVENT_TABLE(wxSTEditorPropertiesDialog, wxDialog)
    EVT_UPDATE_UI(ID_STEPROP_ENCODING_CHOICE,       wxSTEditorPropertiesDialog::OnUpdateNeedEditable)
    EVT_UPDATE_UI(ID_STEPROP_ENCODING_BOM_CHECKBOX, wxSTEditorPropertiesDialog::OnUpdateBomCheckBox)
END_EVENT_TABLE()

wxSTEditorPropertiesDialog::wxSTEditorPropertiesDialog(wxSTEditor* editor)
                           :wxDialog(),
                            m_editor(editor),
                            m_encoding(wxTextEncoding::TypeFromString(editor->GetFileEncoding())),
                            m_bom(editor->GetFileBOM())
{
}

bool wxSTEditorPropertiesDialog::Create(wxWindow* parent,
                                        const wxString& title,
                                        long style)
{
    if (!wxDialog::Create(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, style))
        return false;

    SetIcons(wxSTEditorArtProvider::GetDialogIconBundle());
    wxSTEditorPropertiesSizer(this, true, true);

    wxSTEditorStdDialogButtonSizer(this, wxCANCEL | (IsEditable() ? wxOK : 0));

    // File Properties panel

    const wxFileName fileName = m_editor->GetFileName();

    wxTextCtrl *textCtrl = wxStaticCast(FindWindow(ID_STEPROP_FILENAME_TEXTCTRL), wxTextCtrl);
    textCtrl->SetValue(fileName.GetFullPath(m_editor->GetOptions().GetDisplayPathSeparator()));

    wxDateTime dtOpened, dtAccessed, dtModified, dtCreated;
    wxString strSize;
    if (m_editor->IsFileFromDisk())
    {
        fileName.GetTimes(&dtAccessed, &dtModified, &dtCreated);
        wxULongLong size = fileName.GetSize();
        strSize = wxString::Format(_("%s bytes"), size.ToString().wx_str());

        if (size.GetValue() >= 1024)
        {
            strSize = wxString::Format(wxT("%s (%s)"),
                                        wxFileName::GetHumanReadableSize(size).wx_str(),
                                        strSize.wx_str());
        }
    }
    else
    {
        strSize = _("<Unknown>");
    }
    SET_STATTEXT(ID_STEPROP_FILESIZE_TEXT, strSize);

    dtOpened = m_editor->GetFileModificationTime();
    SET_STATTEXT(ID_STEPROP_FILEOPENED_TEXT  , dtOpened  .IsValid() ? dtOpened  .Format() : wxString(_("Not originally loaded from disk")));
    SET_STATTEXT(ID_STEPROP_FILEMODIFIED_TEXT, dtModified.IsValid() ? dtModified.Format() : wxString(_("<Unknown>")));
    SET_STATTEXT(ID_STEPROP_FILEACCESSED_TEXT, dtAccessed.IsValid() ? dtAccessed.Format() : wxString(_("<Unknown>")));
    SET_STATTEXT(ID_STEPROP_FILECREATED_TEXT , dtCreated .IsValid() ? dtCreated .Format() : wxString(_("<Unknown>")));

    // Information panel

    SET_STATTEXT(ID_STEPROP_LANGUAGE_TEXT, m_editor->GetEditorLangs().IsOk() ? m_editor->GetEditorLangs().GetName(m_editor->GetLanguageId()) : wxString(_("<Unknown>")));

    wxChoice* encodingChoice = wxStaticCast(FindWindow(ID_STEPROP_ENCODING_CHOICE), wxChoice);
    encodingChoice->SetValidator(wxGenericValidator(&m_encoding));
    wxStaticCast(FindWindow(ID_STEPROP_ENCODING_BOM_CHECKBOX), wxCheckBox)->SetValidator(wxGenericValidator(&m_bom));

    for (int i = 0; i < wxTextEncoding::TextEncoding__Count; i++)
    {
        encodingChoice->Append(wxTextEncoding::TypeToString((wxTextEncoding::TextEncoding_Type)i));
    }

    // Current Statistics

    SET_STATTEXT(ID_STEPROP_NUMLINES_TEXT, wxString::Format(wxT("%d"), m_editor->GetLineCount()));
    SET_STATTEXT(ID_STEPROP_NUMCHARS_TEXT, wxString::Format(wxT("%d"), m_editor->GetTextLength()));
    SET_STATTEXT(ID_STEPROP_NUMWORDS_TEXT, wxString::Format(wxT("%d"), (int)m_editor->GetWordCount()));

    wxString mimeStr;
#if wxUSE_MIMETYPE
    wxString ext = fileName.GetExt();
    if (!ext.IsEmpty())
    {
        wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);

        if (ft)
        {
            ft->GetMimeType(&mimeStr);
            delete ft;
        }
    }
#endif
    SET_STATTEXT(ID_STEPROP_FILE_TYPE_TEXT, mimeStr);

    int crlf = 0, cr = 0, lf = 0, tabs = 0;
    m_editor->GetEOLCount(&crlf, &cr, &lf, &tabs);
    SET_STATTEXT(ID_STEPROP_NUMTABS_TEXT, wxString::Format(wxT("%d"), tabs));

    wxString eolStr;
    if (crlf > 0)
        eolStr += wxString::Format(wxT("CRLF (DOS/Win)=%d"), crlf);
    if (cr > 0)
    {
        if (crlf > 0)
            eolStr += wxT(", ");

        eolStr += wxString::Format(wxT("CR (Mac)=%d"), cr);
    }
    if (lf > 0)
    {
        if ((crlf > 0) || (cr > 0))
            eolStr += wxT(", ");

        eolStr += wxString::Format(wxT("LF (Unix)=%d"), lf);
    }
    if (eolStr.IsEmpty())
        eolStr = _("none");

    SET_STATTEXT(ID_STEPROP_EOLCHARS_TEXT, eolStr);

    TransferDataToWindow();
    Fit();
    GetSizer()->SetSizeHints(this);
    Centre();

    return true;
}

bool wxSTEditorPropertiesDialog::TransferDataFromWindow()
{
    bool ok = wxDialog::TransferDataFromWindow();

    if (ok)
    {
        wxASSERT(IsEditable());
        // do not actually store the values until certain that
        // all went right - it did if we get here
        m_editor->SetFileEncoding(wxTextEncoding::TypeToString((wxTextEncoding::TextEncoding_Type)(m_encoding)));
        m_editor->SetFileBOM(m_bom);
        m_editor->SetModified(true);
    }
    return ok;
}

void wxSTEditorPropertiesDialog::OnUpdateNeedEditable(wxUpdateUIEvent& event)
{
    event.Enable(IsEditable());
}

static bool EnableBomCheckBox(wxChoice* list, wxCheckBox* checkbox)
{
    int enc = list->GetSelection();
    bool bom_current = checkbox->IsChecked();
    size_t count;
    bool bom = (wxTextEncoding::GetBOMChars((wxTextEncoding::TextEncoding_Type)enc, &count) != NULL);

    if (bom_current && !bom)
    {
        checkbox->SetValue(false);
    }
    return bom;
}

void wxSTEditorPropertiesDialog::OnUpdateBomCheckBox(wxUpdateUIEvent& event)
{
    bool bom = ::EnableBomCheckBox(wxStaticCast(FindWindow(ID_STEPROP_ENCODING_CHOICE), wxChoice), wxStaticCast(FindWindow(ID_STEPROP_ENCODING_BOM_CHECKBOX), wxCheckBox));

    event.Enable(m_editor->IsEditable() && bom);
}

//-----------------------------------------------------------------------------
// wxSTEditorWindowsDialog
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorWindowsDialog, wxDialog);

BEGIN_EVENT_TABLE(wxSTEditorWindowsDialog, wxDialog)
    EVT_LISTBOX(       ID_STEDLG_WINDOWS_LISTBOX, wxSTEditorWindowsDialog::OnListBox)
    EVT_LISTBOX_DCLICK(ID_STEDLG_WINDOWS_LISTBOX, wxSTEditorWindowsDialog::OnListBox)

    EVT_BUTTON(ID_STEDLGS_WINDOWS_ACTIVATE_BUTTON, wxSTEditorWindowsDialog::OnButton)
    EVT_BUTTON(ID_STEDLGS_WINDOWS_SAVE_BUTTON,     wxSTEditorWindowsDialog::OnButton)
    EVT_BUTTON(ID_STEDLGS_WINDOWS_CLOSE_BUTTON,    wxSTEditorWindowsDialog::OnButton)
END_EVENT_TABLE()

wxSTEditorWindowsDialog::wxSTEditorWindowsDialog(wxSTEditorNotebook *notebook,
                                                 const wxString& title,
                                                 long style)
                        :wxDialog()
{
    m_notebook = notebook;
    m_listBox = NULL;
    wxCHECK_RET(m_notebook, wxT("Invalid parent"));

    if (!wxDialog::Create(notebook, wxID_ANY, title,
                          wxDefaultPosition, wxDefaultSize, style))
        return;

    wxSTEditorWindowsSizer(this, true, true);

    m_listBox = wxStaticCast(FindWindow(ID_STEDLG_WINDOWS_LISTBOX), wxListBox);

    UpdateListBox();
    m_listBox->SetSelection(m_notebook->GetSelection());
    UpdateButtons();
    Centre();
    SetIcons(wxSTEditorArtProvider::GetDialogIconBundle());
    ShowModal();
}

void wxSTEditorWindowsDialog::UpdateListBox()
{
    m_listBox->Clear();
    int n, count = (int)m_notebook->GetPageCount();
    for (n = 0; n < count; n++)
    {
        wxSTEditor* editor = m_notebook->GetEditor(n);
        wxString text = wxString::Format(wxT("%3d : "), n+1);
        // they can stick in different windows in notebook if they like
        if (editor)
            m_listBox->Append(text+editor->GetFileName().GetFullPath());
        else
            m_listBox->Append(text+m_notebook->GetPageText(n));
    }
}

void wxSTEditorWindowsDialog::UpdateButtons()
{
    wxArrayInt selections;
    int count = m_listBox->GetSelections(selections);
    bool enable_activate = count != 0;
    bool enable_save     = count != 0;
    bool enable_close    = count != 0;
    FindWindow(ID_STEDLGS_WINDOWS_ACTIVATE_BUTTON)->Enable(enable_activate);
    FindWindow(ID_STEDLGS_WINDOWS_SAVE_BUTTON    )->Enable(enable_save);
    FindWindow(ID_STEDLGS_WINDOWS_CLOSE_BUTTON   )->Enable(enable_close);
}

void wxSTEditorWindowsDialog::OnListBox(wxCommandEvent& event)
{
    // activate selection and close dialog
    if (event.GetEventType() == wxEVT_COMMAND_LISTBOX_DOUBLECLICKED)
    {
        if (event.GetSelection() != -1)
        {
            m_notebook->SetSelection(event.GetSelection());
            EndModal(wxID_OK);
        }
        return;
    }

    UpdateButtons();
    event.Skip();
}

void wxSTEditorWindowsDialog::OnButton(wxCommandEvent& event)
{
    wxArrayInt selections;
    int n, count = m_listBox->GetSelections(selections);

    // shouldn't get here, but just in case reset buttons
    if (count == 0)
    {
        UpdateButtons();
        return;
    }

    switch (event.GetId())
    {
        case ID_STEDLGS_WINDOWS_ACTIVATE_BUTTON :
        {
            // only activate the first selection
            m_notebook->SetSelection(selections[0]);
            EndModal(wxID_OK);
            break;
        }
        case ID_STEDLGS_WINDOWS_SAVE_BUTTON :
        {
            for (n = 0; n < count; n++)
            {
                wxSTEditor* editor = m_notebook->GetEditor(selections[n]);
                if (editor)
                    editor->SaveFile(false);
            }

            break;
        }
        case ID_STEDLGS_WINDOWS_CLOSE_BUTTON :
        {
            // delete the windows from the end
            for (n = count - 1; n >= 0; n--)
            {
                wxSTEditor* editor = m_notebook->GetEditor(selections[n]);
                if (editor)
                    m_notebook->ClosePage(selections[n]);
            }

            UpdateListBox();
            break;
        }
        default : break;
    }
}

//-----------------------------------------------------------------------------
// wxSTEditorBookmarkDialog
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorBookmarkDialog, wxDialog);

wxPoint wxSTEditorBookmarkDialog::ms_dialogPosition(wxDefaultPosition);
wxSize  wxSTEditorBookmarkDialog::ms_dialogSize(550, 400);

BEGIN_EVENT_TABLE(wxSTEditorBookmarkDialog, wxDialog)
    EVT_TREE_ITEM_ACTIVATED (wxID_ANY, wxSTEditorBookmarkDialog::OnTreeCtrl)
    EVT_TREE_SEL_CHANGED    (wxID_ANY, wxSTEditorBookmarkDialog::OnTreeCtrl)

    EVT_BUTTON(ID_STEDLGS_BOOKMARKS_GOTO_BUTTON,   wxSTEditorBookmarkDialog::OnButton)
    EVT_BUTTON(ID_STEDLGS_BOOKMARKS_DELETE_BUTTON, wxSTEditorBookmarkDialog::OnButton)
END_EVENT_TABLE()

wxSTEditorBookmarkDialog::wxSTEditorBookmarkDialog(wxWindow *win,
                                                   const wxString& title,
                                                   long style)
                         :wxDialog()
{
    m_notebook = NULL;
    m_editor   = NULL;
    m_treeCtrl = NULL;

    wxWindow* parent = win; // what parent to use for this dialog

    if (wxDynamicCast(win, wxSTEditor) != NULL)
    {
        m_editor = wxDynamicCast(win, wxSTEditor);

        // Try to find a wxSTEditorNotebook parent since we can then show
        // all the bookmarks.
        wxWindow *p = win->GetParent();
        while (p)
        {
            if (wxDynamicCast(p, wxSTEditorNotebook) != NULL)
            {
                m_notebook = wxDynamicCast(p, wxSTEditorNotebook);
                parent = m_notebook;
                break;
            }

            p = p->GetParent();
        }
    }
    else if (wxDynamicCast(win, wxSTEditorNotebook) != NULL)
    {
        m_notebook = wxDynamicCast(win, wxSTEditorNotebook);
    }

    if (!wxDialog::Create(parent, wxID_ANY, title, ms_dialogPosition, ms_dialogSize, style))
        return;

    wxCHECK_RET(m_notebook || m_editor, wxT("Invalid parent, must be a wxSTEditorNotebook or a wxSTEditor."));

    wxSTEditorBookmarkSizer(this, true, true);

    m_treeCtrl = wxStaticCast(FindWindow(ID_STEDLGS_BOOKMARKS_TREECTRL), wxTreeCtrl);

    wxImageList* imageList = new wxImageList(16, 16, true, 2);
    imageList->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE,  wxART_MENU, wxSize(16, 16)));
    imageList->Add(wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_MENU, wxSize(16, 16)));
    m_treeCtrl->AssignImageList(imageList);

    UpdateTreeCtrl();
    UpdateButtons();

    if (ms_dialogPosition == wxDefaultPosition)
        Centre();

    ShowModal();
}

wxSTEditorBookmarkDialog::~wxSTEditorBookmarkDialog()
{
    ms_dialogPosition = GetPosition();
    ms_dialogSize     = GetSize();
}

void wxSTEditorBookmarkDialog::UpdateTreeCtrl()
{
    m_treeCtrl->DeleteAllItems();

    wxTreeItemId rootId = m_treeCtrl->AddRoot(wxT("root"));
    wxTreeItemId selectedId;

    int n, count      = m_notebook ? (int)m_notebook->GetPageCount() : 0;
    int selected_page = m_notebook ? (int)m_notebook->GetSelection() : -1;

    for (n = 0; n < count; n++)
    {
        wxSTEditor* editor = m_notebook ? m_notebook->GetEditor(n) : m_editor;
        if (editor == NULL) continue;

        int line = editor->MarkerNext(0, 1<<STE_MARKER_BOOKMARK);
        wxTreeItemId editorId;

        while (line != -1)
        {
            if (!editorId)
            {
                wxString s(wxString::Format(wxT("%-5d : "), n+1) + editor->GetFileName().GetFullPath());
                editorId = m_treeCtrl->AppendItem(rootId, s, 0, -1, NULL);
                m_treeCtrl->SetItemTextColour(editorId, *wxBLUE);

                if (n == selected_page)
                    selectedId = editorId;
            }

            wxString s(wxString::Format(wxT("%-5d : "), line+1) + editor->GetLineText(line));
            if (s.Length() > 100) s = s.Mid(0, 100) + wxT("...");
            wxTreeItemId id = m_treeCtrl->AppendItem(editorId, s, 1, -1, NULL);

            if ((n == selected_page) && (line == editor->GetCurrentLine()))
                selectedId = id;

            line = editor->MarkerNext(line+1, 1<<STE_MARKER_BOOKMARK);
        }
    }

    m_treeCtrl->ExpandAll();
    if (selectedId)
        m_treeCtrl->SelectItem(selectedId);
}

bool wxSTEditorBookmarkDialog::GetItemInfo(const wxTreeItemId& id,
                                           long& notebook_page, long& bookmark_line)
{
    notebook_page = -1;
    bookmark_line = -1;

    if (!id) return false;

    wxTreeItemId parentId = m_treeCtrl->GetItemParent(id);
    // This is a file, not a bookmark, if the parent is the root
    if (parentId == m_treeCtrl->GetRootItem()) return false;

    if (m_treeCtrl->GetItemText(parentId).BeforeFirst(wxT(' ')).Trim(false).ToLong(&notebook_page) &&
        m_treeCtrl->GetItemText(id).BeforeFirst(wxT(' ')).Trim(false).ToLong(&bookmark_line) )
    {
        notebook_page--; // make 0 based
        bookmark_line--;
    }

    return (bookmark_line != -1);
}

void wxSTEditorBookmarkDialog::UpdateButtons()
{
    wxTreeItemId id;
    wxArrayTreeItemIds selectedIds;
    size_t count = m_treeCtrl->GetSelections(selectedIds);

    if (count == 1)
        id = selectedIds[0];

    // is this a filename or a bookmark
    if (id && (m_treeCtrl->GetItemParent(id) == m_treeCtrl->GetRootItem()))
        id = wxTreeItemId();

    // See if anything selected can be deleted, at least one bookmark selected.
    // Files cannot be deleted or gone to, unselect them.
    bool can_delete = false;
    for (size_t n = 0; n < count; ++n)
    {
        long notebook_page = -1;
        long bookmark_line = -1;
        GetItemInfo(selectedIds[n], notebook_page, bookmark_line);

        if (bookmark_line != -1)
            can_delete = true;
        else
            m_treeCtrl->SelectItem(selectedIds[n], false);
    }

    FindWindow(ID_STEDLGS_BOOKMARKS_GOTO_BUTTON)->Enable(id.IsOk());
    FindWindow(ID_STEDLGS_BOOKMARKS_DELETE_BUTTON)->Enable(can_delete);
}

void wxSTEditorBookmarkDialog::OnTreeCtrl(wxTreeEvent& event)
{
    wxTreeItemId id;
    long notebook_page = -1;
    long bookmark_line = -1;

    wxArrayTreeItemIds selectedIds;
    size_t count = m_treeCtrl->GetSelections(selectedIds);

    if (count == 1)
        GetItemInfo(selectedIds[0], notebook_page, bookmark_line);

    // activate selection and close dialog
    if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_ACTIVATED)
    {
        wxCommandEvent buttonEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                   ID_STEDLGS_BOOKMARKS_GOTO_BUTTON);
        OnButton(buttonEvent);
    }
    else if (event.GetEventType() == wxEVT_COMMAND_TREE_SEL_CHANGED)
    {
        if (bookmark_line != -1)
        {
            if (m_notebook)
            {
                m_notebook->SetSelection(notebook_page);
                m_notebook->GetEditor(notebook_page)->GotoLine(bookmark_line);
            }
            else if (m_editor)
                m_editor->GotoLine(bookmark_line);
        }
    }

    UpdateButtons();
    event.Skip();
}

void wxSTEditorBookmarkDialog::OnButton(wxCommandEvent& event)
{
    wxTreeItemId id;
    long notebook_page = -1;
    long bookmark_line = -1;

    wxArrayTreeItemIds selectedIds;
    size_t count = m_treeCtrl->GetSelections(selectedIds);

    if (count > 0)
        GetItemInfo(selectedIds[0], notebook_page, bookmark_line);

    // shouldn't get here, but just in case reset buttons
    if (count == 0)
    {
        UpdateButtons();
        return;
    }

    switch (event.GetId())
    {
        case ID_STEDLGS_BOOKMARKS_GOTO_BUTTON :
        {
            if (bookmark_line != -1)
            {
                if (m_notebook)
                {
                    m_notebook->SetSelection(notebook_page);
                    m_notebook->GetEditor(notebook_page)->GotoLine(bookmark_line);
                }
                else if (m_editor)
                    m_editor->GotoLine(bookmark_line);

                EndModal(wxID_OK);
            }
            break;
        }
        case ID_STEDLGS_BOOKMARKS_DELETE_BUTTON :
        {
            for (size_t n = 0; n < count; ++n)
            {
                id = selectedIds[n];
                GetItemInfo(id, notebook_page, bookmark_line);

                if (bookmark_line == -1) continue;

                if (m_notebook)
                {
                    m_notebook->GetEditor(notebook_page)->MarkerDelete(bookmark_line, STE_MARKER_BOOKMARK);
                }
                else if (m_editor)
                    m_editor->MarkerDelete(bookmark_line, STE_MARKER_BOOKMARK);

                if (m_treeCtrl->GetChildrenCount(m_treeCtrl->GetItemParent(id)) > 1)
                    m_treeCtrl->Delete(id);
                else
                    m_treeCtrl->Delete(m_treeCtrl->GetItemParent(id));
            }

            break;
        }
        default : break;
    }

    UpdateButtons();
}

//-----------------------------------------------------------------------------
// wxSTEditorInsertTextDialog
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorInsertTextDialog, wxDialog);

int wxSTEditorInsertTextDialog::sm_radioID   = ID_STEDLG_INSERT_PREPEND_RADIOBUTTON;
int wxSTEditorInsertTextDialog::sm_spinValue = 1;
wxArrayString wxSTEditorInsertTextDialog::sm_prependValues;
wxArrayString wxSTEditorInsertTextDialog::sm_appendValues;

BEGIN_EVENT_TABLE(wxSTEditorInsertTextDialog, wxDialog)
    EVT_BUTTON     (wxID_ANY, wxSTEditorInsertTextDialog::OnButton)
    EVT_MENU       (wxID_ANY, wxSTEditorInsertTextDialog::OnMenu)
    EVT_RADIOBUTTON(wxID_ANY, wxSTEditorInsertTextDialog::OnRadioButton)
    EVT_TEXT       (wxID_ANY, wxSTEditorInsertTextDialog::OnText)

    EVT_IDLE       (wxSTEditorInsertTextDialog::OnIdle)
END_EVENT_TABLE()

void wxSTEditorInsertTextDialog::Init()
{
    m_editor             = NULL;
    m_editor_sel_start   = 0;
    m_editor_sel_end     = 0;

    m_prependCombo       = NULL;
    m_appendCombo        = NULL;
    m_prependText        = NULL;
    m_insertMenu         = NULL;
    m_testEditor         = NULL;
    m_insert_type        = RadioIdToType(sm_radioID);
    m_column             = sm_spinValue;
    m_prepend_insert_pos = 0;
    m_append_insert_pos  = 0;
    m_created            = false;
}

wxSTEditorInsertTextDialog::wxSTEditorInsertTextDialog(wxSTEditor* editor,
                                                       long style)
{
    Init();

    if (!wxDialog::Create(editor, wxID_ANY, _("Insert Text"), wxDefaultPosition, wxDefaultSize, style))
        return;

    SetIcons(wxSTEditorArtProvider::GetDialogIconBundle());

    m_editor = editor;
    m_editor_sel_start = m_editor->GetSelectionStart();
    m_editor_sel_end   = m_editor->GetSelectionEnd();

    m_testEditor = new wxSTEditor(this, ID_STEDLG_INSERT_EDITOR,
                                  wxDefaultPosition, wxSize(400, 200));

    wxSTEditorInsertTextSizer(this, true, true);
    wxSTEditorStdDialogButtonSizer(this, wxOK | wxCANCEL);

    m_prependText  = wxStaticCast(FindWindow(ID_STEDLG_INSERT_PREPEND_TEXT), wxStaticText);
    m_prependCombo = wxStaticCast(FindWindow(ID_STEDLG_INSERT_PREPEND_COMBO), wxComboBox);
    m_appendCombo  = wxStaticCast(FindWindow(ID_STEDLG_INSERT_APPEND_COMBO), wxComboBox);
    m_prependCombo->Clear();
    m_appendCombo->Clear();

    m_insertMenu = wxSTEditorMenuManager::CreateInsertCharsMenu(NULL, STE_MENU_INSERTCHARS_CHARS);

    wxSTEInitComboBoxStrings(sm_prependValues, m_prependCombo);
    wxSTEInitComboBoxStrings(sm_appendValues,  m_appendCombo);

    m_prependString = m_prependCombo->GetValue();
    m_appendString  = m_appendCombo->GetValue();

    wxStaticCast(FindWindow(ID_STEDLG_INSERT_COLUMN_SPINCTRL), wxSpinCtrl)->SetValue(m_column);
    wxStaticCast(FindWindow(sm_radioID), wxRadioButton)->SetValue(true);

    InitFromEditor();

    Fit();
    GetSizer()->SetSizeHints(this);
    Centre();

    m_created = true;  // now we can handle events

    UpdateControls();
}

wxSTEditorInsertTextDialog::~wxSTEditorInsertTextDialog()
{
    delete m_insertMenu;
}

bool wxSTEditorInsertTextDialog::InitFromEditor()
{
    int line_start = m_editor->LineFromPosition(m_editor_sel_start);
    int line_end   = m_editor->LineFromPosition(m_editor_sel_end);

    // Works on whole lines
    if (1 || (line_start != line_end))
    {
        m_editor_sel_start = m_editor->PositionFromLine(line_start);
        m_editor_sel_end   = m_editor->GetLineEndPosition(line_end);
        m_editor->SetSelection(m_editor_sel_start, m_editor_sel_end);
    }

    wxString initText = m_editor->GetSelectedText();

    m_testEditor->RegisterStyles(m_editor->GetEditorStyles());
    m_testEditor->RegisterLangs(m_editor->GetEditorLangs());
    m_testEditor->SetLanguage(m_editor->GetLanguageId());
    SetText(initText);

    return !initText.IsEmpty();
}

bool wxSTEditorInsertTextDialog::InsertIntoEditor()
{
    switch (m_insert_type)
    {
        case STE_INSERT_TEXT_PREPEND  : return m_editor->InsertTextAtCol(0, m_prependString);
        case STE_INSERT_TEXT_APPEND   : return m_editor->InsertTextAtCol(-1, m_appendString);
        case STE_INSERT_TEXT_ATCOLUMN : return m_editor->InsertTextAtCol(GetColumn(), m_prependString);
        case STE_INSERT_TEXT_SURROUND :
        {
            if (m_appendString.Length() > 0u)
                m_editor->InsertText(m_editor_sel_end, m_appendString);
            if (m_prependString.Length() > 0u)
                m_editor->InsertText(m_editor_sel_start, m_prependString);

            m_editor_sel_start -= (int)m_prependString.Length();
            m_editor_sel_end   += (int)m_prependString.Length();
            m_editor->SetSelection(m_editor_sel_start, m_editor_sel_end);
            return true;
        }
        default : break;
    }

    return false;
}

void wxSTEditorInsertTextDialog::SetText(const wxString& text)
{
    m_initText = text;
    m_testEditor->SetReadOnly(false);
    m_testEditor->SetText(m_initText);
    m_testEditor->SetReadOnly(true);
    FormatText();
}

wxString wxSTEditorInsertTextDialog::GetText()
{
    return m_testEditor->GetText();
}

void wxSTEditorInsertTextDialog::FormatText()
{
    UpdateControls(); // also gets values

    m_testEditor->SetReadOnly(false);
    m_testEditor->SetText(m_initText);
    m_testEditor->SetSelection(0, m_testEditor->GetLength());

    switch (m_insert_type)
    {
        case STE_INSERT_TEXT_PREPEND  :
        {
            m_testEditor->InsertTextAtCol(0, m_prependString);
            break;
        }
        case STE_INSERT_TEXT_APPEND   :
        {
            m_testEditor->InsertTextAtCol(-1, m_appendString);
            break;
        }
        case STE_INSERT_TEXT_ATCOLUMN :
        {
            m_testEditor->InsertTextAtCol(GetColumn(), m_prependString);
            break;
        }
        case STE_INSERT_TEXT_SURROUND :
        {
            STE_TextPos sel_start = 0; //GetSelectionStart();
            STE_TextPos sel_end   = m_testEditor->GetLength(); //GetSelectionEnd();

            if (m_appendString.Length() > 0u)
                m_testEditor->InsertText(sel_end, m_appendString);
            if (m_prependString.Length() > 0u)
                m_testEditor->InsertText(sel_start, m_prependString);

            sel_start -= (STE_TextPos)m_prependString.Length();
            sel_end   += (STE_TextPos)m_prependString.Length();
            m_testEditor->SetSelection(sel_start, sel_end);
            break;
        }
        default : break;
    }

    m_testEditor->SetSelection(0,0);
    m_testEditor->SetReadOnly(true);
}

void wxSTEditorInsertTextDialog::OnButton(wxCommandEvent& event)
{
    if (!m_created) return;

    switch (event.GetId())
    {
        case ID_STEDLG_INSERT_PREPEND_BITMAPBUTTON :
        {
            // set the clientdata of the menu to the combo it's for, see OnMenu
            wxRect r = wxStaticCast(event.GetEventObject(), wxWindow)->GetRect();
            m_insertMenu->SetClientData((void*)m_prependCombo);
            PopupMenu(m_insertMenu, r.GetRight(), r.GetTop());
            break;
        }
        case ID_STEDLG_INSERT_APPEND_BITMAPBUTTON :
        {
            wxRect r = wxStaticCast(event.GetEventObject(), wxWindow)->GetRect();
            m_insertMenu->SetClientData((void*)m_appendCombo);
            PopupMenu(m_insertMenu, r.GetRight(), r.GetTop());
            break;
        }
        case wxID_OK     :
        {
            // only remember these if they pressed OK
            sm_radioID   = GetSelectedRadioId();
            sm_spinValue = m_column;

            if (m_prependString.Length())
                wxSTEPrependArrayString(m_prependString, sm_prependValues, 10);
            if (m_appendString.Length())
                wxSTEPrependArrayString(m_appendString, sm_appendValues, 10);

            InsertIntoEditor();
            break;
        }
        default : break;
    }

    FormatText();
    event.Skip(); // let default processing occur to dismiss the dialog
}

void wxSTEditorInsertTextDialog::OnMenu(wxCommandEvent& event)
{
    if (!m_created) return;
    wxString c;

    switch (event.GetId())
    {
        case ID_STEDLG_INSERTMENU_TAB : c = wxT("\t"); break;
        case ID_STEDLG_INSERTMENU_CR  : c = wxT("\r"); break;
        case ID_STEDLG_INSERTMENU_LF  : c = wxT("\n"); break;
        default : break;
    }

    if (c.Length())  // this must have been for the m_insertMenu
    {
        wxComboBox* cBox = wxStaticCast(m_insertMenu->GetClientData(), wxComboBox);
        wxCHECK_RET(cBox, wxT("Unexpected missing control"));
        wxTextPos pos = (cBox == m_prependCombo) ? m_prepend_insert_pos : m_append_insert_pos;

        wxString s = cBox->GetValue();

        if (pos >= int(s.Length()))
            s += c;
        else if (pos == 0)
            s = c + s;
        else
            s = s.Mid(0, pos) + c + s.Mid(pos);

        cBox->SetValue(s);
        cBox->SetFocus();
        cBox->SetInsertionPoint(pos + (wxTextPos)c.Length());
    }

    FormatText();
}

void wxSTEditorInsertTextDialog::OnRadioButton(wxCommandEvent& )
{
    if (!m_created) return;
    FormatText();
}

void wxSTEditorInsertTextDialog::OnIdle(wxIdleEvent &event)
{
    if (!m_created) return;
    if (IsShown())
    {
        // This is a really ugly hack because the combo forgets its insertion
        //   point in MSW whenever it loses focus
        wxWindow* focus = FindFocus();
        if (m_prependCombo && (focus == m_prependCombo))
            m_prepend_insert_pos = m_prependCombo->GetInsertionPoint();
        if (m_appendCombo && (focus == m_appendCombo))
            m_append_insert_pos = m_appendCombo->GetInsertionPoint();
    }

    event.Skip();
}

void wxSTEditorInsertTextDialog::OnText(wxCommandEvent& event)
{
    if (!m_created) return;
    event.Skip();
    FormatText();
}

void wxSTEditorInsertTextDialog::UpdateControls()
{
    if (!m_created) return;
    m_prependString = m_prependCombo->GetValue();
    m_appendString  = m_appendCombo->GetValue();
    m_column        = wxStaticCast(FindWindow(ID_STEDLG_INSERT_COLUMN_SPINCTRL), wxSpinCtrl)->GetValue();
    m_insert_type   = RadioIdToType(GetSelectedRadioId());

    m_prependCombo->Enable((m_insert_type == STE_INSERT_TEXT_PREPEND) ||
                           (m_insert_type == STE_INSERT_TEXT_ATCOLUMN) ||
                           (m_insert_type == STE_INSERT_TEXT_SURROUND));
    m_appendCombo->Enable ((m_insert_type == STE_INSERT_TEXT_APPEND) ||
                           (m_insert_type == STE_INSERT_TEXT_SURROUND));

    if (m_insert_type == STE_INSERT_TEXT_ATCOLUMN)
        m_prependText->SetLabel(_("Insert"));
    else
        m_prependText->SetLabel(_("Prepend"));
}

#define GET_RADIO_VALUE(id) wxStaticCast(FindWindow(id),wxRadioButton)->GetValue()

wxWindowID wxSTEditorInsertTextDialog::GetSelectedRadioId() const
{
    if (GET_RADIO_VALUE(ID_STEDLG_INSERT_PREPEND_RADIOBUTTON))
        return ID_STEDLG_INSERT_PREPEND_RADIOBUTTON;
    if (GET_RADIO_VALUE(ID_STEDLG_INSERT_APPEND_RADIOBUTTON))
        return ID_STEDLG_INSERT_APPEND_RADIOBUTTON;
    if (GET_RADIO_VALUE(ID_STEDLG_INSERT_ATCOLUMN_RADIOBUTTON))
        return ID_STEDLG_INSERT_ATCOLUMN_RADIOBUTTON;
    if (GET_RADIO_VALUE(ID_STEDLG_INSERT_SURROUND_RADIOBUTTON))
        return ID_STEDLG_INSERT_SURROUND_RADIOBUTTON;

    return wxID_ANY;
}

wxSTEditorInsertTextDialog::STE_InsertText_Type wxSTEditorInsertTextDialog::RadioIdToType( wxWindowID id ) const
{
    // the wxWindowIDs are in the same order as the enum
    if ((id >= ID_STEDLG_INSERT_PREPEND_RADIOBUTTON) && (id <= ID_STEDLG_INSERT_SURROUND_RADIOBUTTON))
        return (STE_InsertText_Type)(id - ID_STEDLG_INSERT_PREPEND_RADIOBUTTON);

    return STE_INSERT_TEXT_PREPEND;
}

//-----------------------------------------------------------------------------
// wxSTEditorColumnizeDialog
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxSTEditorColumnizeDialog, wxDialog);

wxArrayString wxSTEditorColumnizeDialog::sm_splitBeforeArray;
wxArrayString wxSTEditorColumnizeDialog::sm_splitAfterArray;
wxArrayString wxSTEditorColumnizeDialog::sm_preserveArray;
wxArrayString wxSTEditorColumnizeDialog::sm_ignoreArray;

BEGIN_EVENT_TABLE(wxSTEditorColumnizeDialog, wxDialog)
    EVT_BUTTON     (wxID_ANY, wxSTEditorColumnizeDialog::OnButton)
    EVT_TEXT       (wxID_ANY, wxSTEditorColumnizeDialog::OnText)
END_EVENT_TABLE()

void wxSTEditorColumnizeDialog::Init()
{
    if (sm_splitBeforeArray.GetCount() == 0) sm_splitBeforeArray.Add(wxT("){}"));
    if (sm_splitAfterArray.GetCount()  == 0) sm_splitAfterArray.Add(wxT("(,;"));
    if (sm_preserveArray.GetCount()    == 0) sm_preserveArray.Add(wxT("\"\""));
    //if (sm_ignoreArray.GetCount()      == 0) sm_ignoreArray.Add(wxT("\"\""));
    m_splitBeforeCombo = NULL;
    m_splitAfterCombo  = NULL;
    m_preserveCombo    = NULL;
    m_ignoreCombo      = NULL;
    m_testEditor       = NULL;
}

bool wxSTEditorColumnizeDialog::Create(wxWindow* parent, long style)
{
    if (!wxDialog::Create(parent, wxID_ANY, _("Columnize Text"), wxDefaultPosition, wxDefaultSize, style))
        return false;

    m_testEditor = new wxSTEditor(this, ID_STEDLG_COLUMNIZE_EDITOR,
                                        wxDefaultPosition, wxSize(400, 200));
    wxSTEditorColumnizeSizer(this, true, true);

    m_splitBeforeCombo = wxStaticCast(FindWindow(ID_STEDLG_COLUMNIZE_BEFORE_COMBO  ), wxComboBox);
    m_splitAfterCombo  = wxStaticCast(FindWindow(ID_STEDLG_COLUMNIZE_AFTER_COMBO   ), wxComboBox);
    m_preserveCombo    = wxStaticCast(FindWindow(ID_STEDLG_COLUMNIZE_PRESERVE_COMBO), wxComboBox);
    m_ignoreCombo      = wxStaticCast(FindWindow(ID_STEDLG_COLUMNIZE_IGNORE_COMBO  ), wxComboBox);
    m_updateCheckBox   = wxStaticCast(FindWindow(ID_STEDLG_COLUMNIZE_CHECKBOX      ), wxCheckBox);

    wxSTEInitComboBoxStrings(sm_splitBeforeArray, m_splitBeforeCombo);
    wxSTEInitComboBoxStrings(sm_splitAfterArray,  m_splitAfterCombo);
    wxSTEInitComboBoxStrings(sm_preserveArray,    m_preserveCombo);
    wxSTEInitComboBoxStrings(sm_ignoreArray,      m_ignoreCombo);

    return true;
}

void wxSTEditorColumnizeDialog::SetText(const wxString& text)
{
    m_initText = text;
    m_testEditor->SetReadOnly(false);
    m_testEditor->SetText(m_initText);
    m_testEditor->SetReadOnly(true);
}

wxString wxSTEditorColumnizeDialog::GetText()
{
    return m_testEditor->GetText();
}

void wxSTEditorColumnizeDialog::FormatText()
{
    wxString splitBefore = m_splitBeforeCombo->GetValue();
    wxString splitAfter  = m_splitAfterCombo->GetValue();
    wxString preserve    = m_preserveCombo->GetValue();
    wxString ignore      = m_ignoreCombo->GetValue();
    m_testEditor->SetReadOnly(false);
    m_testEditor->SetText(m_initText);
    m_testEditor->Columnize(0, -1, splitBefore, splitAfter, preserve, ignore);
    m_testEditor->SetReadOnly(true);
}

void wxSTEditorColumnizeDialog::OnButton(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case ID_STEDLG_COLUMNIZE_FORMAT_BUTTON :
        {
            FormatText();
            break;
        }
        case ID_STEDLG_COLUMNIZE_ORIGINAL_BUTTON :
        {
            m_testEditor->SetReadOnly(false);
            m_testEditor->SetText(m_initText);
            m_testEditor->SetReadOnly(true);
            break;
        }
        case wxID_OK :
        {
            wxSTEPrependArrayString(m_splitBeforeCombo->GetValue(), sm_splitBeforeArray, 10);
            wxSTEPrependArrayString(m_splitAfterCombo->GetValue(),  sm_splitAfterArray,  10);
            wxSTEPrependArrayString(m_preserveCombo->GetValue(),    sm_preserveArray,    10);
            wxSTEPrependArrayString(m_ignoreCombo->GetValue(),      sm_ignoreArray,      10);
            break;
        }
        default : break;
    }

    event.Skip();
}

void wxSTEditorColumnizeDialog::OnText(wxCommandEvent& event)
{
    event.Skip();

    if (m_updateCheckBox->GetValue())
        FormatText();
}

//-----------------------------------------------------------------------------
// wxSTEditorFileOpenPanel
//-----------------------------------------------------------------------------

#if STE_FILEOPENEXTRA
// panel with custom controls for file dialog
class wxSTEditorFileOpenPanel : public wxPanel
{
    DECLARE_CLASS(wxSTEditorFileOpenPanel)
public:
    wxSTEditorFileOpenPanel();

    bool Create(wxWindow *parent);

    int m_index;
    bool m_bom;

    wxChoice* GetList()
    {
        return wxStaticCast(FindWindow(ID_STEFILEOPEN_ENCODING_CHOICE), wxChoice);
    }
    wxCheckBox* GetCheckBox()
    {
        return wxStaticCast(FindWindow(ID_STEFILEOPEN_ENCODING_BOM_CHECKBOX), wxCheckBox);
    }

    static wxWindow* ControlCreator(wxWindow* parent)
    {
        wxSTEditorFileOpenPanel* wnd = new wxSTEditorFileOpenPanel();

        if (!wnd->Create(parent))
        {
            wxDELETE(wnd);
        }
        return wnd;
    }

    virtual ~wxSTEditorFileOpenPanel()
    {
    }
protected:
    void OnUpdateBomCheckBox(wxUpdateUIEvent&);
    DECLARE_EVENT_TABLE()
};

IMPLEMENT_CLASS(wxSTEditorFileOpenPanel, wxPanel)

BEGIN_EVENT_TABLE(wxSTEditorFileOpenPanel, wxPanel)
    EVT_UPDATE_UI(ID_STEFILEOPEN_ENCODING_BOM_CHECKBOX, wxSTEditorFileOpenPanel::OnUpdateBomCheckBox)
END_EVENT_TABLE()

wxSTEditorFileOpenPanel::wxSTEditorFileOpenPanel() : wxPanel(), m_index(wxTextEncoding::None + ENC_OFFSET)
{
}

// nevet gets called :-(
void wxSTEditorFileOpenPanel::OnUpdateBomCheckBox(wxUpdateUIEvent& event)
{
    bool bom = ::EnableBomCheckBox(GetList(), GetCheckBox());

    event.Enable(bom);
}

bool wxSTEditorFileOpenPanel::Create(wxWindow* parent)
{
    bool ok = wxPanel::Create(parent);

    if (ok)
    {
        wxSTEditorFileOpenSizer(this);

        if (parent->IsKindOf(CLASSINFO(wxSTEditorFileDialog)))
        {
            wxSTEditorFileDialog* dlg = (wxSTEditorFileDialog*)parent;

            m_index = wxTextEncoding::TypeFromString(wxSTEditorFileDialog::m_encoding);
            m_bom   = wxSTEditorFileDialog::m_file_bom;

            for (size_t i = 0; i < wxTextEncoding::TextEncoding__Count; i++)
            {
                GetList()->Append(wxTextEncoding::TypeToString((wxTextEncoding::Type)i));
            }

            GetList()->SetValidator(wxGenericValidator(&m_index));
            if (dlg->HasFdFlag(wxFD_SAVE))
            {
                GetCheckBox()->SetValidator(wxGenericValidator(&m_bom));
            }
            else
            {
                GetCheckBox()->Hide();
            }
            //TransferDataToWindow();
            //trac.wxwidgets.org/ticket/13611
        }
    }
    return ok;
}
#else // !STE_FILEOPENEXTRA
enum filterindex
{
    filterindex_allfiles = 0,
    filterindex_utf8,
    filterindex_unicode,
#ifdef __WXMSW__
    filterindex_oem,
#endif
};
#endif // STE_FILEOPENEXTRA

//-----------------------------------------------------------------------------
// wxSTEditorFileDialog
//-----------------------------------------------------------------------------

IMPLEMENT_CLASS(wxSTEditorFileDialog, wxFileDialog)

/*static*/ wxString wxSTEditorFileDialog::m_encoding;
/*static*/ bool     wxSTEditorFileDialog::m_file_bom = false;

wxSTEditorFileDialog::wxSTEditorFileDialog(wxWindow* parent,
                                           const wxString& message,
                                           const wxString& defaultDir,
                                           const wxString& extensions,
                                           long style)
                     :wxFileDialog(parent, message, defaultDir, wxEmptyString, extensions, style)
{
    //m_encoding = wxBOM_UTF8;
    //SetFilterIndex(extensions.Freq('|')/2); // "All Files (*)" is at the very bottom of the combobox
#if STE_FILEOPENEXTRA
    SetExtraControlCreator(&wxSTEditorFileOpenPanel::ControlCreator);
#else
    switch (wxTextEncoding::TypeFromString(wxSTEditorFileDialog::m_encoding))
    {
        case wxTextEncoding::UTF8:
            SetFilterIndex(filterindex_utf8);
            break;
        case wxTextEncoding::Unicode_LE:
            SetFilterIndex(filterindex_unicode);
            break;
    #ifdef __WXMSW__
        case wxTextEncoding::OEM:
            SetFilterIndex(filterindex_oem);
            break;
    #endif
        default:
            break;
    }
#endif
}

int wxSTEditorFileDialog::ShowModal()
{
    int n = wxFileDialog::ShowModal();

#if STE_FILEOPENEXTRA
    if (n == wxID_OK)
    {
        wxSTEditorFileOpenPanel* wnd = wxStaticCast(GetExtraControl(), wxSTEditorFileOpenPanel);

        //wnd->TransferDataFromWindow();
        //trac.wxwidgets.org/ticket/13611

        m_encoding = wxTextEncoding::TypeToString((wxTextEncoding::Type)wnd->m_index);
        m_file_bom = wnd->m_bom;
    }
#else // !STE_FILEOPENEXTRA
    if (n == wxID_OK)
    {
        switch (GetFilterIndex())
        {
            case filterindex_utf8:
                m_encoding = wxTextEncoding::TypeToString(wxTextEncoding::UTF8);
                break;
            case filterindex_unicode:
                m_encoding = wxTextEncoding::TypeToString(wxTextEncoding::Unicode_LE);
                break;
        #ifdef __WXMSW__
            case filterindex_oem:
                m_encoding = wxTextEncoding::TypeToString(wxTextEncoding::OEM);
                break;
        #endif
        }
    }
#endif // STE_FILEOPENEXTRA
    return n;
}

//-----------------------------------------------------------------------------
// wxSTEditorStdDialogButtonSizer
//-----------------------------------------------------------------------------

wxStdDialogButtonSizer* wxSTEditorStdDialogButtonSizer(wxWindow* parent, long flags)
{
    wxStdDialogButtonSizer* buttonpane = new wxStdDialogButtonSizer();

    if ((flags & wxOK) && (flags & wxCANCEL))
    {
        buttonpane->AddButton(new wxButton(parent, wxID_OK));
        buttonpane->AddButton(new wxButton(parent, wxID_CANCEL));
        buttonpane->GetAffirmativeButton()->SetDefault();
    }
    else if (flags & wxCANCEL)
    {
        buttonpane->AddButton(new wxButton(parent, wxID_CANCEL, _("Cl&ose")));
        buttonpane->GetCancelButton()->SetDefault();
    }
    if (flags & wxAPPLY)
    {
        buttonpane->AddButton(new wxButton(parent, wxID_APPLY, _("&Apply")));
    }
    buttonpane->Realize();

    //parent->GetSizer()->Add(new wxStaticLine(parent), 0, wxEXPAND | wxALL, 5); // separator
    parent->GetSizer()->Add(buttonpane, 0, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 5);

    return buttonpane;
}

//-----------------------------------------------------------------------------
// wxSTEditorAboutDialog
//-----------------------------------------------------------------------------

void wxSTEditorAboutDialog(wxWindow* parent)
{
    wxString msg, buildStr;

#ifdef wxUSE_UNICODE
    #if defined(wxUSE_UNICODE_UTF8) && wxUSE_UNICODE_UTF8 // wx 2.9+
        buildStr = wxT("UTF8");
    #else
        buildStr = wxT("Unicode");
    #endif
#else
    buildStr = wxT("Ansi");
#endif

#ifdef __WXDEBUG__
    if (!buildStr.IsEmpty()) buildStr += wxT(", ");
    buildStr += wxT("Debug"); // don't show release, that's assumed
#endif
    buildStr = wxT(" (") + buildStr + wxT(")");

    msg.Printf( wxT("Welcome to ") STE_VERSION_STRING wxT(".\n\n")
                wxT("Using ") wxVERSION_STRING wxT(", http://www.wxwidgets.org\n")
                wxT("and %s, http://www.scintilla.org\n")
                wxT("\n")
                wxT("Compiled on %s%s."),
            #if (wxVERSION_NUMBER >= 2902)
                wxStyledTextCtrl::GetLibraryVersionInfo().ToString().wx_str(),
            #else
                wxT("Scintilla 1.70"),
            #endif
                wxString::FromAscii(__DATE__).wx_str(), // no need to show time
                buildStr.wx_str()
                );

    // FIXME - or test wxFileConfig doesn't have ClassInfo is this safe?
    //if ((wxFileConfig*)wxConfigBase::Get(false))
    //    msg += wxT("\nConfig file: ")+((wxFileConfig*)wxConfigBase::Get(false))->m_strLocalFile;

   wxAboutDialogInfo info;
   info.SetName(STE_APPDISPLAYNAME);
   info.SetDescription(msg);
   info.SetWebSite(wxT(STE_WEBSITE));
   info.SetLicense(wxT("wxWindows Licence\nhttp://www.wxwidgets.org/about/licence3.txt"));
   info.AddDeveloper(wxT("John Labenski"));
   info.AddDeveloper(wxT("Troels K"));
   info.AddDeveloper(wxT("Otto Wyss"));
   info.SetIcon(wxArtProvider::GetIcon(wxART_STEDIT_APP, wxART_MESSAGE_BOX));
   ::wxAboutBox(info, parent);
}
