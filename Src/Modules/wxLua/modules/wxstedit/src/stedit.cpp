///////////////////////////////////////////////////////////////////////////////
// Name:        stedit.cpp
// Purpose:     wxSTEditor
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// RCS-ID:
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/*
Updated to SciTE 3.0.1, 12/3/2011

Code below marked with this copyright is under this license.
"Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>"

License for Scintilla and SciTE

Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>

All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.

NEIL HODGSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL NEIL HODGSON BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "precomp.h"

#include <wx/printdlg.h>        // wxPageSetupDialog
#include <wx/fileconf.h>        // wxFileConfig
#include <wx/clipbrd.h>         // wxClipboard
#include <wx/wfstream.h>        // wxFileInputStream
#include <wx/numdlg.h>
#include <wx/scrolbar.h>
#include <wx/choicdlg.h>
#include <wx/textdlg.h>
#include <wx/srchctrl.h>
#include <wx/sstream.h>
#include <wx/log.h>

#if (wxVERSION_NUMBER >= 2900)
    #include <wx/uiaction.h>
#endif

#include "wx/stedit/stedit.h"
#include "wx/stedit/steexprt.h"
#include "wx/stedit/steart.h"
#include "wx/stedit/stetree.h"

#include "wxtrunk.h"
#include "wxext.h"

//-----------------------------------------------------------------------------
// Global data

wxString calltipWordCharacters(wxT("_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
wxString autoCompleteStartCharacters(calltipWordCharacters);
wxString wordCharacters(wxT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#")); // FIXME add to langs

// These are strings to match wxStyledTextCtrl::GetEOLMode();
static const int wxSTC_EOL_Strings_count = 3;
static const wxString wxSTC_EOL_Strings[wxSTC_EOL_Strings_count] =
{
    wxT("CRLF (Dos/MS Windows)"),   // wxSTC_EOL_CRLF
    wxT("CR (Mac)"),                // wxSTC_EOL_CR
    wxT("LF (Unix)")                // wxSTC_EOL_LF
};

//-----------------------------------------------------------------------------
// wxSTEditorRefData - data that the styled text editor shares with refed ones
//-----------------------------------------------------------------------------

wxSTEditorRefData::wxSTEditorRefData()
                  :wxObjectRefData(),
                   m_file_bom(wxBOM_None),
                   m_steLang_id(STE_LANG_NULL),
                   m_treeItemData(NULL),
                   m_last_autoindent_line(-1),
                   m_last_autoindent_len(0),
                   m_state(0),
                   m_dirty_flag(false)
{
}

wxSTEditorRefData::~wxSTEditorRefData()
{
    if (m_treeItemData != NULL)
        m_treeItemData->m_steRefData = NULL;

    m_editors.Clear();
}

bool wxSTEditorRefData::SetLanguage(const wxFileName &filePath)
{
    int lang = STE_LANG_NULL;

    // use current langs or default if none
    if (m_steLangs.IsOk())
        lang = m_steLangs.FindLanguageByFilename(filePath);
    else
        lang = wxSTEditorLangs(true).FindLanguageByFilename(filePath);

    if (lang != STE_LANG_NULL)
        return SetLanguage(lang);

    return false;
}

//-----------------------------------------------------------------------------
// wxSTEditor
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSTEditor, wxStyledTextCtrl)

BEGIN_EVENT_TABLE(wxSTEditor, wxStyledTextCtrl)
    EVT_SET_FOCUS               (wxSTEditor::OnSetFocus)

    EVT_CONTEXT_MENU            (wxSTEditor::OnContextMenu)
    EVT_KEY_DOWN                (wxSTEditor::OnKeyDown)
    //EVT_KEY_UP                (wxSTEditor::OnKeyUp)
    EVT_MOUSEWHEEL              (wxSTEditor::OnMouseWheel)
    EVT_SCROLL                  (wxSTEditor::OnScroll)
    EVT_SCROLLWIN               (wxSTEditor::OnScrollWin)

    EVT_MENU                    (wxID_ANY, wxSTEditor::OnMenu)

    EVT_STC_CHARADDED           (wxID_ANY, wxSTEditor::OnSTCCharAdded)
    EVT_STC_UPDATEUI            (wxID_ANY, wxSTEditor::OnSTCUpdateUI)
    EVT_STC_MARGINCLICK         (wxID_ANY, wxSTEditor::OnSTCMarginClick)
    EVT_STEDITOR_MARGINDCLICK   (wxID_ANY, wxSTEditor::OnSTCMarginDClick)

    EVT_STEDITOR_STATE_CHANGED  (wxID_ANY, wxSTEditor::OnSTEState)
    EVT_STEDITOR_SET_FOCUS      (wxID_ANY, wxSTEditor::OnSTEFocus)

    EVT_FIND                    (wxID_ANY, wxSTEditor::OnFindDialog)
    EVT_FIND_NEXT               (wxID_ANY, wxSTEditor::OnFindDialog)
    EVT_FIND_REPLACE            (wxID_ANY, wxSTEditor::OnFindDialog)
    EVT_FIND_REPLACE_ALL        (wxID_ANY, wxSTEditor::OnFindDialog)
    EVT_FIND_CLOSE              (wxID_ANY, wxSTEditor::OnFindDialog)
    EVT_STEFIND_GOTO            (wxID_ANY, wxSTEditor::OnFindDialog)
END_EVENT_TABLE()

// Put these macros at the top of a function you want to block executing
// when either the editor is starting up or shutting down.
#define STE_INITRETURN       if (!m_sendEvents || IsBeingDeleted()) return
#define STE_INITRETURNVAL(a) if (!m_sendEvents || IsBeingDeleted()) return (a)

void wxSTEditor::Init()
{
    // This is the same as CLASSINFO(wxSTEditorRefDataObject)->CreateObject()
    // unless the user initializes STE_GlobalRefDataClassInfo to something else
    m_refData = dynamic_cast<wxSTEditorRefData*>(STE_GlobalRefDataClassInfo->CreateObject());

    m_sendEvents = false;
    m_activating = false;

    m_marginDClickTime   =  0;
    m_marginDClickLine   = -1;
    m_marginDClickMargin = -1;
}

wxSTEditor::wxSTEditor( wxWindow *parent, wxWindowID id,
                        const wxPoint& pos, const wxSize& size,
                        long style, const wxString &name ) : wxStyledTextCtrl()
{
    Init();
    Create(parent, id, pos, size, style, name);
}

wxSTEditor::~wxSTEditor()
{
    // For safety, duplicates Destroy() function.

    // Kill all these so that an extraneous focus event won't make us segfault
    // Yes we're in the destructor... strange things happen.
    m_sendEvents = false;

    // remove us from everything so nothing can "get at us"
    GetSTERefData()->RemoveEditor(this);

    // don't destroy prefs since we may be a refed editor, remove us though
    if (GetEditorPrefs().IsOk())  GetEditorPrefs().RemoveEditor(this);
    if (GetEditorStyles().IsOk()) GetEditorStyles().RemoveEditor(this);
    if (GetEditorLangs().IsOk())  GetEditorLangs().RemoveEditor(this);

    // if we're a refed editor, release the document.
    if (GetRefData()->GetRefCount() > 1)
        ReleaseDocument(GetDocPointer());
}

bool wxSTEditor::Destroy()
{
    // For safety, duplicates destructor.

    // Kill all these so that an extraneous focus event won't make us segfault
    m_sendEvents = false;

    // remove us from everything so nothing can "get at us"
    GetSTERefData()->RemoveEditor(this);

    // don't destroy prefs since we may be a refed editor, remove us though
    if (GetEditorPrefs().IsOk())  GetEditorPrefs().RemoveEditor(this);
    if (GetEditorStyles().IsOk()) GetEditorStyles().RemoveEditor(this);
    if (GetEditorLangs().IsOk())  GetEditorLangs().RemoveEditor(this);

    return wxStyledTextCtrl::Destroy();
}

bool wxSTEditor::Create( wxWindow *parent, wxWindowID id,
                         const wxPoint& pos, const wxSize& size,
                         long style, const wxString& name )
{
    if (!wxStyledTextCtrl::Create(parent, id, pos, size, style, name))
        return false;

    if ((size.x > 0) && (size.y > 0))
        SetInitialSize(size);

    SetStateSingle(STE_CANPASTE, CanPaste());

    //SetMarginWidth(0, 0); // for some reason margin 1 is not set to 0
    //SetMarginWidth(1, 0);
    //SetMarginWidth(2, 0);

    SetProperty(wxT("fold"), wxT("1")); // might as well turn them on even if unused

    GetSTERefData()->AddEditor(this);

    // turn on sending events after variables are fully initialized.
    m_sendEvents = true;

    return true;
}

wxSTEditor* wxSTEditor::Clone(wxWindow *parent, wxWindowID id,
                              const wxPoint& pos, const wxSize& size,
                              long style, const wxString& name) const
{
    wxSTEditor *editor = wxStaticCast(GetClassInfo()->CreateObject(), wxSTEditor);
    editor->Create(parent, id, pos, size, style, name);
    return editor;
}

void wxSTEditor::RefEditor(wxSTEditor *origEditor)
{
    wxCHECK_RET(origEditor && (origEditor != this) &&
                (origEditor->GetRefData() != GetRefData()), wxT("Invalid editor to ref"));

    // remove us from the prefs, if any
    if (GetEditorPrefs().IsOk())  GetEditorPrefs().RemoveEditor(this);
    if (GetEditorStyles().IsOk()) GetEditorStyles().RemoveEditor(this);
    if (GetEditorLangs().IsOk())  GetEditorLangs().RemoveEditor(this);

    // remove us from current refData
    GetSTERefData()->RemoveEditor(this);

    // Ref our new data
    Ref(*origEditor);
    // Ref scintilla's editor
    AddRefDocument(origEditor->GetDocPointer());
    SetDocPointer(origEditor->GetDocPointer());

    GetSTERefData()->AddEditor(this);

    // register us into the prefs, if any
    if (GetEditorStyles().IsOk()) GetEditorStyles().RegisterEditor(this);
    if (GetEditorPrefs().IsOk())  GetEditorPrefs().RegisterEditor(this);
    if (GetEditorLangs().IsOk())  GetEditorLangs().RegisterEditor(this);
}

//-----------------------------------------------------------------------------

void wxSTEditor::CreateOptions(const wxSTEditorOptions& options)
{
    GetSTERefData()->m_options = options;

    // Ok if they're invalid since that's how we clear them
    RegisterStyles(GetOptions().GetEditorStyles());
    RegisterPrefs(GetOptions().GetEditorPrefs());
    RegisterLangs(GetOptions().GetEditorLangs());

    wxSTEditorMenuManager *steMM = GetOptions().GetMenuManager();

    // create the editor popup menu
    if (steMM && GetOptions().HasEditorOption(STE_CREATE_POPUPMENU) &&
        !GetOptions().GetEditorPopupMenu())
    {
        GetOptions().SetEditorPopupMenu(steMM->CreateEditorPopupMenu(), false);
    }

    // create the accelerator table
    if (steMM && GetOptions().HasEditorOption(STE_CREATE_ACCELTABLE) &&
        (GetOptions().GetEditorPopupMenu() || GetOptions().GetMenuBar()))
    {
        wxAcceleratorTable table(steMM->CreateAcceleratorTable(GetOptions().GetEditorPopupMenu(),
                                                               GetOptions().GetMenuBar()));
        SetAcceleratorTable(table);
    }

    // Send created event to parent, not to this.
    // A derived editor would simply have overridden this function.
    wxCommandEvent event(wxEVT_STEDITOR_CREATED, GetId());
    event.SetEventObject(this);
    GetParent()->GetEventHandler()->ProcessEvent(event);
}

const wxSTEditorOptions& wxSTEditor::GetOptions() const
{
    return GetSTERefData()->m_options;
}
wxSTEditorOptions& wxSTEditor::GetOptions()
{
    return GetSTERefData()->m_options;
}
void wxSTEditor::SetOptions(const wxSTEditorOptions& options)
{
    GetSTERefData()->m_options = options;
}

//-----------------------------------------------------------------------------

#if (wxVERSION_NUMBER < 2900)
bool wxSTEditor::PositionToXY(STE_TextPos pos, long *col, long *row) const
{
    if ((pos < 0) || (pos > GetLength()))
    {
        if (col) *col = 0;
        if (row) *row = 0;
        return false;
    }

    int r = LineFromPosition(pos);
    if (row) *row = r;
    if (col) *col = pos - PositionFromLine(r);
    return true;
}
#endif

void wxSTEditor::SetEditable(bool editable)
{
    if (IsEditable() == editable)
        return;

#if (wxVERSION_NUMBER >= 2900)
    wxStyledTextCtrl::SetEditable(editable);
#else
    wxStyledTextCtrl::SetReadOnly(!editable); // SetEditable() doesn't exist in wx28
#endif

    SendEvent(wxEVT_STEDITOR_STATE_CHANGED, STE_EDITABLE, GetState(), GetFileName().GetFullPath());
}

bool wxSTEditor::IsModified() const
{
#if (wxVERSION_NUMBER >= 2900)
    return GetSTERefData()->m_dirty_flag || wxStyledTextCtrl::IsModified();
#else
    return GetSTERefData()->m_dirty_flag || wxConstCast(this, wxSTEditor)->GetModify();
#endif
}

void wxSTEditor::DiscardEdits()
{
#if (wxVERSION_NUMBER >= 2900)
    wxStyledTextCtrl::DiscardEdits();
#else
    SetSavePoint();
#endif
    GetSTERefData()->m_dirty_flag = false;

    SendEvent(wxEVT_STEDITOR_STATE_CHANGED, STE_MODIFIED, GetState(), GetFileName().GetFullPath());
}

void wxSTEditor::MarkDirty()
{
    //wxStyledTextCtrl::MarkDirty(); // not implemented, asserts
    GetSTERefData()->m_dirty_flag = true;
    SendEvent(wxEVT_STEDITOR_STATE_CHANGED, STE_MODIFIED, GetState(), GetFileName().GetFullPath());
}

bool wxSTEditor::GetViewNonPrint() const
{
    return wxConstCast(this, wxSTEditor)->GetViewEOL() &&
            (wxSTC_WS_INVISIBLE != wxConstCast(this, wxSTEditor)->GetViewWhiteSpace());
}

void wxSTEditor::SetViewNonPrint(bool show_non_print)
{
   GetEditorPrefs().SetPrefBoolByID(ID_STE_PREF_VIEW_EOL, show_non_print);
   GetEditorPrefs().SetPrefBoolByID(ID_STE_PREF_VIEW_WHITESPACE, show_non_print);
}

// --------------------------------------------------------------------------

bool wxSTEditor::TranslatePos(STE_TextPos start_pos,        STE_TextPos end_pos,
                              STE_TextPos* trans_start_pos, STE_TextPos* trans_end_pos,
                              STE_TranslatePosType type)
{
    int length = GetLength();

    if ( ((start_pos == 0) || (start_pos == -1)) && (end_pos == -1))  // do whole document
    {
        end_pos = length;
    }
    else                                            // do selection
    {
        STE_TextPos sel_start = (type == STE_TRANSLATE_SELECTION) ? GetSelectionStart() :
                                (type == STE_TRANSLATE_SELECTION) ? GetTargetStart()    : start_pos;
        STE_TextPos sel_end   = (type == STE_TRANSLATE_SELECTION) ? GetSelectionEnd()   :
                                (type == STE_TRANSLATE_SELECTION) ? GetTargetEnd()      : end_pos;

        if (start_pos < 0) start_pos = sel_start;
        if (end_pos   < 0) end_pos   = sel_end;
    }

    if (start_pos == end_pos)                       // do current line
    {
        STE_TextPos pos = GetCurrentPos();

        int line  = LineFromPosition(pos);
        start_pos = PositionFromLine(line);
        end_pos   = GetLineEndPosition(line);
    }

    // ensure valid
    start_pos = wxMin(start_pos, length);
    end_pos   = wxMin(end_pos,   length);
    start_pos = wxMax(start_pos, 0);
    end_pos   = wxMax(end_pos,   0);

    // reorder to go from min to max
    if (trans_start_pos) *trans_start_pos = wxMin(start_pos, end_pos);
    if (trans_end_pos  ) *trans_end_pos   = wxMax(start_pos, end_pos);

    return start_pos < end_pos;
}

bool wxSTEditor::TranslateLines(int  top_line,       int  bottom_line,
                                int* trans_top_line, int* trans_bottom_line,
                                STE_TranslatePosType type)
{
    int line_count = GetLineCount() - 1;
    line_count = wxMax(0, line_count);

    if ((top_line == 0) && (bottom_line == -1))     // do whole document
    {
        bottom_line = line_count;
    }
    else                                            // do selection
    {
        STE_TextPos sel_start = (type == STE_TRANSLATE_SELECTION) ? GetSelectionStart() :
                                (type == STE_TRANSLATE_SELECTION) ? GetTargetStart()    : GetCurrentPos();
        STE_TextPos sel_end   = (type == STE_TRANSLATE_SELECTION) ? GetSelectionEnd()   :
                                (type == STE_TRANSLATE_SELECTION) ? GetTargetEnd()      : GetCurrentPos();

        if (top_line < 0)
            top_line = LineFromPosition(sel_start);
        if (bottom_line < 0)
            bottom_line = LineFromPosition(sel_end);
    }

    // ensure valid
    top_line    = wxMin(top_line,    line_count);
    bottom_line = wxMin(bottom_line, line_count);
    top_line    = wxMax(top_line,    0);
    bottom_line = wxMax(bottom_line, 0);

    // reorder to go from min to max
    if (trans_top_line   ) *trans_top_line    = wxMin(top_line, bottom_line);
    if (trans_bottom_line) *trans_bottom_line = wxMax(top_line, bottom_line);

    return top_line < bottom_line;
}

bool wxSTEditor::TextRangeIsWord(STE_TextPos start_pos, STE_TextPos end_pos) const
{
    STE_TextPos len = GetLength();

    if ((start_pos >= end_pos) || (start_pos < 0) || (end_pos > len))
        return false;

    wxString text( GetTextRange(wxMax(0, start_pos-1), wxMin(end_pos+1, len)) );
    if (text.IsEmpty()) return false;

    if ((start_pos == 0) || (wordCharacters.Find(text[0]) == wxNOT_FOUND))
    {
        if ((end_pos == len) || (wordCharacters.Find(text[text.size()-1]) == wxNOT_FOUND))
            return true;
    }

    return false;
}

wxString wxSTEditor::GetTargetText() const
{
    STE_TextPos target_start = GetTargetStart();
    STE_TextPos target_end   = GetTargetEnd();

    if (target_start == target_end) return wxEmptyString;
    return GetTextRange(wxMin(target_start, target_end), wxMax(target_start, target_end));
}

static wxClipboardHelper::Clipboard_Type ClipboardTypeConv(STE_ClipboardType clip_type)
{
    switch (clip_type)
    {
        case STE_CLIPBOARD_PRIMARY : return wxClipboardHelper::CLIPBOARD_PRIMARY;
        case STE_CLIPBOARD_BOTH    : return wxClipboardHelper::CLIPBOARD_BOTH;
        case STE_CLIPBOARD_DEFAULT :
        default                    : return wxClipboardHelper::CLIPBOARD_DEFAULT;
    }
}

/*static*/ bool wxSTEditor::IsClipboardTextAvailable(STE_ClipboardType clip_type)
{
    return wxClipboardHelper::IsTextAvailable(ClipboardTypeConv(clip_type));
}

/*static*/ bool wxSTEditor::IsClipboardFormatAvailable(const enum wxDataFormatId* array,
                                                       size_t array_count,
                                                       STE_ClipboardType clip_type)
{
    return wxClipboardHelper::IsFormatAvailable(array, array_count, ClipboardTypeConv(clip_type));
}

/*static*/ bool wxSTEditor::GetClipboardText(wxString* str, STE_ClipboardType clip_type)
{
    wxCHECK_MSG(str != NULL, false, wxT("NULL input string to get clipboard data into."));
    return wxClipboardHelper::GetText(str, ClipboardTypeConv(clip_type));
}

/*static*/ bool wxSTEditor::SetClipboardText(const wxString& str, STE_ClipboardType clip_type)
{
    return wxClipboardHelper::SetText(str, ClipboardTypeConv(clip_type));
}

/*static*/ bool wxSTEditor::SetClipboardHtml(const wxString& htmldata)
{
    return wxClipboardHelper::SetHtmlText(htmldata);
}

bool wxSTEditor::PasteRectangular()
{
    wxString text;
    bool ok = GetClipboardText(&text, STE_CLIPBOARD_DEFAULT);

    if (ok)
    {
        text = ConvertEOLMode(text, GetEOLMode()); // FIXME: is this really necessary?
        PasteRectangular(text, -1);
    }

    return ok;
}

void wxSTEditor::PasteRectangular(const wxString& str, STE_TextPos pos)
{
    BeginUndoAction();
    //ClearSelection(); // we don't paste into selection for this

    pos = (pos > -1) ? pos : GetCurrentPos();

    int line                   = LineFromPosition(pos);
    STE_TextPos line_start_pos = PositionFromLine(line);
    STE_TextPos line_end_pos   = GetLineEndPosition(line);
    STE_TextPos line_pos       = pos - line_start_pos;

    wxString eolStr(GetEOLString());

    wxStringTokenizer tkz(str, wxT("\r\n"), wxTOKEN_STRTOK);
    for ( ; tkz.HasMoreTokens(); line++)
    {
        if (line >= GetLineCount())
            AppendText(eolStr);

        line_start_pos = PositionFromLine(line);
        line_end_pos   = GetLineEndPosition(line);

        wxString token(tkz.GetNextToken());
        if (line_end_pos < line_start_pos + line_pos)
            InsertText(line_end_pos, wxString(wxT(' '), line_start_pos + line_pos - line_end_pos));

        InsertText(line_start_pos + line_pos, token);
    }

    EndUndoAction();
    NotifyChange();
}

// function defined in ScintillaWX.cpp as wxConvertEOLMode()
// Note: they #ifdef it for wxUSE_DATAOBJ, but that's not a requirement
/*static*/ wxTextFileType wxSTEditor::ConvertEOLModeType(int stc_eol_mode)
{
    wxTextFileType type;

    switch (stc_eol_mode)
    {
        case wxSTC_EOL_CRLF : type = wxTextFileType_Dos; break;
        case wxSTC_EOL_CR   : type = wxTextFileType_Mac; break;
        case wxSTC_EOL_LF   : type = wxTextFileType_Unix; break;
        default             : type = wxTextBuffer::typeDefault; break;
    }
    return type;
}

/*static*/ wxString wxSTEditor::ConvertEOLMode(const wxString& str, int stc_eol_mode)
{
    return wxTextBuffer::Translate(str, ConvertEOLModeType(stc_eol_mode));
}

wxString wxSTEditor::GetEOLString(int stc_eol_mode) const
{
    if (stc_eol_mode < 0) stc_eol_mode = GetEOLMode();

    wxTextFileType type = ConvertEOLModeType(stc_eol_mode);

    if (type != wxTextFileType_None)
    {
        return wxTextBuffer::GetEOL(type);
    }
    wxFAIL_MSG(wxT("Invalid EOL mode"));
    return wxT("\n");
}

void wxSTEditor::AppendTextGotoEnd(const wxString &text, bool goto_end)
{
    // stay at end if already there
    if (!goto_end)
        goto_end = (GetCurrentLine() == GetLineCount());

    AppendText(text);
    if (goto_end)
        GotoPos(GetLength());
}

int wxSTEditor::GetLineLength(int line) const
{
    return (int)GetLineText(line).Length();
}

// GetLineText() is not implemented in wx28;
// it is implemented in wx trunk, but is wrong, see trac.wxwidgets.org/ticket/13646
wxString wxSTEditor::GetLineText(int line) const
{
    wxString lineText(GetLine(line));
    size_t len = lineText.Length();

    if (len > 0)
    {
        if (lineText[len-1] == wxT('\n'))
        {
            if ((len > 1) && (lineText[len-2] == wxT('\r'))) // remove \r\n for DOS
                return lineText.Mid(0, len-2);
            else
                return lineText.Mid(0, len-1);               // remove \n for Unix
        }
        else if (lineText[len-1] == wxT('\r'))               // remove \r for mac
            return lineText.Mid(0, len-1);
    }

    return lineText; // shouldn't happen, but maybe?
}

void wxSTEditor::SetLineText(int line, const wxString& text, bool inc_newline)
{
    wxString prepend;
    int line_count = GetLineCount();

    // add lines if necessary
    if (line >= line_count)
    {
        wxString eolStr(GetEOLString());
        size_t n, count = line - line_count;
        for (n = 0; n <= count; n++)
            prepend += eolStr;

        AppendText(prepend);
    }

    STE_TextPos pos = PositionFromLine(line);
    int line_len = inc_newline ? (int)GetLine(line).Length() : GetLineEndPosition(line) - pos;

    //wxPrintf(wxT("SetLineText l %d lc %d added %d len %d end %d '%s'\n"), line, line_count, line-line_count, GetLine(line).Length() , GetLineEndPosition(line)-pos, text.wx_str()); fflush(stdout);

    STE_TextPos target_start = GetTargetStart();
    STE_TextPos target_end   = GetTargetEnd();

    SetTargetStart(pos);
    SetTargetEnd(pos + line_len);
    ReplaceTarget(text);

    int diff = (int)prepend.Length() + (int)text.Length() - line_len;
    SetTargetStart(target_start < pos            ? target_start : target_start + diff);
    SetTargetEnd(  target_end   < pos + line_len ? target_end   : target_end   + diff);
}

size_t wxSTEditor::GetWordCount(const wxString& text) const
{
    size_t count = 0;
    bool new_word = false;

    // a word is a series of wxIsalnum
    for (wxString::const_iterator it = text.begin(); it != text.end(); it++)
    {
        if (wxIsalnum(*it))
        {
            if (!new_word)
            {
                new_word = true;
                count++;
            }
        }
        else
            new_word = false;
    }

    return count;
}

size_t wxSTEditor::GetWordCount(STE_TextPos start_pos, STE_TextPos end_pos, STE_TranslatePosType type)
{
    wxString text;
    if (TranslatePos(start_pos, end_pos, &start_pos, &end_pos, type))
        text = GetTextRange(start_pos, end_pos);

    return GetWordCount(text);
}

size_t wxSTEditor::GetWordArrayCount(const wxString& text,
                                     const wxArrayString& words,
                                     wxArrayInt& count,
                                     bool ignoreCase)
{
    size_t total_count = 0;

    count.Clear();
    const size_t word_count = words.GetCount();
    if (word_count == 0)
        return total_count;

    count.Add(0, word_count);
    const wxChar *c = text.GetData();
    size_t n, i, len = text.Length();

    for (n = 0; n < len; ++n, ++c)
    {
        for (i = 0; i < word_count; i++)
        {
            size_t word_len = words[i].Length();
            if (word_len && (len - n >= word_len))
            {
                const wxChar *word_c = words[i].GetData();
                if ((ignoreCase && (wxStrnicmp(c, word_c, word_len) == 0)) ||
                    (wxStrncmp(c, word_c, word_len) == 0))
                {
                    count[i]++;
                    total_count++;
                }
            }
        }
    }
    return total_count;
}

void wxSTEditor::GetEOLCount(int *crlf_, int *cr_, int *lf_, int *tabs_)
{
    int crlf = 0, cr = 0, lf = 0, tabs = 0;
    const wxString text(GetText());
    const wxChar *c = text.GetData();
    size_t n, len = text.Length();

    for (n = 0; n < len; ++n, ++c)
    {
        if ((*c) == wxT('\r'))
        {
            if ((n < len-1) && (c[1] == wxT('\n')))
            {
                ++crlf;
                ++n;
                ++c;
            }
            else
                ++cr;
        }
        else if ((*c) == wxT('\n'))
            ++lf;
        else if ((*c) == wxT('\t'))
            ++tabs;
    }

    if (crlf_) *crlf_ = crlf;
    if (cr_)   *cr_   = cr;
    if (lf_)   *lf_   = lf;
    if (tabs_) *tabs_ = tabs;
}

void wxSTEditor::SetIndentation(int width, int top_line, int bottom_line,
                                STE_TranslatePosType type)
{
    TranslateLines(top_line, bottom_line, &top_line, &bottom_line, type);

    BeginUndoAction();
    for (int n = top_line; n <= bottom_line; n++)
    {
        int indent = GetLineIndentation(n) + width;
        SetLineIndentation(n, wxMax( indent, 0));
    }
    EndUndoAction();
}

size_t wxSTEditor::ConvertTabsToSpaces(bool to_spaces,
                                       STE_TextPos start_pos, STE_TextPos end_pos,
                                       STE_TranslatePosType type)
{
    if (!TranslatePos(start_pos, end_pos, &start_pos, &end_pos, type))
        return 0;

    STE_TextPos pos = GetCurrentPos();

    STE_TextPos orig_sel_start = GetSelectionStart();
    STE_TextPos orig_sel_end   = GetSelectionEnd();

    SetTargetStart(start_pos);
    SetTargetEnd(end_pos);
    wxString spaceString;
    if (GetTabWidth() > 0) spaceString = wxString(wxT(' '), GetTabWidth());
    wxString findString(   !to_spaces ? spaceString : wxString(wxT("\t")));
    wxString replaceString( to_spaces ? spaceString : wxString(wxT("\t")));
    int diff = (int)replaceString.Length() - (int)findString.Length();

    SetSearchFlags(0);
    size_t count = 0;

    BeginUndoAction();
    for (STE_TextPos find_pos = SearchInTarget(findString);
         find_pos >= 0;
         find_pos = SearchInTarget(findString))
    {
        count++;
        ReplaceTarget(replaceString);
        SetTargetStart(find_pos);
        end_pos += diff;
        SetTargetEnd(end_pos);
    }
    EndUndoAction();

    int len = GetTextLength();
    // return cursor to last position
    GotoPos(wxMin(pos, len));
    // reselect what they had
    if (orig_sel_start != orig_sel_end)
    {
        orig_sel_end += int(count)*diff;
        SetSelection(orig_sel_start, orig_sel_end);
    }

    return count;
}

bool wxSTEditor::RemoveTrailingWhitespace(int top_line, int bottom_line)
{
    TranslateLines(top_line, bottom_line, &top_line, &bottom_line);

    bool done = false;
    BeginUndoAction();
    for (int n = top_line; n <= bottom_line; n++)
    {
        const STE_TextPos line_start = PositionFromLine(n);
        const STE_TextPos line_end = GetLineEndPosition(n);
        STE_TextPos pos;

        for (pos = line_end; pos > line_start; pos--)
        {
            const char chr = (char)GetCharAt(pos-1);
            if ((chr != ' ') && (chr != '\t')) break;
        }
        if (pos < line_end)
        {
            done = true;
            SetTargetStart(pos);
            SetTargetEnd(line_end);
            ReplaceTarget(wxEmptyString);
        }
    }
    EndUndoAction();
    return done;
}

bool wxSTEditor::RemoveCharsAroundPos(STE_TextPos pos, const wxString& remove)
{
    if (pos < 0)
        pos = GetCurrentPos();

    if (pos > GetLength()) // yep not >=
        return false;

    int n, line = LineFromPosition(pos);
    int line_start = LineFromPosition(line);
    STE_TextPos line_end   = GetLineEndPosition(line);

    STE_TextPos space_start = pos;
    STE_TextPos space_end   = pos;

    for (n = pos; n > line_start; n--)
    {
        wxChar chr = (wxChar)GetCharAt(n-1);
        if (remove.Find(chr) != wxNOT_FOUND)
            space_start = n-1;
        else
            break;
    }
    for (n = pos; n < line_end; n++)
    {
        wxChar chr = (wxChar)GetCharAt(n);
        if (remove.Find(chr) != wxNOT_FOUND)
            space_end = n+1;
        else
            break;
    }

    if (space_start != space_end)
    {
        SetTargetStart(space_start);
        SetTargetEnd(space_end);
        ReplaceTarget(wxEmptyString);
        return true;
    }
    return false;
}

bool wxSTEditor::InsertTextAtCol(int col, const wxString& text,
                                 int top_line, int bottom_line)
{
    if (text.IsEmpty())
        return false;

    STE_TextPos sel_start = GetSelectionStart();
    STE_TextPos sel_end   = GetSelectionEnd();

    TranslateLines(top_line, bottom_line, &top_line, &bottom_line);

    bool done = false;
    BeginUndoAction();
    for (int n = top_line; n <= bottom_line; n++)
    {
        const STE_TextPos line_start = PositionFromLine(n);
        const STE_TextPos line_end   = GetLineEndPosition(n);
        STE_TextPos pos = (col >= 0) ? (line_start + col) : line_end;
        wxString s(text);

        // if inserting before end of line then pad it
        if (pos > line_end)
        {
            s = wxString(wxT(' '), size_t(pos - line_end)) + text;
            pos = line_end;
        }

        // make sure that the selection stays in same place
        if (pos <= sel_start)
        {
            sel_start += (int)s.Length();
            sel_end   += (int)s.Length();
        }
        else if ((pos >= sel_start) && (pos < sel_end))
            sel_end += (int)s.Length();

        InsertText(pos, s);
    }
    EndUndoAction();

    SetSelection(sel_start, sel_end);
    return done;
}

// Find a set of chars like wxString::Find(), but from a starting position
// without having to make a copy of the input string.
static int wxString_FindFromPos(const wxString& str, const wxString& chars, size_t start_pos)
{
    wxChar chPrev = 0;
    size_t n = start_pos;
    wxString::const_iterator it = str.begin() + start_pos;

    for ( ; it != str.end(); it++, n++)
    {
        wxChar ch = *it;
        int idx = chars.Find(ch);

        // char in str is in chars and is not a " preceeded by a \, eg. \"
        if ((idx != wxNOT_FOUND) &&
            ( (n == 0) || (ch != wxT('\"')) || (chPrev != wxT('\\')) ) )
        {
            return (int)n;
        }
        chPrev = ch;
    }
    return wxNOT_FOUND;
}

bool wxSTEditor::Columnize(int top_line, int bottom_line,
                           const wxString& splitBefore_,
                           const wxString& splitAfter_,
                           const wxString& preserveChars,
                           const wxString& ignoreAfterChars_)
{
    TranslateLines(top_line, bottom_line, &top_line, &bottom_line);

    // only one or no lines
    if (top_line > bottom_line - 1)
        return false;

    // fix up the splitBefore/After by removing any extra whitespace
    wxString splitBefore(splitBefore_);
    splitBefore.Replace(wxT(" "),  wxEmptyString, true);
    splitBefore.Replace(wxT("\t"), wxEmptyString, true);
    splitBefore += wxT(" \t");

    wxString splitAfter(splitAfter_);
    splitAfter.Replace(wxT(" "),  wxEmptyString, true);
    splitAfter.Replace(wxT("\t"), wxEmptyString, true);

    wxString ignoreAfterChars(ignoreAfterChars_);
    ignoreAfterChars.Replace(wxT(" "),  wxEmptyString, true);
    ignoreAfterChars.Replace(wxT("\t"), wxEmptyString, true);

    // parse preserveChars '"" () []' and fill preserveStart with the first
    //   char to start preserving and fill preserveArray with the chars to
    //   end with, eg. preserveStart = '"([', preserveArray = " ) ]
    //   this allows for multiple endings to a single start char
    wxArrayString preserveEndArray;
    wxString preserveStart;

    for (wxStringTokenizer tkz(preserveChars, wxT(" "), wxTOKEN_STRTOK);
         tkz.HasMoreTokens();
         )
    {
        wxString token(tkz.GetNextToken());
        preserveStart += token[0];
        preserveEndArray.Add((token.Length() < 2) ? token : token.Mid(1));
    }

    int line, max_start_pos = 0;
    wxArrayInt maxLenArray;
    wxArrayInt *matchStartArray = new wxArrayInt[bottom_line - top_line + 1];
    wxArrayInt *matchLenArray   = new wxArrayInt[bottom_line - top_line + 1];

    for (line = top_line; line <= bottom_line; line++)
    {
        wxString lineText(GetLine(line).Strip(wxString::trailing));
        int col = 0, len = (int)lineText.Length();
        const wxChar *c = lineText.GetData();
        bool ignore = false;

        for (int n = 0; n < len;  )
        {
            // skip whitespace always
            while ((n < len) && ((*c == wxT(' ')) || (*c == wxT('\t')))) { c++; n++; }
            if (n == len) break;
            int match_start = n;

            bool split_before = false;
            bool split_after  = false;

            // iterate through "words" until separator is found
            while (n < len)
            {
                if (!ignore)
                {
                    // if last was split_before, c is not incremented and n == match_start
                    split_before = (n > match_start) && (splitBefore.Find(wxChar(*c)) != wxNOT_FOUND);
                    if (split_before)
                        break;

                    ignore = (ignoreAfterChars.Find(wxChar(*c)) != wxNOT_FOUND);

                    split_after = splitAfter.Find(wxChar(*c)) != wxNOT_FOUND;
                    if (split_after)
                    {
                        c++; n++;
                        break;
                    }
                }

                // second time through, just exit
                if (ignore)
                {
                    c += len - n;
                    n = len;
                    break;
                }

                // leave text between preserve chars alone
                int pre_start = preserveStart.Find(wxChar(*c));
                if (!ignore && (pre_start != wxNOT_FOUND))
                {
                    int ppos = wxString_FindFromPos(lineText, preserveEndArray[pre_start], n+1);
                    if (ppos != wxNOT_FOUND)
                    {
                        c += (ppos - n) + 1;
                        n = ppos + 1;
                        break;
                    }
                }

                c++; n++;
            }

            int match_len = n - match_start;

            // don't drop any chars - case for whitespace then split_before
            if (match_len == 0)
            {
                if (n == len) break; // all done - perhaps whitespace?

                match_len++;
                c++; n++;
            }

            matchStartArray[line-top_line].Add(match_start);
            matchLenArray[line-top_line].Add(match_len);

            //wxPrintf(wxT("line %d col %d start %d len %d\n"), line, col, match_start, match_len);

            // save the max starting position
            if ((col == 0) && (max_start_pos < match_start))
                max_start_pos = match_start;

            // save the max lengths
            if (col >= int(maxLenArray.GetCount()))
                maxLenArray.Add(match_len);
            else if (maxLenArray[col] < match_len)
                maxLenArray[col] = match_len;

            col++;
        }
    }

    for (line = top_line; line <= bottom_line; line++)
    {
        wxString lineText(GetLine(line));
        wxString newLine;
        if (max_start_pos > 0)
            newLine = wxString(wxT(' '), max_start_pos);

        int col, num_cols = (int)matchStartArray[line-top_line].GetCount();
        for (col = 0; col < num_cols; col++)
        {
            int match_start = matchStartArray[line-top_line][col];
            int match_len   = matchLenArray[line-top_line][col];
            newLine += lineText.Mid(match_start, match_len);
            if ((col < num_cols - 1) && (match_len < 1+maxLenArray[col]))
                newLine += wxString(wxT(' '), 1+maxLenArray[col]-match_len);

            //wxPrintf(wxT("'%s' '%s'\n"), lineText.wx_str(), newLine.wx_str()); fflush(stdout);
        }

        SetLineText(line, newLine, false);
    }

    delete []matchStartArray;
    delete []matchLenArray;

    return true;
}

// --------------------------------------------------------------------------
// Show dialogs

bool wxSTEditor::ShowInsertTextDialog()
{
    wxSTEditorInsertTextDialog dialog(this);

    return dialog.ShowModal() == wxID_OK;
}

bool wxSTEditor::ShowColumnizeDialog()
{
    wxString text(GetSelectedText());
    if (text.IsEmpty()) return false;

    wxSTEditorColumnizeDialog dialog(GetModalParent());
    dialog.GetTestEditor()->RegisterStyles(GetEditorStyles());
    dialog.GetTestEditor()->RegisterLangs(GetEditorLangs());
    dialog.GetTestEditor()->SetLanguage(GetLanguageId());
    dialog.SetText(text);
    dialog.FormatText();
    if ( dialog.ShowModal() != wxID_OK )
        return false;

    ReplaceSelection(dialog.GetText());
    return true;
}

bool wxSTEditor::ShowConvertEOLModeDialog()
{
    int eol_mode = GetEOLMode();

    wxSingleChoiceDialog dialog(GetModalParent(),
                      wxString(_("Current EOL : "))+wxSTC_EOL_Strings[eol_mode],
                      _("Convert End of Line chars"), wxSTC_EOL_Strings_count, wxSTC_EOL_Strings);
    dialog.SetSelection(eol_mode);

    if ( dialog.ShowModal() != wxID_OK )
        return false;

    int choice = dialog.GetSelection();

    if (GetEditorPrefs().IsOk())
        GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_EOL_MODE, choice);
    else
        SetEOLMode(choice);

    ConvertEOLs(choice);
    return true;
}

bool wxSTEditor::ShowSetZoomDialog()
{
    wxNumberEntryDialog numDlg(GetModalParent(),
                               _("Scale font sizes : -10...20 (not all fonts supported)"),
                               wxEmptyString,
                               _("Change text font size"),
                               GetZoom(), -10, 20, wxDefaultPosition);
    bool ok = (wxID_CANCEL != numDlg.ShowModal());
    if (ok)
    {
        int val = numDlg.GetValue();
        if (GetEditorPrefs().IsOk())
            GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_ZOOM, val);
        else
            SetZoom(val);
    }
    return ok;
}

bool wxSTEditor::ShowGotoLineDialog()
{
    wxString msg(wxString::Format(_("Line number : 1...%d"), GetLineCount()));
    long line = wxGetNumberFromUser(msg, wxEmptyString, _("Goto line"),
                                    GetCurrentLine()+1, 1, GetLineCount(), GetModalParent());

    if (line > 0)
    {
        GotoLine(line-1);
        return true;
    }
    return false;
}

// --------------------------------------------------------------------------
// Toggle folding

void wxSTEditor::ToggleFoldAtLine(int line)
{
    // Help STC figure where folds are, else you need to scroll to bottom before this works
    ColouriseDocument();
    if (line < 0) line = GetCurrentLine();

    if (!STE_HASBIT(GetFoldLevel(line), wxSTC_FOLDLEVELHEADERFLAG))
        line = GetFoldParent(line);
    if (line >= 0)
        ToggleFold(line);
}
void wxSTEditor::ExpandFoldsToLevel(int level, bool expand)
{
    // Help STC figure where folds are, else you need to scroll to bottom before this works
    ColouriseDocument();

    const int line_n = GetLineCount();
    for (int n = 0; n < line_n; n++)
    {
        int line_level = GetFoldLevel(n);
        if (STE_HASBIT(line_level, wxSTC_FOLDLEVELHEADERFLAG))
        {
            line_level -= wxSTC_FOLDLEVELBASE;
            line_level &= wxSTC_FOLDLEVELNUMBERMASK;

            if ((( expand && (line_level <= level)) ||
                 (!expand && (line_level >= level))) && (GetFoldExpanded(n) != expand))
                ToggleFold(n);
        }
    }

    EnsureCaretVisible(); // seems to keep it in nearly the same place
}

// --------------------------------------------------------------------------

#define STE_VERSION_STRING_SVN STE_VERSION_STRING // wxT(" svn 2920")

#if (wxVERSION_NUMBER >= 2902)
/*static*/ wxVersionInfo wxSTEditor::GetStEditorVersionInfo()
{
    return wxVersionInfo(STE_APPDISPLAYNAME, STE_MAJOR_VERSION, STE_MINOR_VERSION, STE_RELEASE_VERSION, STE_VERSION_STRING_SVN);
}
#else
/*static*/ wxString wxSTEditor::GetStEditorVersionString()
{
    return STE_VERSION_STRING_SVN;
}
#endif

// --------------------------------------------------------------------------
// Load/Save Functions

wxFileName wxSTEditor::GetFileName() const
{
    return GetSTERefData()->m_fileName;
}

void wxSTEditor::SetFileName(const wxFileName& fileName, bool send_event)
{
    if (GetSTERefData()->m_fileName != fileName)
    {
        GetSTERefData()->m_fileName = fileName;

        // Cleanup the filename and make it absolute
        if (fileName.FileExists())
            GetSTERefData()->m_fileName.Normalize();

        if (send_event)
        {
            SendEvent(wxEVT_STEDITOR_STATE_CHANGED, STE_FILENAME, GetState(), GetFileName().GetFullPath());
        }
    }
}

bool wxSTEditor::CopyFilePathToClipboard()
{
    return SetClipboardText(GetFileName().GetFullPath());
}

wxDateTime wxSTEditor::GetFileModificationTime() const
{
    return GetSTERefData()->m_modifiedTime;
}
void wxSTEditor::SetFileModificationTime(const wxDateTime &dt)
{
    GetSTERefData()->m_modifiedTime = dt;
}

bool wxSTEditor::LoadFile( wxInputStream& stream,
                           const wxFileName& fileName,
                           int flags,
                           wxWindow* parent,
                           const wxString& strEncoding)
{
    wxString str;
    bool ok = LoadFileToString(&str, stream, fileName, flags, parent, strEncoding);

    if (ok)
    {
        SetTextAndInitialize(str);

        SendEvent(wxEVT_STEDITOR_STATE_CHANGED, STE_FILENAME, GetState(), fileName.GetFullPath());
    }
    return ok;
}

bool wxSTEditor::LoadFile( const wxFileName &fileName_,
                           const wxString &extensions_,
                           bool query_if_changed,
                           const wxString& encoding )
{
    if (query_if_changed && GetOptions().HasEditorOption(STE_QUERY_SAVE_MODIFIED) &&
        (QuerySaveIfModified(true) == wxCANCEL))
    {
        return false;
    }

    wxFileName fileName = fileName_;
    wxString extensions(extensions_.Length() ? extensions_ : GetOptions().GetDefaultFileExtensions());

    if (fileName.GetFullPath().IsEmpty())
    {
        fileName = GetFileName();
        wxString path;

        if (!fileName.GetFullPath().IsEmpty())
        {
            wxFileName fn(fileName);
            path     = fileName.GetPath();
            fileName = wxFileName(wxEmptyString, fileName.GetFullName());
        }
        else
            path = GetOptions().GetDefaultFilePath();

        fileName = wxFileSelector(_("Open file"), path, fileName.GetFullPath(),
                                  wxEmptyString, extensions,
                                  wxFD_OPEN | wxFD_FILE_MUST_EXIST,
                                  GetModalParent());

        if (fileName.GetFullPath().IsEmpty())
            return false;
    }

    if (!fileName.FileExists())
        return false;

    if (!fileName.IsAbsolute())
        fileName.MakeAbsolute();

    GetOptions().SetDefaultFilePath(fileName.GetPath());

    int load_flags = STE_LOAD_FROM_DISK;
    if (GetEditorPrefs().IsOk())
        load_flags |= GetEditorPrefs().GetPrefInt(STE_PREF_LOAD_UNICODE);

    // use streams method to allow loading unicode files
    wxFileInputStream stream(fileName.GetFullPath());

    return stream.IsOk() && LoadFile(stream, fileName, load_flags, NULL, encoding);
}

bool wxSTEditor::LoadFileToString( wxString* str,
                                   wxInputStream& stream,
                                   const wxFileName& fileName,
                                   int flags,
                                   wxWindow* parent,
                                   const wxString& strEncoding )
{
    wxCHECK_MSG(str, false, wxS("string pointer must be provided") );

    wxTextEncoding::TextEncoding_Type encoding = wxTextEncoding::TypeFromString(strEncoding);
    bool show_errdlg = !STE_HASBIT(flags, STE_LOAD_NOERRDLG);
    flags = flags & (~STE_LOAD_NOERRDLG); // strip this to match flag

    const wxFileOffset stream_len = stream.GetLength();
    bool ok = (stream_len <= STE_MaxFileSize);

    if (!ok)
    {
        if (show_errdlg)
        {
            wxMessageBox(_("This file is too large for this editor, sorry."),
                         _("Error loading file"),
                         wxOK|wxICON_EXCLAMATION, parent);
        }

        return false;
    }
    {
        bool want_lang = GetEditorPrefs().IsOk() && GetEditorPrefs().GetPrefBool(STE_PREF_LOAD_INIT_LANG);
        wxCharBuffer charBuf(stream_len);
        wxBOM file_bom = wxBOM_None;

        if ((encoding == wxTextEncoding::Ascii) && dynamic_cast<wxStringInputStream*>(&stream))
        {
            // wxStringInputStream is utf8 always
            encoding = wxTextEncoding::UTF8;
        }

        ok = ((size_t)stream_len == stream.Read(charBuf.data(), stream_len).LastRead());

        if (ok)
        {
            bool found_lang = false;
            bool is_html    = false;
            bool is_xml     = false;

            if (want_lang && !found_lang)
            {
                found_lang = SetLanguage(fileName);
                if (found_lang)
                {
                    is_html = (0 == GetEditorLangs().GetName(GetLanguageId()).CmpNoCase(wxT("html")));
                    is_xml  = (0 == GetEditorLangs().GetName(GetLanguageId()).CmpNoCase(wxT("xml")));
                }
            }
            if ((want_lang && !found_lang) || ( (is_html || is_xml) && (encoding == wxTextEncoding::Ascii)) )
            {
                // sample just the first line; feeble but functional attempt to detect xml (in files w/o the .xml extension),
                // and/or utf8 encoding in html files (w html extension)
                // typical html: content="charset=utf-8"
                // typical xml: encoding="utf-8"
                const char* newline = strpbrk(charBuf.data(), "\n\r");
                size_t len = newline ? (newline - charBuf.data()) : stream_len;
                wxCharBuffer firstline(len);

                strncpy(firstline.data(), charBuf.data(), len);

                if (want_lang && !found_lang)
                {
                    const char xml_header[] = "<?xml version=\"";

                    if ( (len >= WXSIZEOF(xml_header)) &&
                         (0 == strncmp(xml_header, firstline.data(), WXSIZEOF(xml_header)-1))
                       )
                    {
                        found_lang = is_xml = SetLanguage(wxFileName(wxEmptyString, fileName.GetName(), wxT("xml")));
                    }
                }
                if (encoding == wxTextEncoding::Ascii)
                {
                    if (is_html)
                    {
                        wxTextEncoding::TypeFromString(&encoding, firstline.data(), "charset=", "; \"");
                    }
                    if (is_xml)
                    {
                        wxTextEncoding::TypeFromString(&encoding, firstline.data(), "encoding=\"", "\"");
                    }
                }
            }
            switch (encoding)
            {
                case wxTextEncoding::Ascii:
                    // load file and get BOM
                    ok = wxTextEncoding::CharToStringDetectBOM(str, charBuf, stream_len, &file_bom);
                #if !(wxUSE_UNICODE || (defined(wxUSE_UNICODE_UTF8) && wxUSE_UNICODE_UTF8))
                    if (ok) switch (file_bom)
                    {
                        case wxBOM_UTF16LE:
                            if (flags == STE_LOAD_QUERY_UNICODE)
                            {
                                int ret = wxMessageBox(_("Unicode text file. Convert to Ansi text?"),
                                                       _("Load Unicode?"),
                                                       wxYES_NO | wxCANCEL | wxCENTRE | wxICON_QUESTION,
                                                       parent);
                                switch (ret)
                                {
                                    case wxYES:
                                        break;
                                    case wxNO:
                                    case wxCANCEL:
                                    default:
                                        ok = false;
                                        break;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                #endif
                    if (ok) switch (file_bom)
                    {
                        case wxBOM_UTF8:    encoding = wxTextEncoding::UTF8;       break;
                        case wxBOM_UTF16LE: encoding = wxTextEncoding::Unicode_LE; break;
                        default:            encoding = wxTextEncoding::Ascii;      break;
                    }
                    break;
                case wxTextEncoding::Unicode_LE:
                case wxTextEncoding::UTF8:
                case wxTextEncoding::ISO8859_1:
            #ifdef __WXMSW__
                case wxTextEncoding::OEM:
            #endif
                    // get BOM
                    file_bom = wxConvAuto_DetectBOM(charBuf.data(), stream_len);

                    // load file
                    ok = wxTextEncoding::CharToString(str, charBuf, stream_len, encoding, file_bom);

                    break;
                default:
                    ok = false;
                    break;
            }
            if (ok)
            {
                // sanity check
                ok = !(stream_len && str->IsEmpty());
            }
            if (!ok)
            {
                wxMessageBox(_("Bad encoding."),
                             _("Error loading file"),
                             wxOK|wxICON_ERROR, parent);
                // give it one more shot
                if (wxTextEncoding::Ascii != encoding)
                {
                    ok = wxTextEncoding::CharToString(str, charBuf, stream_len, wxTextEncoding::Ascii);
                }
            }
        }
        if (ok)
        {
            SetFileEncoding(wxTextEncoding::TypeToString(encoding));
            SetFileBOM(file_bom != wxBOM_None);
            SetFileModificationTime(fileName.GetModificationTime());
            SetFileName(fileName, false);
        }
    }

    return ok;
}

bool wxSTEditor::SaveFile( wxOutputStream& stream, const wxString& encoding, bool file_bom)
{
    return wxTextEncoding::SaveFile(GetText(), stream, wxTextEncoding::TypeFromString(encoding), file_bom);
}

bool wxSTEditor::SaveFile( bool use_dialog, const wxString &extensions_ )
{
    wxFileName selectedFileName;
    wxString   selectedFileEncoding;
    bool       selected_file_bom = false;

    bool ok = SaveFileDialog(use_dialog, extensions_, &selectedFileName, &selectedFileEncoding, &selected_file_bom);

    if (ok)
    {
        ok = SaveFile(selectedFileName, selectedFileEncoding, selected_file_bom);

        // remember where they tried to save their file
        if (use_dialog)
            GetOptions().SetDefaultFilePath(selectedFileName.GetPath());
    }

    return ok;
}

bool wxSTEditor::SaveFile( const wxFileName& fileName,
                           const wxString& fileEncoding,
                           bool write_file_bom )
{
    wxFile file;

    // FIXME check for write permission wxAccess - access
    if (!file.Open(fileName.GetFullPath(), wxFile::write))
    {
        wxMessageBox(wxString::Format(_("Error opening file to save : '%s'"), fileName.GetFullPath(GetOptions().GetDisplayPathSeparator()).wx_str()),
                     _("Save file error"), wxOK|wxICON_ERROR , GetModalParent());
        return false;
    }

    if (GetEditorPrefs().IsOk())
    {
        if (GetEditorPrefs().GetPrefBool(STE_PREF_SAVE_REMOVE_WHITESP))
            RemoveTrailingWhitespace(0, -1);
        if (GetEditorPrefs().GetPrefBool(STE_PREF_SAVE_CONVERT_EOL))
            ConvertEOLs(GetEditorPrefs().GetPrefInt(STE_PREF_EOL_MODE));
    }

    wxFileOutputStream outStream(file);

    if (outStream.IsOk() && SaveFile(outStream, fileEncoding, write_file_bom))
    {
        file.Close();

        SetFileModificationTime(fileName.GetModificationTime());

        SetModified(false);
        SetFileName(fileName, true);
        UpdateCanDo(true);
        SetFileEncoding(fileEncoding);
        SetFileBOM(write_file_bom);
        return true;
    }
    else
    {
        wxMessageBox(wxString::Format(_("Error saving file : '%s'"), fileName.GetFullPath(GetOptions().GetDisplayPathSeparator()).wx_str()),
                     _("Save file error"), wxOK|wxICON_ERROR , GetModalParent());
    }
    return false;
}

bool wxSTEditor::SaveFileDialog( bool use_dialog, const wxString &extensions_,
                                 wxFileName* selectedFileName,
                                 wxString*   selectedFileEncoding,
                                 bool*       selected_file_bom)
{
    wxFileName fileName(GetFileName());
    wxString extensions(extensions_.IsEmpty() ? GetOptions().GetDefaultFileExtensions() : extensions_);
    wxString encoding  (GetFileEncoding());
    bool file_bom       = GetFileBOM();

    // if not a valid filename or it wasn't loaded from disk - force using dialog
    if (fileName.GetFullPath().IsEmpty() || !fileName.IsOk() || !IsFileFromDisk())
        use_dialog = true;

    if (use_dialog)
    {
        wxString path(GetOptions().GetDefaultFilePath());
        wxString fileNamePath(fileName.GetPath());

        if (!fileNamePath.IsEmpty())
            path = fileNamePath;

        wxSTEditorFileDialog fileDialog( this, _("Save file"),
                                         path,
                                         extensions,
                                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        fileDialog.SetFilename(fileName.GetFullName());
        fileDialog.m_encoding = encoding;
        fileDialog.m_file_bom = file_bom;

        if (fileDialog.ShowModal() == wxID_OK)
        {
            if (selectedFileName)     *selectedFileName     = fileDialog.GetPath();
            if (selectedFileEncoding) *selectedFileEncoding = fileDialog.m_encoding;
            if (selected_file_bom)    *selected_file_bom    = fileDialog.m_file_bom;
            return true;
        }
    }
    else
    {
        // use the current file info
        if (selectedFileName)     *selectedFileName     = fileName;
        if (selectedFileEncoding) *selectedFileEncoding = encoding;
        if (selected_file_bom)    *selected_file_bom    = file_bom;
        return true;
    }

    return false;
}

bool wxSTEditor::NewFile( const wxString &title_ )
{
    if (GetOptions().HasEditorOption(STE_QUERY_SAVE_MODIFIED) &&
         (QuerySaveIfModified(true) == wxCANCEL))
        return false;

    wxString title(title_);

    while (title.IsEmpty())
    {
        title = wxGetTextFromUser(_("New file name"), _("New file"),
                                  GetOptions().GetDefaultFileName(), GetModalParent());

        if (title.IsEmpty())
            return false;

        if (wxIsWild(title))
        {
            int ret = wxMessageBox(_("The filename contains wildcard characters."),
                                   _("Invalid filename"),
                                   wxOK|wxCANCEL|wxCENTRE|wxICON_ERROR, GetModalParent());

            if (ret == wxCANCEL)
                return false;
        }
    }

    SetFileModificationTime(wxInvalidDateTime); // set invalid time

    ClearAll();
    EmptyUndoBuffer();
    if (GetEditorPrefs().IsOk() && GetEditorPrefs().GetPrefBool(STE_PREF_LOAD_INIT_LANG))
    {
        SetLanguage(wxFileName(title));
    }

    SetFileName(wxFileName(title), true);
    UpdateCanDo(true);
    return true;
}

bool wxSTEditor::Revert()
{
    bool ok = (wxYES == wxMessageBox(_("Discard changes and load last saved version ?"),
                                    _("Revert changes?"),
                                    wxYES_NO | wxICON_QUESTION, GetModalParent()));
    if (ok)
    {
        ok = LoadFile(GetFileName(), wxEmptyString, false);
    }

    return ok;
}

void wxSTEditor::SetTextAndInitialize(const wxString& str)
{
    ClearAll();
    SetText(str);
    EmptyUndoBuffer();
    SetModified(false);
    GotoPos(0);
    ScrollToColumn(0); // extra help to ensure scrolled to 0
                       // otherwise scrolled halfway thru 1st char

    SetLanguage(GetSTERefData()->m_steLang_id); // -> Colourise();
    UpdateCanDo(IsShown());
}

int wxSTEditor::QuerySaveIfModified(bool save_file, int style)
{
    if (!IsModified())
        return wxNO;

    bool sendEvents = m_sendEvents;
    m_sendEvents = false; // block focus when dialog closes

    int ret = wxMessageBox(wxString::Format(_("%s\nHas unsaved changes.\nWould you like to save your file before closing?"),
                                 GetFileName().GetFullPath(GetOptions().GetDisplayPathSeparator()).wx_str()),
                           _("Unsaved changes"),
                           style|wxCENTRE|wxICON_QUESTION, GetModalParent());

    m_sendEvents = sendEvents;

    if (save_file && (ret == wxYES))
    {
        // use dialog if it wasn't originally loaded from disk
        if (!SaveFile(!IsFileFromDisk()))
        {
           ret = wxCANCEL;
        }
    }

    return ret;
}

bool wxSTEditor::IsAlteredOnDisk(bool show_reload_dialog)
{
    // do we currently have a valid filename and datetime from loading?
    if (!IsFileFromDisk()) return false;

    wxLogNull nullLog; // no errors, we handle them ourselves

    wxDateTime fileDateTime;
    wxFileName fileName(GetFileName());

    // The file should exist, unless they moved/deleted it
    if (fileName.FileExists())
        fileDateTime = fileName.GetModificationTime();

    if (!fileDateTime.IsValid())
    {
        // oops, file is gone, just tell them
        if (show_reload_dialog)
        {
            wxMessageBox(wxString::Format(_("%s\nDoesn't exist on disk anymore."),
                           GetFileName().GetFullPath(GetOptions().GetDisplayPathSeparator()).wx_str()),
                          _("File removed from disk"),
                          wxOK | wxICON_EXCLAMATION, GetModalParent());
        }

        // reset to unknown, assume they know what they're doing
        SetFileModificationTime(wxInvalidDateTime);
        return true;
    }

    bool altered = (GetFileModificationTime() != fileDateTime);

    if (altered && show_reload_dialog)
    {
        int ret = wxMessageBox( wxString::Format(_("The file '%s' has been modified externally.\nWould you like to reload the file?"),
                                    GetFileName().GetFullPath(GetOptions().GetDisplayPathSeparator()).wx_str()),
                                _("File changed on disk"),
                                wxYES_NO | wxICON_QUESTION, GetModalParent());
        if (ret == wxYES)
        {
            // try to put the editor back on the same line after loading
            int visibleLine = GetFirstVisibleLine() + LinesOnScreen();
            STE_TextPos currentPos = GetCurrentPos();
            LoadFile(GetFileName());
            GotoLine(wxMin(visibleLine, GetNumberOfLines()));
            LineScroll(0, -2);
            GotoPos(wxMin(currentPos, GetLength()));
        }
        else
        {
            // reset to unknown, they don't care so don't ask again
            SetFileModificationTime(wxInvalidDateTime);
        }
    }

    return altered;
}

void wxSTEditor::ShowPropertiesDialog()
{
    wxSTEditorPropertiesDialog dlg(this);

    if (dlg.Create(GetModalParent(), wxGetStockLabelEx(wxID_PROPERTIES, wxSTOCK_PLAINTEXT)))
    {
        dlg.ShowModal();
    }
}

bool wxSTEditor::ShowExportDialog()
{
    wxFileName fileName = GetFileName();
    int file_format;

    wxSTEditorExportDialog dialog(GetModalParent());
    file_format = dialog.GetFileFormat();
    fileName = dialog.FileNameExtChange(fileName, file_format);
    dialog.SetFileName(fileName);

    if ( dialog.ShowModal() != wxID_OK )
        return false;

    fileName    = dialog.GetFileName();
    file_format = dialog.GetFileFormat();

    wxSTEditorExporter steExport(this);

    return steExport.ExportToFile(file_format, fileName, true, true);
}

// --------------------------------------------------------------------------
// Find/Replace functions

wxSTEditorFindReplaceData *wxSTEditor::GetFindReplaceData() const
{
    return GetOptions().GetFindReplaceData();
}

wxString wxSTEditor::GetFindString() const
{
    wxCHECK_MSG(GetFindReplaceData(), wxEmptyString, wxT("Invalid find/replace data"));
    return GetFindReplaceData()->GetFindString();
}

wxString wxSTEditor::GetReplaceString() const
{
    wxCHECK_MSG(GetFindReplaceData(), wxEmptyString, wxT("Invalid find/replace data"));
    return GetFindReplaceData()->GetReplaceString();
}

int wxSTEditor::GetFindFlags() const
{
    wxCHECK_MSG(GetFindReplaceData(), 0, wxT("Invalid find/replace data"));
    return GetFindReplaceData()->GetFlags();
}

wxSTEditorFindReplaceDialog* wxSTEditor::GetCurrentFindReplaceDialog()
{
    // We can't really save a pointer to it since the notebook may create one.
    // In any case, there should ONLY EVER be one created at once.
    return wxDynamicCast(wxWindow::FindWindowByName(wxSTEditorFindReplaceDialogNameStr), wxSTEditorFindReplaceDialog);
}

void wxSTEditor::ShowFindReplaceDialog(bool find)
{
    wxSTEditorFindReplaceData* steFindReplaceData = GetFindReplaceData();
    wxCHECK_RET(steFindReplaceData != NULL, wxT("Invalid find/replace data"));
    wxSTEditorFindReplaceDialog* dialog = GetCurrentFindReplaceDialog();

    bool create = true;

    if (dialog != NULL)
    {
        if ((  find  && !(dialog->GetWindowStyle() & wxFR_REPLACEDIALOG)) ||
            ((!find) &&  (dialog->GetWindowStyle() & wxFR_REPLACEDIALOG)) )
        {
            create = false;
            dialog->Raise();
        }
        else
        {
            dialog->Destroy();
            dialog = NULL;
        }
    }

    bool is_results_editor = (wxDynamicCast(this, wxSTEditorFindResultsEditor) != NULL);

    if (create)
    {
        int style = STE_FR_NOALLDOCS;

        wxWindow *parent = GetParent();

        // try to set parent to notebook if possible
        while (parent && (wxDynamicCast(parent, wxSTEditorNotebook) == NULL))
            parent = parent->GetParent();

        // if we found a notebook then use it
        if (parent && wxDynamicCast(parent, wxSTEditorNotebook))
        {
            style = 0;
        }
        else
        {
            parent = GetParent();
            // try to set parent to splitter if possible
            if (wxDynamicCast(parent, wxSTEditorSplitter) == NULL)
                parent = this;
        }

        if (is_results_editor)
        {
            style = STE_SETBIT(style, STE_FR_NOALLDOCS,     true);
            style = STE_SETBIT(style, STE_FR_NOFINDALL,     true);
            style = STE_SETBIT(style, STE_FR_NOBOOKMARKALL, true);

            steFindReplaceData->SetFlag(STE_FR_ALLDOCS,     false);
            steFindReplaceData->SetFlag(STE_FR_FINDALL,     false);
            steFindReplaceData->SetFlag(STE_FR_BOOKMARKALL, false);
        }

        //style |= wxSTAY_ON_TOP; // it's annoying when it gets hidden
        SetStateSingle(STE_CANFIND, !GetFindString().IsEmpty());

        wxString selectedText(GetSelectedText());
        if (!selectedText.IsEmpty() && (selectedText.Length() < 100u))
            SetFindString(selectedText, true);

        dialog = new wxSTEditorFindReplaceDialog(parent,
                                                 steFindReplaceData,
                                                 wxGetStockLabelEx(find ? wxID_FIND : wxID_REPLACE, wxSTOCK_PLAINTEXT),
                                                 style | (find ? 0 : wxFR_REPLACEDIALOG));
        dialog->Show();
    }
}

void wxSTEditor::OnFindDialog(wxFindDialogEvent& event)
{
    wxSTERecursionGuard guard(m_rGuard_OnFindDialog);
    if (guard.IsInside()) return;

    // deal with this event if not all docs, else grandparent notebook will do it
    if (!STE_HASBIT(event.GetFlags(), STE_FR_ALLDOCS))
        HandleFindDialogEvent(event);
    else
        event.Skip(true);
}

void wxSTEditor::HandleFindDialogEvent(wxFindDialogEvent& event)
{
    wxCHECK_RET(GetFindReplaceData(), wxT("Invalid find/replace data"));

    wxEventType eventType = event.GetEventType();
    wxString findString   = event.GetFindString();
    long flags            = event.GetFlags();

    // For this event we only go to the previously found location.
    if (eventType == wxEVT_STEFIND_GOTO)
    {
        wxSTEditorFoundStringData foundStringData;
        if (foundStringData.FromString(findString))
            wxSTEditorFindReplaceData::GotoFindAllString(foundStringData, this);
        return;
    }

    SetStateSingle(STE_CANFIND, !findString.IsEmpty());
    SetFindString(findString, true);
    SetFindFlags(flags, true);

    STE_TextPos pos = GetCurrentPos();
    if ((eventType == wxEVT_COMMAND_FIND) && STE_HASBIT(flags, STE_FR_WHOLEDOC))
        pos = -1;

    // we have to move cursor to start of word if last backwards search suceeded
    //   note cmp is ok since regexp doesn't handle searching backwards
    if ((eventType == wxEVT_COMMAND_FIND_NEXT) && !STE_HASBIT(flags, wxFR_DOWN))
    {
        if ((labs(GetSelectionEnd() - GetSelectionStart()) == STE_TextPos(findString.Length()))
            && (GetFindReplaceData()->StringCmp(findString, GetSelectedText(), flags)))
                pos -= (STE_TextPos)findString.Length() + 1; // doesn't matter if it matches or not, skip it
    }

    if ((eventType == wxEVT_COMMAND_FIND) || (eventType == wxEVT_COMMAND_FIND_NEXT))
    {
        if (STE_HASBIT(flags, STE_FR_FINDALL|STE_FR_BOOKMARKALL))
        {
            wxArraySTEditorFoundStringData& foundStringArray = GetFindReplaceData()->GetFoundStringArray();
            wxArrayInt startPositions;
            wxArrayInt endPositions;
            size_t n, count = FindAllStrings(findString, flags,
                                             &startPositions, &endPositions);

            wxString name(GetFileName().GetFullName());

            for (n = 0; n < count; n++)
            {
                int line = LineFromPosition(startPositions[n]);
                if (STE_HASBIT(flags, STE_FR_BOOKMARKALL))
                    MarkerAdd(line, STE_MARKER_BOOKMARK);

                if (STE_HASBIT(flags, STE_FR_FINDALL))
                {
                    foundStringArray.Add(wxSTEditorFoundStringData(GetFileName(), 
                                                                   line, 
                                                                   PositionFromLine(line), 
                                                                   startPositions[n], 
                                                                   endPositions[n]-startPositions[n], 
                                                                   GetLine(line)));
                }
            }
        }
        else
        {
            pos = FindString(findString, pos, -1, flags, STE_FINDSTRING_SELECT|STE_FINDSTRING_GOTO);

        /*
            // alternate way to do this, but our method is more flexible
            SearchAnchor();
            if (STE_HASBIT(flags, wxFR_DOWN))
                pos = SearchNext(flags, findString);
            else
                pos = SearchPrev(flags, findString);
        */

        // FIXME - dialog for find no more occurances is annoying
        // if (pos < 0)
        // wxMessageBox(_("No more occurances of: \"") + findString + _("\""),
        //              _("Find string"), wxOK | wxICON_EXCLAMATION, this);

            if (pos >= 0)
            {
                //SetFocus(); FIXME
            }
            else
            {
                wxBell(); // bell ok to signify no more occurances?
                SetStateSingle(STE_CANFIND, false);
            }
        }
    }
    else if (eventType == wxEVT_COMMAND_FIND_REPLACE)
    {
        if (!SelectionIsFindString(findString, flags))
        {
            wxBell();
            return;
        }

        pos = GetSelectionStart();
        wxString replaceString(event.GetReplaceString());
        ReplaceSelection(replaceString);
        GotoPos(pos); // makes first part of selection visible
        SetSelection(pos, pos + (STE_TextPos)replaceString.Length());
        //SetFocus();
    }
    else if (eventType == wxEVT_COMMAND_FIND_REPLACE_ALL)
    {
        wxString replaceString(event.GetReplaceString());
        if (findString == replaceString)
            return;

        int count = 0;

        {
            wxBusyCursor busy;
            count = ReplaceAllStrings(findString, replaceString, flags);
        }

        wxString msg(wxString::Format(_("Replaced %d occurances of\n'%s' with '%s'."),
                                      count, findString.wx_str(), replaceString.wx_str()));

        wxWindow* parent = wxDynamicCast(event.GetEventObject(), wxWindow);
        wxMessageBox( msg, _("Finished replacing"),
                      wxOK|wxICON_INFORMATION,
                      parent ? parent : GetModalParent());

        SetStateSingle(STE_CANFIND, false);
    }
    else if (eventType == wxEVT_COMMAND_FIND_CLOSE)
    {
        //if (wxDynamicCast(event.GetEventObject(), wxDialog))
        //    ((wxDialog*)event.GetEventObject())->Destroy();
    }
}

void wxSTEditor::SetFindString(const wxString &findString, bool send_evt)
{
    wxString lastFindString(GetFindReplaceData()->GetFindString());

    GetFindReplaceData()->SetFindString(findString);

    if (findString.Length())
        GetFindReplaceData()->AddFindString(findString);

    if (send_evt && (lastFindString != findString))
    {
        SetStateSingle(STE_CANFIND, !findString.IsEmpty());
        SendEvent(wxEVT_STEDITOR_STATE_CHANGED, STE_CANFIND, GetState(), GetFileName().GetFullPath());
    }
}
void wxSTEditor::SetFindFlags(long flags, bool send_evt)
{
    if (send_evt && (GetFindReplaceData()->GetFlags() != flags))
    {
        GetFindReplaceData()->SetFlags(flags);
        SendEvent(wxEVT_STEDITOR_STATE_CHANGED, STE_CANFIND, GetState(), GetFileName().GetFullPath());
    }
}

STE_TextPos wxSTEditor::FindString(const wxString &findString,
                                   STE_TextPos start_pos, STE_TextPos end_pos,
                                   int flags,
                                   int action,
                                   STE_TextPos* found_start_pos, STE_TextPos* found_end_pos)
{
    if (findString.IsEmpty())
        return wxNOT_FOUND;

    if (flags == -1) flags = GetFindFlags();
    int sci_flags = wxSTEditorFindReplaceData::STEToScintillaFindFlags(flags);
    SetSearchFlags(sci_flags);

    int textLength = GetTextLength();

    if (STE_HASBIT(flags, wxFR_DOWN))
    {
        start_pos = (start_pos == -1) ? GetCurrentPos() : wxMax(0, wxMin(start_pos, textLength));
        end_pos   = (end_pos   == -1) ? textLength      : wxMax(0, wxMin(end_pos,   textLength));
        STE_TextPos s_pos = wxMin(start_pos, end_pos);
        STE_TextPos e_pos = wxMax(start_pos, end_pos);
        start_pos = s_pos;
        end_pos   = e_pos;
    }
    else
    {
        start_pos = (start_pos == -1) ? GetCurrentPos() : wxMax(0, wxMin(start_pos, textLength));
        end_pos   = (end_pos   == -1) ? 0               : wxMax(0, wxMin(end_pos,   textLength));
        STE_TextPos s_pos = wxMax(start_pos, end_pos);
        STE_TextPos e_pos = wxMin(start_pos, end_pos);
        start_pos = s_pos;
        end_pos   = e_pos;
    }

    // set the target to search within
    STE_TextPos target_start = GetTargetStart();
    STE_TextPos target_end   = GetTargetEnd();
    SetTargetStart(start_pos);
    SetTargetEnd(end_pos);

    // search for the string in target, target is changed to surround string
    STE_TextPos pos = SearchInTarget(findString);
    STE_TextPos search_target_start = GetTargetStart();
    STE_TextPos search_target_end   = GetTargetEnd();
    if (found_start_pos) *found_start_pos = search_target_start;
    if (found_end_pos  ) *found_end_pos   = search_target_end;

    // replace target to what it used to be
    SetTargetStart(target_start);
    SetTargetEnd(target_end);

    //wxPrintf(wxT("Find start %d end %d pos %d\n"), start_pos, end_pos, pos); fflush(stdout);

    if (pos >= 0)
    {
        if (STE_HASBIT(action, STE_FINDSTRING_GOTO))
            GotoPos(pos); // makes first part of selection visible
        if (STE_HASBIT(action, STE_FINDSTRING_SELECT))
            SetSelection(search_target_start, search_target_end);
    }
    else if (STE_HASBIT(flags, STE_FR_WRAPAROUND))
    {
        flags &= (~STE_FR_WRAPAROUND); // don't allow another recursion
        if (STE_HASBIT(flags, wxFR_DOWN))
        {
            pos = FindString(findString, 0, -1, flags, action,
                             found_start_pos, found_end_pos);
        }
        else
        {
            pos = FindString(findString, textLength, -1, flags, action,
                             found_start_pos, found_end_pos);
        }
    }

    return pos;
}

bool wxSTEditor::SelectionIsFindString(const wxString &findString, int flags)
{
    if (findString.IsEmpty()) return false;
    if (flags == -1) flags = GetFindFlags();
    bool is_found = false;

    flags &= (~STE_FR_WRAPAROUND); // just search selected text

    STE_TextPos sel_start = GetSelectionStart();
    STE_TextPos sel_end   = GetSelectionEnd();

    if (sel_start == sel_end) return false;

    STE_TextPos found_start_pos = 0;
    STE_TextPos found_end_pos   = 0;

    STE_TextPos find_pos = FindString(findString, sel_start, sel_end,
                                      flags, STE_FINDSTRING_NOTHING,
                                      &found_start_pos, &found_end_pos);

    if ((find_pos != -1) && (found_start_pos == sel_start) && (found_end_pos == sel_end))
        is_found = true;

    return is_found;
}

int wxSTEditor::ReplaceAllStrings(const wxString &findString,
                                  const wxString &replaceString,
                                  int flags)
{
    if (findString.IsEmpty() || (findString == replaceString))
        return 0;

    int count = 0;
    int replace_len = (int)replaceString.Length();
    if (flags == -1) flags = GetFindFlags();
    flags = (flags | wxFR_DOWN) & (~STE_FR_WRAPAROUND); // do it in a single pass
    STE_TextPos cursor_pos = GetCurrentPos();  // return here when done

    STE_TextPos pos = 0;
    STE_TextPos found_start_pos = 0;
    STE_TextPos found_end_pos   = 0;

    for (pos = FindString(findString, 0, -1, flags, STE_FINDSTRING_NOTHING,
                         &found_start_pos, &found_end_pos);
         pos != wxNOT_FOUND;
         pos = FindString(findString, pos + replace_len, -1, flags, STE_FINDSTRING_NOTHING,
                         &found_start_pos, &found_end_pos)
        )
    {
        ++count;
        SetTargetStart(found_start_pos);
        SetTargetEnd(found_end_pos);
        if (STE_HASBIT(flags, STE_FR_REGEXP))
            replace_len = ReplaceTargetRE(replaceString);
        else
            replace_len = ReplaceTarget(replaceString);

        // back up original cursor position to the "same" place
        if (pos < cursor_pos)
            cursor_pos += (replace_len - (found_end_pos-found_start_pos));
    }

    // return to starting pos or as close as possible
    //GotoPos(wxMin(cursor_pos, GetLength()));

    SetStateSingle(STE_CANFIND, findString != GetFindString());

    // extra check here since we've modified it, but it might not get an UI event
    if (count > 0)
        UpdateCanDo(true);

    return count;
}

size_t wxSTEditor::FindAllStrings(const wxString &str, int flags,
                                  wxArrayInt* startPositions,
                                  wxArrayInt* endPositions)
{
    // just start at beginning and look forward
    if (flags == -1) flags = GetFindFlags();
    flags = (flags | wxFR_DOWN) & (~STE_FR_WRAPAROUND); // do it in a single pass

    STE_TextPos pos = 0;
    STE_TextPos found_start_pos = 0;
    STE_TextPos found_end_pos   = 0;
    size_t count = 0;

    wxArrayInt positions;

    for (pos = FindString(str, 0, -1, flags, STE_FINDSTRING_NOTHING, &found_start_pos, &found_end_pos);
         pos != wxNOT_FOUND;
         pos = FindString(str, found_end_pos, -1, flags, STE_FINDSTRING_NOTHING, &found_start_pos, &found_end_pos))
    {
        count++;
        if (startPositions) startPositions->Add(found_start_pos);
        if (endPositions  ) endPositions->Add(found_end_pos);
    }

    return count;
}

// --------------------------------------------------------------------------
// Indicator functions

void wxSTEditor::SetIndicator(STE_TextPos pos, int len, int indic)
{
    STE_TextPos n, n_end = pos+len;
    for (n = pos; n < n_end; ++n)
    {
        int sty = GetStyleAt(n);
        StartStyling(n, wxSTC_INDICS_MASK);
        SetStyling(1, sty|indic);
    }
}

bool wxSTEditor::IndicateAllStrings(const wxString &str,
                                    int find_flags, int indic,
                                    wxArrayInt* startPositions1, wxArrayInt* endPositions1)
{
    wxString findString(str.IsEmpty() ? GetFindString() : str);
    if (find_flags == -1) find_flags = GetFindFlags();

    wxArrayInt startPositions2;
    wxArrayInt endPositions2;

    wxArrayInt* startPositions = startPositions1 ? startPositions1 : &startPositions2;
    wxArrayInt* endPositions   = endPositions1   ? endPositions1   : &endPositions2;


    size_t n, count = FindAllStrings(findString, find_flags,
                                     startPositions, endPositions);

    for (n = 0; n < count; n++)
    {
        SetIndicator(startPositions->Item(n),
                     endPositions->Item(n) - startPositions->Item(n), indic);
    }

    return count != 0;
}

bool wxSTEditor::ClearIndicator(int pos, int indic)
{
    int sty = GetStyleAt(pos);

    if (STE_HASBIT(sty, indic))
    {
        sty &= (~indic);
        StartStyling(pos, wxSTC_INDICS_MASK);
        SetStyling(1, sty);
        return true;
    }
    else
        return false;
}

int wxSTEditor::ClearIndication(int pos, int indic)
{
    int len = GetLength();
    int n = pos;

    for (n = pos; n >= 0; n--)
    {
        if (!ClearIndicator(n, indic))
            break;
    }

    for (n = pos+1; n < len; ++n)
    {
        if (!ClearIndicator(n, indic))
            break;
    }

    return n-1;
}

void wxSTEditor::ClearAllIndicators(int indic)
{
    int n, len = GetLength();
    for (n = 0; n < len; ++n)
        ClearIndicator(n, indic);
}

// --------------------------------------------------------------------------
// This code copied from SciTEBase.cxx
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
enum { noPPC, ppcStart, ppcMiddle, ppcEnd, ppcDummy };

int wxSTEditor::IsLinePreprocessorCondition(const wxString &line)
{
    if (!GetEditorLangs().IsOk() || line.IsEmpty())
        return noPPC;

    const wxString preprocessorSymbol = GetEditorLangs().GetPreprocessorSymbol(GetLanguageId());
    const wxString preprocCondStart   = GetEditorLangs().GetPreprocessorStart(GetLanguageId());
    const wxString preprocCondMiddle  = GetEditorLangs().GetPreprocessorMid(GetLanguageId());
    const wxString preprocCondEnd     = GetEditorLangs().GetPreprocessorEnd(GetLanguageId());

    const wxChar *currChar = line.GetData();
    wxString word;

    if (!currChar) {
        return false;
    }

    while (wxIsspace(*currChar) && *currChar)
        currChar++;

    if (!preprocessorSymbol.IsEmpty() && (*currChar == preprocessorSymbol))
    {
        currChar++;
        while (wxIsspace(*currChar) && *currChar)
            currChar++;

        while (!wxIsspace(*currChar) && *currChar)
            word += wxString(*currChar++);

        if (preprocCondStart.Contains(word))
            return ppcStart;

        if (preprocCondMiddle.Contains(word))
            return ppcMiddle;

        if (preprocCondEnd.Contains(word))
            return ppcEnd;
    }
    return noPPC;
}

// This code copied from SciTEBase.cxx
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
bool wxSTEditor::FindMatchingPreprocessorCondition(
    int &curLine,               ///< Number of the line where to start the search
    int direction,              ///< Direction of search: 1 = forward, -1 = backward
    int condEnd1,               ///< First status of line for which the search is OK
    int condEnd2)               ///< Second one

{
    bool isInside = false;
    wxString line;
    int status, level = 0;
    int maxLines = GetLineCount()-1;

    while ((curLine < maxLines) && (curLine > 0) && !isInside)
    {
        curLine += direction;   // Increment or decrement
        line = GetLine(curLine);
        status = IsLinePreprocessorCondition(line);

        if (((direction == 1) && (status == ppcStart)) || ((direction == -1) && (status == ppcEnd)))
            level++;
        else if ((level > 0) && (((direction == 1) && (status == ppcEnd)) || ((direction == -1) && (status == ppcStart))))
            level--;
        else if ((level == 0) && ((status == condEnd1) || (status == condEnd2)))
            isInside = true;
    }

    return isInside;
}

// This code copied from SciTEBase.cxx
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
bool wxSTEditor::FindMatchingPreprocCondPosition(
    bool isForward,             ///< @c true if search forward
    STE_TextPos& mppcAtCaret,   ///< Matching preproc. cond.: current position of caret
    STE_TextPos& mppcMatch)     ///< Matching preproc. cond.: matching position
{
    bool isInside = false;
    int curLine;
    wxString line;
    int status;

    if (!GetEditorLangs().IsOk())
        return false;

    // Get current line
    curLine = LineFromPosition(mppcAtCaret);
    line = GetLine(curLine);
    status = IsLinePreprocessorCondition(line);

    switch (status)
    {
        case ppcStart:
        {
            if (isForward)
                isInside = FindMatchingPreprocessorCondition(curLine, 1, ppcMiddle, ppcEnd);
            else
            {
                mppcMatch = mppcAtCaret;
                return true;
            }
            break;
        }
        case ppcMiddle:
        {
            if (isForward)
                isInside = FindMatchingPreprocessorCondition(curLine, 1, ppcMiddle, ppcEnd);
            else
                isInside = FindMatchingPreprocessorCondition(curLine, -1, ppcStart, ppcMiddle);

            break;
        }
        case ppcEnd:
        {
            if (isForward)
            {
                mppcMatch = mppcAtCaret;
                return true;
            }
            else
                isInside = FindMatchingPreprocessorCondition(curLine, -1, ppcStart, ppcMiddle);

            break;
        }
        default:        // Should be noPPC
        {
            if (isForward)
                isInside = FindMatchingPreprocessorCondition(curLine, 1, ppcMiddle, ppcEnd);
            else
                isInside = FindMatchingPreprocessorCondition(curLine, -1, ppcStart, ppcMiddle);

            break;
        }
    }

    if (isInside)
        mppcMatch = PositionFromLine(curLine);

    return isInside;
}

static bool IsBrace(char ch) {
    return ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == '{' || ch == '}';
}

// This code copied from SciTEBase.cxx
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
bool wxSTEditor::DoFindMatchingBracePosition(STE_TextPos& braceAtCaret, STE_TextPos& braceOpposite, bool sloppy)
{
    int maskStyle = (1 << GetStyleBitsNeeded()) - 1;
    bool isInside = false;

    int ste_languageID = GetLanguageId();

    int bracesStyle = GetEditorLangs().IsOk() && GetEditorLangs().HasLanguage(ste_languageID)
                         ? GetEditorLangs().GetBracesStyle(ste_languageID) : 10;
    int lexLanguage = GetLexer();
    int bracesStyleCheck = bracesStyle; // FIXME what is this?
    STE_TextPos caretPos = GetCurrentPos();
    braceAtCaret = -1;
    braceOpposite = -1;
    char charBefore = '\0';
    char styleBefore = '\0';
    int lengthDoc = GetLength();
    if ((lengthDoc > 0) && (caretPos > 0))
    {
        // Check to ensure not matching brace that is part of a multibyte character
        if (PositionBefore(caretPos) == (caretPos - 1))
        {
            charBefore = GetCharAt(caretPos-1);
            styleBefore = static_cast<char>(GetStyleAt(caretPos-1)&maskStyle);
        }
    }
    // Priority goes to character before caret
    if (charBefore && IsBrace(charBefore) &&
        ((styleBefore == bracesStyleCheck) || (!bracesStyle)))
    {
        braceAtCaret = caretPos - 1;
    }

    bool colonMode = false;
    if ((lexLanguage == wxSTC_LEX_PYTHON) &&
        (':' == charBefore) && (wxSTC_P_OPERATOR == styleBefore))
    {
        braceAtCaret = caretPos - 1;
        colonMode = true;
    }
    bool isAfter = true;
    if ((lengthDoc > 0) && sloppy && (braceAtCaret < 0) && (caretPos < lengthDoc))
    {
        // No brace found so check other side
        // Check to ensure not matching brace that is part of a multibyte character
        if (PositionAfter(caretPos) == (caretPos + 1))
        {
            char charAfter = GetCharAt(caretPos);
            char styleAfter = static_cast<char>(GetStyleAt(caretPos-1)&maskStyle);
            if (charAfter && IsBrace(charAfter) && ((styleAfter == bracesStyleCheck) || (!bracesStyle)))
            {
                braceAtCaret = caretPos;
                isAfter = false;
            }
            if ((lexLanguage == wxSTC_LEX_PYTHON) &&
                (':' == charAfter) && (wxSTC_P_OPERATOR == styleAfter))
            {
                braceAtCaret = caretPos;
                colonMode = true;
            }
        }
    }
    if (braceAtCaret >= 0)
    {
        if (colonMode)
        {
            int lineStart = LineFromPosition(braceAtCaret);
            int lineMaxSubord = GetLastChild(lineStart, -1);
            braceOpposite = GetLineEndPosition(lineMaxSubord);
        }
        else
            braceOpposite = BraceMatch(braceAtCaret);

        if (braceOpposite > braceAtCaret)
            isInside = isAfter;
        else
            isInside = !isAfter;
    }
    return isInside;
}
// This code copied from SciTEBase.cxx
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
void wxSTEditor::DoBraceMatch() {
    //if (!bracesCheck)
    //        return;
    STE_TextPos braceAtCaret = -1;
    STE_TextPos braceOpposite = -1;
    bool bracesSloppy = false; // FIXME added
    DoFindMatchingBracePosition(braceAtCaret, braceOpposite, bracesSloppy);

    //wxPrintf(wxT("Found brace %d, at %d opp %d\n"), int(found), braceAtCaret, braceOpposite);

    if ((braceAtCaret != -1) && (braceOpposite == -1))
    {
        BraceBadLight(braceAtCaret);
        SetHighlightGuide(0);
    }
    else // must run this to unhighlight last hilighted brace
    {
        char chBrace = 0;
        if (braceAtCaret >= 0)
            chBrace = static_cast<char>(GetCharAt(braceAtCaret));

        BraceHighlight(braceAtCaret, braceOpposite);
        int columnAtCaret = GetColumn(braceAtCaret);
        int columnOpposite = GetColumn(braceOpposite);
        if (chBrace == ':')
        {
            int lineStart = LineFromPosition(braceAtCaret);
            STE_TextPos indentPos = GetLineIndentPosition(lineStart);
            STE_TextPos indentPosNext = GetLineIndentPosition(lineStart + 1);
            columnAtCaret = GetColumn(indentPos);
            int columnAtCaretNext = GetColumn(indentPosNext);
            int indentSize = GetIndent();
            if (columnAtCaretNext - indentSize > 1)
                columnAtCaret = columnAtCaretNext - indentSize;
            if (columnOpposite == 0)        // If the final line of the structure is empty
                columnOpposite = columnAtCaret;
        }
        else
        {
            if (LineFromPosition(braceAtCaret) == LineFromPosition(braceOpposite))
            {
                // Avoid attempting to draw a highlight guide
                columnAtCaret = 0;
                columnOpposite = 0;
            }
        }

        // they only get hilighted when SetIndentationGuides is set true
        if (GetEditorPrefs().IsOk() && GetEditorPrefs().GetPrefInt(STE_PREF_INDENT_GUIDES))
            SetHighlightGuide(wxMin(columnAtCaret, columnOpposite));
    }
}

// This code copied from SciTEBase.cxx
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
STE_TextPos wxSTEditor::GetCaretInLine() {
        STE_TextPos caret = GetCurrentPos();
        int line = LineFromPosition(caret);
        STE_TextPos lineStart = PositionFromLine(line);
        return caret - lineStart;
}

// --------------------------------------------------------------------------
// Autocomplete functions

wxString wxSTEditor::GetAutoCompleteKeyWords(const wxString& root)
{
    wxString words;
    if (root.IsEmpty()) return words;

    wxArrayString wordArray;
    DoGetAutoCompleteKeyWords(root, wordArray);
    wordArray.Sort();

    size_t n, word_count = wordArray.GetCount();
    if (word_count > 0)
    {
        words += wordArray[0];

        for (n = 1; n < word_count; n++)
            words += wxT(" ") + wordArray[n];
    }

    return words;
}

size_t wxSTEditor::DoGetAutoCompleteKeyWords(const wxString& root, wxArrayString& wordArray)
{
    wxSTEditorLangs langs(GetEditorLangs());
    int lang_n = GetLanguageId();
    if (!langs.IsOk() || !langs.HasLanguage(lang_n)) return 0;

    size_t n, count = 0, keyword_count = langs.GetKeyWordsCount(lang_n);
    for (n = 0; n < keyword_count; n++)
    {
        wxStringTokenizer tkz(langs.GetKeyWords(lang_n, n));
        while ( tkz.HasMoreTokens() )
        {
            wxString token(tkz.GetNextToken());

            if (token.StartsWith(root) && (wordArray.Index(token) == wxNOT_FOUND))
            {
                count++;
                wordArray.Add(token);
            }
        }
    }

    return count;
}


// This code copied from SciTEBase.cxx (though it bears no resemblance to the original)
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
wxString wxSTEditor::EliminateDuplicateWords(const wxString& words0) const {
    wxString words;
    wxShadowObjectFields wordHashMap; // we just use the string part

    // make hashmap of words to remove duplicates and sort them
    wxStringTokenizer tokenizer(words0, wxT(" "));
    while ( tokenizer.HasMoreTokens() )
    {
        wxString token(tokenizer.GetNextToken());
        wordHashMap[token] = 0; // value doesn't matter
    }

    wxShadowObjectFields::const_iterator it = wordHashMap.begin(),
                                         it_end = wordHashMap.end();

    for ( ; it != it_end; it++)
    {
        words += it->first + wxT(" ");
    }

    if (!words.IsEmpty()) words.RemoveLast(1); // remove trailing space

    return words;
}

// This code copied from SciTEBase.cxx
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
bool wxSTEditor::StartAutoComplete() {
        wxString line = GetLine(GetCurrentLine());
        int current = GetCaretInLine();

        int startword = current;

        startword = WordStartPosition(current, true); // FIXME good enough?

        //while ((startword > 0) &&
        //        ((calltipWordCharacters.Find(line[startword - 1]) != wxNOT_FOUND) ||
        //         (autoCompleteStartCharacters.Find(line[startword - 1]) != wxNOT_FOUND))) {
        //       startword--;
        //}

        wxString root = line.Mid(startword, current - startword);

        if (!root.IsEmpty()) {
                //wxString words; //= apis.GetNearestWords(root.wx_str(), root.length(),
                                  //  autoCompleteIgnoreCase, calltipParametersStart[0]);
                wxString words = GetAutoCompleteKeyWords(root);
                if (!words.IsEmpty()) {
                        words = EliminateDuplicateWords(words);
                        AutoCompShow((int)root.Length(), words);
                }
                return true;
        }
        return false;
}

/*
int wxCMPFUNC_CONV wxSTE_StringSort(const wxString& first, const wxString& second)
{
    return first.Cmp(second);
}
int wxCMPFUNC_CONV wxSTE_StringSortNoCase(const wxString& first, const wxString& second)
{
    return first.CmpNoCase(second);
}
*/

// Default characters that can appear in a word
//static bool iswordcharforsel(wxChar ch) {
//        return !wxStrchr(wxT("\t\n\r !\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~"), ch);
//}

// This code copied from SciTEBase.cxx - NOT updated to 1.73
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
bool wxSTEditor::StartAutoCompleteWord(bool onlyOneWord, bool add_keywords) {
        bool autoCompleteIgnoreCase = false; // FIXME added

        wxString line = GetLine(GetCurrentLine());
        int current = GetCaretInLine();

        int startword = current;
        // Autocompletion of pure numbers is mostly an annoyance
        bool allNumber = true;
        while (startword > 0 && (wordCharacters.Find(line[startword - 1]) != wxNOT_FOUND)) {
                startword--;
                if (line[startword] < wxT('0') || line[startword] > wxT('9')) {
                        allNumber = false;
                }
        }
        if (startword == current || allNumber)
                return true;
        wxString root = line.Mid(startword, current - startword);

        int doclen = GetLength();
        //TextToFind ft = {{0, 0}, 0, {0, 0}};
        //ft.lpstrText = const_cast<char*>(root.wx_str());
        //ft.chrg.cpMin = 0;
        //ft.chrg.cpMax = doclen;
        //ft.chrgText.cpMin = 0;
        //ft.chrgText.cpMax = 0;
        const int flags = wxSTC_FIND_WORDSTART | (autoCompleteIgnoreCase ? 0 : wxSTC_FIND_MATCHCASE);
        STE_TextPos posCurrentWord = GetCurrentPos() - (STE_TextPos)root.length();
        size_t minWordLength = 0;
        size_t nwords = 0;

        // wordsNear contains a list of words separated by single spaces and with a space
        // at the start and end. This makes it easy to search for words.
        wxArrayString wordsNear; //(autoCompleteIgnoreCase ? wxSTE_StringSortNoCase : wxSTE_StringSort);
        //wxSortedArrayString wordsNear; //(autoCompleteIgnoreCase ? wxSTE_StringSortNoCase : wxSTE_StringSort);

        if (add_keywords)
        {
            DoGetAutoCompleteKeyWords(root, wordsNear);
            wordsNear.Sort();

        }

        int posFind = FindText(0, doclen, root, flags);
        while (posFind >= 0 && posFind < doclen) {      // search all the document
                int wordEnd = posFind + static_cast<int>(root.length());
                //wxPrintf(wxT("AutoComp '%s' pos %d/%d - %d\n"), root.c_str(), (int)wordEnd, (int)doclen, (int)wordsNear.GetCount());
                if (posFind != posCurrentWord) {
                        while (wordCharacters.Find(wordEnd < doclen ? (wxChar)GetCharAt(wordEnd) : wxT('\0')) != wxNOT_FOUND)
                                wordEnd++;
                        unsigned int wordLength = wordEnd - posFind;
                        if (wordLength > root.length()) {
                                wxString word = GetTextRange(posFind, wordEnd);
                                //word.insert(0, wxT("\n"));
                                //word.append(wxT("\n"));
                                if (wordsNear.Index(word) == wxNOT_FOUND) {        // add a new entry
                                        wordsNear.Add(word);
                                        if (minWordLength < wordLength)
                                                minWordLength = wordLength;

                                        nwords++;
                                        if (onlyOneWord && nwords > 1) {
                                                return true;
                                        }
                                }
                        }
                }
                //ft.chrg.cpMin = wordEnd;
                posFind = FindText(wordEnd, doclen, root, flags);
        }

        size_t length = wordsNear.GetCount();
        if ((length > 0) && (!onlyOneWord || (minWordLength > root.length()))) {
                wxString words(wordsNear[0]);
                for (size_t n = 1; n < length; n++)
                    words += wxT(" ") + wordsNear[n];

                AutoCompShow((int)root.length(), words);
        } else {
                AutoCompCancel();
        }
        return true;
}

// --------------------------------------------------------------------------
// Print functions

bool wxSTEditor::ShowPrintDialog()
{
#if STE_USE_HTML_PRINT
    wxHtmlEasyPrinting htmlPrint(_("Print document"));
    *htmlPrint.GetPrintData()     = *wxSTEditorPrintout::GetPrintData(true);
    *htmlPrint.GetPageSetupData() = *wxSTEditorPrintout::GetPageSetupData(true);
    wxSTEditorExporter steExport(this);
    bool ret = htmlPrint.PrintText(steExport.RenderAsHTML());
    if (ret)
    {
        *wxSTEditorPrintout::GetPrintData(true)     = *htmlPrint.GetPrintData();
        *wxSTEditorPrintout::GetPageSetupData(true) = *htmlPrint.GetPageSetupData();
    }
    return ret;

#else //!STE_USE_HTML_PRINT
    wxPrintData *printData = wxSTEditorPrintout::GetPrintData(true);

    wxPrintDialogData printDialogData( *printData );
    wxPrinter printer(&printDialogData);
    wxSTEditorPrintout printout(this);

    if (!printer.Print(GetModalParent(), &printout, true))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
        {
            wxMessageBox( _("A print error occurred, perhaps your printer is not correctly setup?"),
                          _("Print error"), wxOK|wxICON_ERROR, GetModalParent());
            return false;
        }
    }

    *printData = printer.GetPrintDialogData().GetPrintData();
    return true;
#endif //STE_USE_HTML_PRINT
}

bool wxSTEditor::ShowPrintPreviewDialog()
{
#if STE_USE_HTML_PRINT
    wxHtmlEasyPrinting htmlPrint(_("Preview document"));
    *htmlPrint.GetPrintData()     = *wxSTEditorPrintout::GetPrintData(true);
    *htmlPrint.GetPageSetupData() = *wxSTEditorPrintout::GetPageSetupData(true);
    wxSTEditorExporter steExport(this);
    bool ret = htmlPrint.PreviewText(steExport.RenderAsHTML());
    if (ret)
    {
        *wxSTEditorPrintout::GetPrintData(true)     = *htmlPrint.GetPrintData();
        *wxSTEditorPrintout::GetPageSetupData(true) = *htmlPrint.GetPageSetupData();
    }
    return ret;

#else //!STE_USE_HTML_PRINT
    wxPrintData *printData = wxSTEditorPrintout::GetPrintData(true);

    wxPrintDialogData printDialogData( *printData );
    wxPrintPreview *preview = new wxPrintPreview(new wxSTEditorPrintout(this),
                                                 new wxSTEditorPrintout(this),
                                                 &printDialogData);
    if (!preview->IsOk())
    {
        delete preview;

        wxMessageBox(_("A print error occurred, perhaps your printer is not correctly setup?"),
                     _("Print preview error"), wxOK|wxICON_ERROR, GetModalParent());
        return false;
    }

    wxPreviewFrame *frame = new wxPreviewFrameEx(preview, this, wxGetStockLabelEx(wxID_PREVIEW, wxSTOCK_PLAINTEXT));
    frame->SetIcons(wxSTEditorArtProvider::GetDialogIconBundle()); // use the pencil even in embedded wxStEdit
    ::wxFrame_ClonePosition(frame, this); // Clone main frame position
    frame->Initialize();
    frame->Show();
    return true;
#endif //STE_USE_HTML_PRINT
}

bool wxSTEditor::ShowPrintSetupDialog()
{
    wxPrintData *printData = wxSTEditorPrintout::GetPrintData(true);

    // Nah, these are separate things
    //if (GetEditorPrefs().IsOk())
    //    printData->SetColour(GetEditorPrefs().GetPrefInt(STE_PREF_PRINTCOLOURMODE) != wxSTC_PRINT_BLACKONWHITE);

    wxPrintDialogData printDialogData(*printData);
    wxPrintDialog printerDialog(GetModalParent(), &printDialogData);

    bool ok = (wxID_CANCEL != printerDialog.ShowModal());
    if (ok)
    {
        *printData = printerDialog.GetPrintDialogData().GetPrintData();
    }
    return ok;
}

bool wxSTEditor::ShowPrintPageSetupDialog()
{
    wxPageSetupData *pageSetupData = wxSTEditorPrintout::GetPageSetupData(true);
    wxPrintData *printData = wxSTEditorPrintout::GetPrintData(true);
    *pageSetupData = *printData;

    wxPageSetupDialog pageSetupDialog(GetModalParent(), pageSetupData);
    bool ok = (wxID_CANCEL != pageSetupDialog.ShowModal());
    if (ok)
    {
        *printData = pageSetupDialog.GetPageSetupData().GetPrintData();
        *pageSetupData = pageSetupDialog.GetPageSetupData();
    }
    return ok;
}

bool wxSTEditor::ShowPrintOptionsDialog()
{
    wxSTEditorPrintOptionsDialog dialog(GetModalParent());
    bool ok = (wxID_OK == dialog.ShowModal());
    if (ok)
    {
        if (GetEditorPrefs().IsOk())
        {
            GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_PRINT_COLOURMODE,    dialog.GetPrintColourMode(), false);
            GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_PRINT_MAGNIFICATION, dialog.GetPrintMagnification(), false);
            GetEditorPrefs().SetPrefBoolByID(ID_STE_PREF_PRINT_WRAPMODE,     dialog.GetPrintWrapMode(), false);
            GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_PRINT_LINENUMBERS,   dialog.GetPrintLinenumbers(), true);
        }
        else
        {
            SetPrintColourMode(dialog.GetPrintColourMode());
            SetPrintMagnification(dialog.GetPrintMagnification());
            SetPrintWrapMode(dialog.GetPrintWrapMode());
            // oops no way to set this
        }
    }
    return ok;
}

// --------------------------------------------------------------------------
// Update menu items

void wxSTEditor::UpdateAllItems()
{
    UpdateItems(GetOptions().GetEditorPopupMenu(), GetOptions().GetMenuBar(),
                                                   GetOptions().GetToolBar());
    UpdateItems(GetOptions().GetNotebookPopupMenu());
    UpdateItems(GetOptions().GetSplitterPopupMenu());
}

void wxSTEditor::UpdateItems(wxMenu *menu, wxMenuBar *menuBar, wxToolBar *toolBar)
{
    if (!menu && !menuBar && !toolBar) return;

    const bool readonly = GetReadOnly();
    const bool fold = GetMarginWidth(STE_MARGIN_FOLD) > 0;
    const bool sel = HasSelection();
    const bool sel_lines = !sel ? false : (LineFromPosition(GetSelectionStart()) !=
                                          (LineFromPosition(GetSelectionEnd())));

    // Edit menu items
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_SAVE,         CanSave());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_REVERT,       IsModified() && IsFileFromDisk());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_CUT,          CanCut());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_COPY,         CanCopy());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_COPY_HTML,  CanCopy());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_COPY_PRIMARY, CanCopy());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_PASTE,        CanPaste());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_PASTE_NEW,  IsClipboardTextAvailable());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_PASTE_RECT, CanPaste());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_COMPLETEWORD, !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_CLEAR,          !readonly);

    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_LINE_CUT,       !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_LINE_DELETE,    !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_LINE_TRANSPOSE, !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_LINE_DUPLICATE, !readonly);

    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_FIND_NEXT, CanFind());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_FIND_PREV, CanFind());
    STE_MM::DoCheckItem( menu, menuBar, toolBar, ID_STE_FIND_DOWN, GetFindDown());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_REPLACE,     !readonly);

    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_UNDO, CanUndo());
    STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_REDO, CanRedo());

    STE_MM::DoCheckItem(menu, menuBar, toolBar, ID_STE_READONLY, readonly);

    // Tool menu items
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_UPPERCASE,       !readonly && sel);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_LOWERCASE,       !readonly && sel);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_INCREASE_INDENT, !readonly && sel);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_DECREASE_INDENT, !readonly && sel);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_LINES_JOIN,      !readonly && sel_lines);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_LINES_SPLIT,     !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_TABS_TO_SPACES,  !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_SPACES_TO_TABS,  !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_CONVERT_EOL,     !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_TRAILING_WHITESPACE, !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_REMOVE_CHARSAROUND,  !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_INSERT_TEXT,     !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_INSERT_DATETIME, !readonly);
    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_COLUMNIZE,       !readonly && sel_lines);

    wxSTEUpdateSearchCtrl(toolBar, ID_STE_TOOLBAR_SEARCHCTRL, GetFindReplaceData());

    // View menu items
    STE_MM::DoSetTextItem(menu, menuBar, ID_STE_PREF_EDGE_COLUMN,
                          wxString::Format(_("Long l&ine guide column (%d)..."), GetEdgeColumn()));

    STE_MM::DoEnableItem(menu,  menuBar, toolBar, ID_STE_FOLDS_TOGGLE_CURRENT, fold);
    STE_MM::DoEnableItem(menu,  menuBar, toolBar, ID_STE_FOLDS_COLLAPSE_LEVEL, fold);
    STE_MM::DoEnableItem(menu,  menuBar, toolBar, ID_STE_FOLDS_EXPAND_LEVEL,   fold);
    STE_MM::DoEnableItem(menu,  menuBar, toolBar, ID_STE_FOLDS_COLLAPSE_ALL,   fold);
    STE_MM::DoEnableItem(menu,  menuBar, toolBar, ID_STE_FOLDS_EXPAND_ALL,     fold);

    STE_MM::DoSetTextItem(menu, menuBar, ID_STE_PREF_ZOOM,
                          wxString::Format(_("&Scale font size (%d)..."), GetZoom()));
    STE_MM::DoCheckItem(menu, menuBar, toolBar, ID_STE_VIEW_NONPRINT, GetViewNonPrint());
    STE_MM::DoCheckItem(menu, menuBar, toolBar, ID_STE_PREF_VIEW_EOL, GetViewEOL());
    STE_MM::DoCheckItem(menu, menuBar, toolBar, ID_STE_PREF_VIEW_WHITESPACE, (wxSTC_WS_INVISIBLE != GetViewWhiteSpace()) ? true : false);

    // Pref menu items
    STE_MM::DoSetTextItem(menu, menuBar, ID_STE_PREF_TAB_WIDTH,    wxString::Format(_("Set tab &width (%d)..."), GetTabWidth()));
    STE_MM::DoSetTextItem(menu, menuBar, ID_STE_PREF_INDENT_WIDTH, wxString::Format(_("Set indent wi&dth (%d)..."), GetIndent()));
    STE_MM::DoSetTextItem(menu, menuBar, ID_STE_PREF_EOL_MODE,     _("&EOL Mode (")+wxSTC_EOL_Strings[GetEOLMode()].BeforeFirst(wxT(' '))+wxT(")..."));

    if (GetEditorPrefs().IsOk())
        GetEditorPrefs().UpdateMenuToolItems(menu, menuBar, toolBar);
}

bool wxSTEditor::HandleMenuEvent(wxCommandEvent& event)
{
    wxSTERecursionGuard guard(m_rGuard_HandleMenuEvent);
    if (guard.IsInside()) return false;

    int win_id = event.GetId();

    switch (win_id)
    {
        // File menu items ----------------------------------------------------
        case wxID_NEW    : NewFile(wxEmptyString); return true;
        case wxID_OPEN   : LoadFile(); return true;
        case wxID_SAVE   : SaveFile(false); return true;
        case wxID_SAVEAS : SaveFile(true); return true;
        case wxID_REVERT : Revert(); return true;

        case ID_STE_EXPORT : ShowExportDialog(); return true;

        case ID_STE_PROPERTIES : ShowPropertiesDialog(); return true;

        case wxID_PRINT              : ShowPrintDialog(); return true;
        case wxID_PREVIEW            : ShowPrintPreviewDialog(); return true;
        case wxID_PRINT_SETUP        : ShowPrintSetupDialog(); return true;
        case ID_STE_PRINT_PAGE_SETUP : ShowPrintPageSetupDialog(); return true;
        case ID_STE_PRINT_OPTIONS    : ShowPrintOptionsDialog(); return true;
        // Edit menu items ----------------------------------------------------
        case wxID_CUT         : Cut();   return true;
        case wxID_COPY        : Copy();  return true;
        case ID_STE_COPY_HTML :
        {
            wxSTEditorExporter steExport(this);
            wxString text(steExport.RenderAsHTML(GetSelectionStart(), GetSelectionEnd()));

        #ifdef __WXMSW__
            text.Replace(wxT("\n"), wxTextBuffer::GetEOL(wxTextFileType_Dos)); // to make Notepad happy
        #endif
            SetClipboardHtml(text);
            return true;
        }
        case ID_STE_COPY_PRIMARY :
        {
            SetClipboardText(GetSelectedText(), STE_CLIPBOARD_BOTH);
            return true;
        }
        case wxID_PASTE        : Paste(); return true;
        case ID_STE_PASTE_RECT : PasteRectangular(); return true;
        case wxID_CLEAR        :
        {
            // Let scintilla handle the WXK_DELETE so it can be used in the macro recorder
            // and we guarantee the same behavior for the menu item and the delete key.
        #if (wxVERSION_NUMBER >= 2900)
            wxUIActionSimulator().Char(WXK_DELETE);
        #else
            wxKeyEvent keyEvent(wxEVT_KEY_DOWN);
            keyEvent.m_keyCode = WXK_DELETE;
            wxStyledTextCtrl::OnKeyDown(keyEvent);
        #endif
            return true;
        }
        case ID_STE_PREF_SELECTION_MODE    :
        {
            if (GetEditorPrefs().IsOk())
            {
                GetEditorPrefs().SetPrefInt(STE_PREF_SELECTION_MODE,
                    event.IsChecked() ? wxSTC_SEL_RECTANGLE : wxSTC_SEL_STREAM);
            }

            if (event.IsChecked())
                SetSelectionMode(wxSTC_SEL_RECTANGLE);
            else
                Cancel();

            return true;
        }
        case wxID_SELECTALL        : SelectAll(); return true;

        case ID_STE_LINE_CUT       : LineCut(); return true;
        case ID_STE_LINE_COPY      : LineCopy(); return true;
        case ID_STE_LINE_DELETE    : LineDelete(); return true;
        case ID_STE_LINE_TRANSPOSE : LineTranspose(); return true;
        case ID_STE_LINE_DUPLICATE : LineDuplicate(); return true;

        case wxID_FIND             : ShowFindReplaceDialog(true); return true;

        case ID_STE_TOOLBAR_SEARCHCTRL_MENU0 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU1 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU2 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU3 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU4 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU5 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU6 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU7 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU8 :
        case ID_STE_TOOLBAR_SEARCHCTRL_MENU9 :
        {
            wxSTEditorFindReplaceData* findReplaceData = GetFindReplaceData();
            if (findReplaceData == NULL) return true;

            int index = win_id - ID_STE_TOOLBAR_SEARCHCTRL_MENU0;
            if (index >= (int)findReplaceData->GetFindStrings().GetCount()) return true;
            wxString findString(findReplaceData->GetFindStrings()[index]);

            SetFindString(findString, true);
            wxCommandEvent menuEvent(wxEVT_COMMAND_MENU_SELECTED, ID_STE_FIND_NEXT);
            GetEventHandler()->AddPendingEvent(menuEvent);
            return true;
        }
        case ID_STE_TOOLBAR_SEARCHCTRL :
        {
            if (event.GetEventType() == wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN)
            {
                //wxSearchCtrl* searchCtrl = wxDynamicCast(event.GetEventObject(), wxSearchCtrl);
                //if (searchCtrl != NULL)
                //    searchCtrl->Clear();

                return true;
            }
            else if (event.GetEventType() == wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN)
            {
                // popup menu is shown
                //return true;
            }

            wxString findString(event.GetString());
            if (GetFindString() != findString)
                SetFindString(findString, true);

            // call our find next processing code
            win_id = ID_STE_FIND_NEXT;

            // fall though...
        }
        case ID_STE_FIND_PREV :
        case ID_STE_FIND_NEXT :
        {
            // try our parents (stenotebook) to see if they can handle it
            // we need to use a wxFindDialogEvent to set string and flags
            wxFindDialogEvent event2(wxEVT_COMMAND_FIND_NEXT, GetId());
            event2.SetEventObject(this);
            event2.SetFindString(GetFindString());
            int orig_flags = GetFindFlags();
            int flags = orig_flags & ~(STE_FR_FINDALL | STE_FR_BOOKMARKALL);
            if (win_id == ID_STE_FIND_PREV)
            {
               flags = STE_SETBIT(flags, wxFR_DOWN, !(flags & wxFR_DOWN));
            }
            event2.SetFlags(flags);
            if (!GetParent()->GetEventHandler()->ProcessEvent(event2))
                FindString(GetFindString(), GetCurrentPos(), -1, flags);

            // restore flags to how they were before
            SetFindFlags(orig_flags, true);

            // Make it easy to keep pressing enter to search again, normally the editor gets the focus.
            if ((event.GetId() == ID_STE_TOOLBAR_SEARCHCTRL) &&
                (wxDynamicCast(event.GetEventObject(), wxWindow) != NULL))
            {
                wxDynamicCast(event.GetEventObject(), wxWindow)->SetFocus();
            }

            return true;
        }
        case ID_STE_FIND_DOWN :
        {
            int flags = GetFindFlags();
            flags = STE_SETBIT(flags, wxFR_DOWN, event.IsChecked());
            SetFindFlags(flags, true);
            UpdateAllItems(); // help toolbar get updated
            return true;
        }
        case wxID_REPLACE : ShowFindReplaceDialog(false); return true;

        case ID_STE_GOTO_LINE : ShowGotoLineDialog(); return true;

        case wxID_REDO : Redo(); return true;
        case wxID_UNDO : Undo(); return true;

        case ID_STE_READONLY     : SetReadOnly(event.IsChecked()); return true;
        case ID_STE_COMPLETEWORD : StartAutoCompleteWord(false, true); return true;
        case ID_STE_COPYPATH     : CopyFilePathToClipboard(); return true;

        // Tools menu items ---------------------------------------------------
        case ID_STE_UPPERCASE   : UpperCase(); /* CmdKeyExecute(wxSTC_CMD_UPPERCASE); */ return true;
        case ID_STE_LOWERCASE   : LowerCase(); /* CmdKeyExecute(wxSTC_CMD_LOWERCASE); */ return true;
        case ID_STE_LINES_JOIN  :
        case ID_STE_LINES_SPLIT :
        {
            SetTargetStart(GetSelectionStart());
            SetTargetEnd(GetSelectionEnd());
            if (win_id == ID_STE_LINES_JOIN)
                LinesJoin();
            else
                LinesSplit(TextWidth(wxSTC_STYLE_DEFAULT, wxString(wxT('W'), GetEdgeColumn())));
            return true;
        }
        case ID_STE_INCREASE_INDENT : SetIndentation(GetIndent()); return true;
        case ID_STE_DECREASE_INDENT : SetIndentation(-GetIndent()); return true;

        case ID_STE_TABS_TO_SPACES  : ConvertTabsToSpaces(true); return true;
        case ID_STE_SPACES_TO_TABS  : ConvertTabsToSpaces(false); return true;
        case ID_STE_CONVERT_EOL     : ShowConvertEOLModeDialog(); return true;
        case ID_STE_VIEW_NONPRINT   : SetViewNonPrint(event.IsChecked()); return true;

        case ID_STE_TRAILING_WHITESPACE : RemoveTrailingWhitespace(); return true;
        case ID_STE_REMOVE_CHARSAROUND  : RemoveCharsAroundPos(); return true;

        case ID_STE_INSERT_TEXT     : ShowInsertTextDialog(); return true;
        case ID_STE_INSERT_DATETIME : ReplaceSelection(wxDateTime::Now().Format()); return true;

        case ID_STE_COLUMNIZE : ShowColumnizeDialog(); return true;

        // View menu items ----------------------------------------------------
        case ID_STE_FOLDS_TOGGLE_CURRENT : ToggleFoldAtLine(); return true;

        case ID_STE_FOLDS_COLLAPSE_LEVEL :
        {
            int num = wxGetNumberFromUser(_("Level to collapse all folds to"),
                                          wxEmptyString, _("Collapse folds to level"),
                                          (GetFoldLevel(GetCurrentLine())&wxSTC_FOLDLEVELNUMBERMASK)-wxSTC_FOLDLEVELBASE,
                                          0, wxSTC_FOLDLEVELNUMBERMASK-wxSTC_FOLDLEVELBASE, GetModalParent());
            if (num >= 0)
                CollapseFoldsToLevel(num);
            return true;
        }
        case ID_STE_FOLDS_EXPAND_LEVEL   :
        {
            int num = wxGetNumberFromUser(_("Level to expand all folds to"),
                                          wxEmptyString, _("Expand folds to level"),
                                          (GetFoldLevel(GetCurrentLine())&wxSTC_FOLDLEVELNUMBERMASK)-wxSTC_FOLDLEVELBASE,
                                          0, wxSTC_FOLDLEVELNUMBERMASK-wxSTC_FOLDLEVELBASE, GetModalParent());
            if (num >= 0)
                ExpandFoldsToLevel(num);
            return true;
        }
        case ID_STE_FOLDS_COLLAPSE_ALL : CollapseAllFolds();  return true;
        case ID_STE_FOLDS_EXPAND_ALL   : ExpandAllFolds();    return true;
        case ID_STE_PREF_ZOOM          : ShowSetZoomDialog(); return true;
        // Bookmark menu items ------------------------------------------------
        case ID_STE_BOOKMARKS :
        {
            wxSTEditorBookmarkDialog(this, _("Windows"));
            return true;

            // TODO : make a full dialog to add and delete markers and work with notebook
            wxArrayString bookmarks;
            int current_line = (int)GetCurrentLine();
            int best_marker = 0;
            int best_marker_distance = 100000;

            int line = MarkerNext(0, 1<<STE_MARKER_BOOKMARK);
            while (line >= 0)
            {
                if (labs(current_line - line) < best_marker_distance)
                    best_marker = (int)bookmarks.GetCount();

                wxString s(wxString::Format(wxT("%-5d : "), line+1) + GetLineText(line));
                if (s.Length() > 100) s = s.Mid(0, 100) + wxT("...");
                bookmarks.Add(s);
                line = MarkerNext(line+1, 1<<STE_MARKER_BOOKMARK);
            }

            if (bookmarks.empty())
                bookmarks.Add(_("No bookmarks in document"));

#if (wxVERSION_NUMBER > 2900)
            wxString choice = wxGetSingleChoice(_("Choose a bookmark to go to"), _("Bookmarks"),
                                                bookmarks, best_marker, this);
#else
            wxString choice = wxGetSingleChoice(_("Choose a bookmark to go to"), _("Bookmarks"),
                                                bookmarks, this);
#endif
            if (!choice.IsEmpty())
            {
                long l = 0;
                if (choice.BeforeFirst(wxT(' ')).Trim(false).ToLong(&l))
                    GotoLine(l-1);
            }
            return true;
        }
        case ID_STE_BOOKMARK_TOGGLE :
        {
            if ((MarkerGet(GetCurrentLine()) & (1<<STE_MARKER_BOOKMARK)) != 0)
                MarkerDelete(GetCurrentLine(), STE_MARKER_BOOKMARK);
            else
                MarkerAdd(GetCurrentLine(), STE_MARKER_BOOKMARK);
            return true;
        }
        case ID_STE_BOOKMARK_FIRST :
        {
            int line = MarkerNext(0, 1<<STE_MARKER_BOOKMARK);
            if (line != -1)
                GotoLine(line);
            return true;
        }
        case ID_STE_BOOKMARK_PREVIOUS :
        {
            // Note: Scintilla is forgiving about invalid line nums (at least for now)
            int line = MarkerPrevious(GetCurrentLine()-1, 1<<STE_MARKER_BOOKMARK);
            if (line != -1)
                GotoLine(line);
            return true;
        }
        case ID_STE_BOOKMARK_NEXT :
        {
            int line = MarkerNext(GetCurrentLine()+1, 1<<STE_MARKER_BOOKMARK);
            if (line != -1)
                GotoLine(line);
            return true;
        }
        case ID_STE_BOOKMARK_LAST :
        {
            int line = MarkerPrevious(GetLineCount(), 1<<STE_MARKER_BOOKMARK);
            if (line != -1)
                GotoLine(line);
            return true;
        }
        case ID_STE_BOOKMARK_CLEAR :
        {
            MarkerDeleteAll(STE_MARKER_BOOKMARK);
            return true;
        }
        // Preference menu items ----------------------------------------------
        case ID_STE_PREFERENCES :
        {
            if (GetEditorPrefs().IsOk() || GetEditorStyles().IsOk() || GetEditorLangs().IsOk())
            {
                wxSTEditorPrefPageData editorData(GetEditorPrefs(),
                                                  GetEditorStyles(),
                                                  GetEditorLangs(),
                                                  GetLanguageId(),
                                                  this);

                wxSTEditorPrefDialog prefDialog(editorData, GetModalParent());

                prefDialog.ShowModal();
            }

            return true;
        }
        case ID_STE_PREF_EDGE_COLUMN :
        {
            int val = wxGetNumberFromUser(_("Column to show long line marker"),
                                          wxEmptyString, _("Set long line marker"),
                                          GetEdgeColumn(),
                                          0, 255, GetModalParent());
            if (val >= 0)
            {
                if (GetEditorPrefs().IsOk())
                    GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_EDGE_COLUMN, val);
                else
                    SetEdgeColumn(val);
            }

            return true;
        }
        case ID_STE_PREF_TAB_WIDTH :
        {
            int val = wxGetNumberFromUser(_("Characters to expand tabs"),
                                          wxEmptyString, _("Set tab width"),
                                          GetTabWidth(),
                                          0, 255, GetModalParent());
            if (val >= 0)
            {
                if (GetEditorPrefs().IsOk())
                    GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_TAB_WIDTH, val);
                else
                    SetTabWidth(val);
            }

            return true;
        }
        case ID_STE_PREF_INDENT_WIDTH :
        {
            int val = wxGetNumberFromUser(_("Characters to indent"), wxEmptyString, _("Set indentation width"),
                                          GetIndent(),
                                          0, 255, GetModalParent());
            if (val >= 0)
            {
                if (GetEditorPrefs().IsOk())
                    GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_INDENT_WIDTH, val);
                else
                    SetIndent(val);
            }

            return true;
        }
        case ID_STE_PREF_EOL_MODE :
        {
            int eol_mode = GetEOLMode();
            int val = wxGetSingleChoiceIndex(wxString(_("Current EOL : "))+wxSTC_EOL_Strings[eol_mode],
                                             _("Select EOL mode"), wxSTC_EOL_Strings_count, wxSTC_EOL_Strings, GetModalParent());

            if ((val != -1) && (val != eol_mode))
            {
                if (GetEditorPrefs().IsOk())
                    GetEditorPrefs().SetPrefIntByID(ID_STE_PREF_EOL_MODE, val);
                else
                    SetEOLMode(val);
            }
            return true;
        }
        case ID_STE_SAVE_PREFERENCES :
        {
            if (wxConfigBase::Get(false))
                GetOptions().SaveConfig(*wxConfigBase::Get(false));

            return true;
        }

        default : break;
    }

    // check if wxSTEditorPref knows this id
    if ( GetEditorPrefs().IsOk() && (win_id >= ID_STE_PREF__FIRST) &&
                                    (win_id <= ID_STE_PREF__LAST))
    {
        GetEditorPrefs().SetPrefBoolByID(win_id, event.IsChecked());
        return true;
    }

    return false;
}

long wxSTEditor::UpdateCanDo(bool send_event)
{
    STE_INITRETURNVAL(0);

    long state_change = 0;

    if (HasState(STE_MODIFIED) != IsModified())
    {
        SetStateSingle(STE_MODIFIED, !HasState(STE_MODIFIED));
        state_change |= STE_MODIFIED;
    }
    if (HasState(STE_CANCUT) != CanCut())
    {
        SetStateSingle(STE_CANCUT, !HasState(STE_CANCUT));
        state_change |= STE_CANCUT;
    }
    if (HasState(STE_CANCOPY) != CanCopy())
    {
        SetStateSingle(STE_CANCOPY, !HasState(STE_CANCOPY));
        state_change |= STE_CANCOPY;
    }
    if (HasState(STE_CANPASTE) != CanPaste())
    {
        // In GTK you get 2 UpdateUI events per key press, the first CanPaste()
        // returns false since Editor::SelectionContainsProtected->RangeContainsProtected()
        // returns true which is a bug in scintilla I think.
        // This is why we override CanPaste() to simply return IsEditable() for GTK.
        // wxWidgets/src/stc/ScintillaWX.cpp - CanPaste() calls Editor::CanPaste() and checks for empty clipboard.
        // wxWidgets/src/stc/scintilla/src/Editor.cxx - bool Editor::CanPaste() does the SelectionContainsProtected code.
        //printf("Paste %d %d %d %d\n", (int)HasState(STE_CANPASTE), (int)CanPaste(), (int)GetReadOnly(), (int)IsEditable());

        SetStateSingle(STE_CANPASTE, !HasState(STE_CANPASTE));
        state_change |= STE_CANPASTE;
    }
    if (HasState(STE_CANUNDO) != CanUndo())
    {
        SetStateSingle(STE_CANUNDO, !HasState(STE_CANUNDO));
        state_change |= STE_CANUNDO;
    }
    if (HasState(STE_CANREDO) != CanRedo())
    {
        SetStateSingle(STE_CANREDO, !HasState(STE_CANREDO));
        state_change |= STE_CANREDO;
    }
    if (HasState(STE_CANSAVE) != CanSave())
    {
        SetStateSingle(STE_CANSAVE, !HasState(STE_CANSAVE));
        state_change |= STE_CANSAVE;
    }
    if (CanFind() != (GetFindReplaceData() && GetFindString().Length()))
    {
        // just reset it to unknown
        SetStateSingle(STE_CANFIND, GetFindReplaceData() && GetFindString().Length());
        state_change |= STE_CANFIND;
    }
    if (HasState(STE_EDITABLE) != IsEditable())
    {
        SetStateSingle(STE_EDITABLE, !HasState(STE_EDITABLE));
        state_change |= STE_EDITABLE;
    }

    if (send_event && (state_change != 0))
       SendEvent(wxEVT_STEDITOR_STATE_CHANGED, state_change, GetState(), GetFileName().GetFullPath());

    return state_change;
}

void wxSTEditor::OnSTEState(wxSTEditorEvent &event)
{
    STE_INITRETURN;
    event.Skip();

    wxMenu    *menu    = GetOptions().GetEditorPopupMenu();
    wxMenuBar *menuBar = GetOptions().GetMenuBar();
    wxToolBar *toolBar = GetOptions().GetToolBar();

    if (!menu && !menuBar && !toolBar)
        return;

    if (event.HasStateChange(STE_CANSAVE))
        STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_SAVE, event.GetStateValue(STE_CANSAVE));

    if (event.HasStateChange(STE_CANCUT))
        STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_CUT, event.GetStateValue(STE_CANCUT));
    if (event.HasStateChange(STE_CANCOPY))
    {
        STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_COPY,           event.GetStateValue(STE_CANCOPY));
        STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_COPY_PRIMARY, event.GetStateValue(STE_CANCOPY));
        STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_COPY_HTML,    event.GetStateValue(STE_CANCOPY));
    }
    if (event.HasStateChange(STE_CANPASTE))
    {
        STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_PASTE,        event.GetStateValue(STE_CANPASTE));
        STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_PASTE_NEW,  IsClipboardTextAvailable());
        STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_PASTE_RECT, event.GetStateValue(STE_CANPASTE));
    }
    if (event.HasStateChange(STE_CANUNDO))
        STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_UNDO, event.GetStateValue(STE_CANUNDO));
    if (event.HasStateChange(STE_CANREDO))
        STE_MM::DoEnableItem(menu, menuBar, toolBar, wxID_REDO, event.GetStateValue(STE_CANREDO));

    if (event.HasStateChange(STE_CANFIND))
    {
        STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_FIND_NEXT, event.GetStateValue(STE_CANFIND));
        STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STE_FIND_PREV, event.GetStateValue(STE_CANFIND));
        wxSTEUpdateSearchCtrl(toolBar, ID_STE_TOOLBAR_SEARCHCTRL, GetFindReplaceData());
    }

    // update everything for a big change like this
    if (event.HasStateChange(STE_EDITABLE))
        UpdateAllItems();
}

// --------------------------------------------------------------------------
// Get/SetLanguage

bool wxSTEditor::SetLanguage(int lang)
{
    if (!GetSTERefData()->SetLanguage(lang))
        return false;

    size_t n, editRefCount = GetSTERefData()->GetEditorCount();

    if (GetEditorStyles().IsOk())
    {
        for (n = 0; n < editRefCount; n++)
            GetEditorStyles().UpdateEditor(GetSTERefData()->GetEditor(n));
    }
    if (GetEditorPrefs().IsOk())
    {
        for (n = 0; n < editRefCount; n++)
            GetEditorPrefs().UpdateEditor(GetSTERefData()->GetEditor(n));
    }
    if (GetEditorLangs().IsOk())
    {
        for (n = 0; n < editRefCount; n++)
            GetEditorLangs().UpdateEditor(GetSTERefData()->GetEditor(n)); // -> Colourise()
    }
    else
    {
        ColouriseDocument();
    }
    return true;
}

bool wxSTEditor::SetLanguage(const wxFileName &filePath)
{
    int lang = STE_LANG_NULL;

    // use current langs or default if none
    if (GetEditorLangs().IsOk())
        lang = GetEditorLangs().FindLanguageByFilename(filePath);
    else
        lang = wxSTEditorLangs(true).FindLanguageByFilename(filePath);

    if (lang != STE_LANG_NULL)
        return SetLanguage(lang);

    return false;
}

int wxSTEditor::GetLanguageId() const
{
    return GetSTERefData()->m_steLang_id;
}

void wxSTEditor::SetFileEncoding(const wxString& encoding)
{
    GetSTERefData()->m_encoding = encoding;
}

wxString wxSTEditor::GetFileEncoding() const
{
    return GetSTERefData()->m_encoding;
}

void wxSTEditor::SetFileBOM(bool file_bom)
{
    GetSTERefData()->m_file_bom = file_bom;
}

bool wxSTEditor::GetFileBOM() const
{
    return GetSTERefData()->m_file_bom;
}

// --------------------------------------------------------------------------
// Get/Set Editor Prefs, Styles, Langs

const wxSTEditorPrefs& wxSTEditor::GetEditorPrefs() const
{
    return GetSTERefData()->m_stePrefs;
}
wxSTEditorPrefs& wxSTEditor::GetEditorPrefs()
{
    return GetSTERefData()->m_stePrefs;
}
void wxSTEditor::RegisterPrefs(const wxSTEditorPrefs& prefs)
{
    if (GetEditorPrefs().IsOk())
    {
        GetEditorPrefs().RemoveEditor(this);
        GetEditorPrefs().Destroy();
    }
    if (!prefs.IsOk())
        return;

    GetEditorPrefs().Create(prefs);
    GetEditorPrefs().RegisterEditor(this);
}

const wxSTEditorStyles& wxSTEditor::GetEditorStyles() const
{
    return GetSTERefData()->m_steStyles;
}
wxSTEditorStyles& wxSTEditor::GetEditorStyles()
{
    return GetSTERefData()->m_steStyles;
}
void wxSTEditor::RegisterStyles(const wxSTEditorStyles& styles)
{
    if (GetEditorStyles().IsOk())
    {
        GetEditorStyles().RemoveEditor(this);
        GetEditorStyles().Destroy();
    }
    if (!styles.IsOk())
        return;

    GetEditorStyles().Create(styles);
    GetEditorStyles().RegisterEditor(this);
}

const wxSTEditorLangs& wxSTEditor::GetEditorLangs() const
{
    return GetSTERefData()->m_steLangs;
}
wxSTEditorLangs& wxSTEditor::GetEditorLangs()
{
    return GetSTERefData()->m_steLangs;
}
void wxSTEditor::RegisterLangs(const wxSTEditorLangs& langs)
{
    if (GetEditorLangs().IsOk())
    {
        GetEditorLangs().RemoveEditor(this);
        GetEditorLangs().Destroy();
    }
    if (!langs.IsOk())
        return;

    GetEditorLangs().Create(langs);
    GetEditorLangs().RegisterEditor(this);
}

// --------------------------------------------------------------------------
// Event handling

bool wxSTEditor::SendEvent(wxEventType eventType, int evt_int, long extra_long,
                           const wxString &evtStr, bool do_post )
{
    STE_INITRETURNVAL(false);

    //wxPrintf(wxT("Send event %d, %d %ld '%s'\n"), long(eventType), evt_int, extra_long, evtStr.wx_str());

    if ((eventType == wxEVT_STEDITOR_STATE_CHANGED) ||
        (eventType == wxEVT_STEDITOR_SET_FOCUS) ||
        (eventType == wxEVT_STESHELL_ENTER))
    {
        wxSTEditorEvent event(GetId(), eventType, this,
                              evt_int, extra_long, evtStr);

        if ( do_post )
        {
            GetEventHandler()->AddPendingEvent(event);
            return false;
        }

        return GetEventHandler()->ProcessEvent(event);
    }

    wxCommandEvent event(eventType, GetId());
    event.SetInt(evt_int);
    event.SetExtraLong(extra_long);
    event.SetString(evtStr);
    event.SetEventObject(this);

    if ( do_post )
    {
        GetEventHandler()->AddPendingEvent(event);
        return false;
    }

   return GetEventHandler()->ProcessEvent(event);
}

void wxSTEditor::OnKeyDown(wxKeyEvent& event)
{
    //wxPrintf(wxT("Char %d %d %d\n"), int('c'), event.GetKeyCode(), event.AltDown());

    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
        {
            if (HasSelection())
                RemoveSelection();

            event.Skip(); // allow other default behavior too

            break;
        }
        case WXK_INSERT : // already have ID_STE_SELECT_RECT for Alt-Shift-P
        {
            if (event.AltDown() && event.ShiftDown())
                PasteRectangular();
            else
                event.Skip();

            break;
        }
        default:
        {
            event.Skip();
            break;
        }
    }
}

void wxSTEditor::OnKeyUp(wxKeyEvent& event)
{
    event.Skip();
}

#ifdef skip
static void wxSTEditor_SplitLines(wxSTEditor* editor, int pos, int line_n) // FIXME verify wrapping code
{
    int line_len = editor->LineLength(line_n);
    int edge_col = editor->GetEdgeColumn();

    //wxPrintf(wxT("wxSTEditor_SplitLines\n"));

    if (line_len > edge_col)
    {
        wxString s(editor->GetLineText(line_n));
        size_t n, len = s.Length();

        //wxPrintf(wxT("%d %d '%s'\n"), line_n, len, s.wx_str());

        for (n = edge_col-1; n >= 0; n--)
        {
            if ((s[n] == wxT(' ')) || (s[n] == wxT('\t')))
            {
                wxString keep(s.Mid(0, n));
                wxString split(s.Mid(n+1, len-1));
                //wxPrintf(wxT("'%s' '%s'\n"), keep.wx_str(), split.wx_str());
                editor->SetLineText(line_n, keep);
                editor->MarkerAdd(line_n, STE_MARKER_BOOKMARK);

                int line_count = editor->GetLineCount();
                wxString next;
                int marker = 0;

                if (line_n + 1 < line_count)
                {
                    marker = editor->MarkerGet(line_n+1);

                    if (marker & (1<<STE_MARKER_BOOKMARK))
                    {
                        next = editor->GetLineText(line_n+1);
                    }
                    else
                    {
                        editor->InsertText(editor->PositionFromLine(line_n+1), wxT("\n"));
                    }
                }

                editor->SetLineText(line_n+1, split+next);
                editor->MarkerAdd(line_n+1, STE_MARKER_BOOKMARK);
                wxSTEditor_SplitLines(editor, pos, line_n+1);

                break;
            }
        }
    }
}
#endif

void wxSTEditor::OnSTCCharAdded(wxStyledTextEvent &event)
{
    //ResetLastAutoIndentLine(); // FIXME - make this a preference
    event.Skip();

    const wxChar c = event.GetKey();

    //wxPrintf(wxT("Char added %d '%c'\n"), int(c), c);

    // Change this if support for mac files with \r if needed
    if ((c == wxT('\n')) && GetEditorPrefs().IsOk() &&
         GetEditorPrefs().GetPrefBool(STE_PREF_AUTOINDENT))
    {
        const int line = GetCurrentLine();
        const int indent = line < 1 ? 0 : GetLineIndentation(line - 1);

        if (indent != 0)
        {
            // store previous line settings
            GetSTERefData()->m_last_autoindent_line = line;
            GetSTERefData()->m_last_autoindent_len  = GetLineLength(line);

            SetLineIndentation(line, indent);
            GotoPos(GetLineIndentPosition(line));
        }
    }
#ifdef skip
    else if (0)
    {
        STE_TextPos pos = GetCurrentPos();
        int line_n = LineFromPosition(pos);
        wxSTEditor_SplitLines(this, pos, line_n);
        GotoPos(pos);
    }
#endif
}

void wxSTEditor::OnSTCUpdateUI(wxStyledTextEvent &event)
{
    STE_INITRETURN;

    event.Skip(true);

    if (GetEditorPrefs().IsOk())
    {
        if (GetEditorPrefs().GetPrefBool(STE_PREF_HIGHLIGHT_BRACES))
            DoBraceMatch();
    }

    // todo add pref for this
    if (0)
    {
        STE_TextPos start_pos = 0, end_pos = 0;
        GetSelection(&start_pos, &end_pos);

        if ((start_pos + 3   < end_pos) &&
            (start_pos + 40  > end_pos) &&
            TextRangeIsWord(start_pos, end_pos))
        {
            wxString text(GetTextRange(start_pos, end_pos));

            if (GetSTERefData()->m_hilighted_word != text)
            {
                if (!GetSTERefData()->m_hilighted_word.IsEmpty())
                {
                    wxArrayInt& startPositions = GetSTERefData()->m_hilightedArray;
                    int n, count = (int)startPositions.GetCount();
                    for (n = 0; n < count; ++n)
                        ClearIndication(startPositions[n], wxSTC_INDIC1_MASK);

                    startPositions.Clear();
                }

                GetSTERefData()->m_hilighted_word = text;

                IndicateAllStrings(text, STE_FR_WHOLEWORD|STE_FR_MATCHCASE, wxSTC_INDIC1_MASK,
                                   &GetSTERefData()->m_hilightedArray);
                //Refresh(false);
            }
        }
        else if (!GetSTERefData()->m_hilighted_word.IsEmpty())
        {
            GetSTERefData()->m_hilighted_word.Clear();
            wxArrayInt& startPositions = GetSTERefData()->m_hilightedArray;
            int n, count = (int)startPositions.GetCount();
            for (n = 0; n < count; ++n)
                ClearIndication(startPositions[n], wxSTC_INDIC1_MASK);

            startPositions.Clear();
            //Refresh(false);
        }
    }

    UpdateCanDo(true);
}

void wxSTEditor::OnMouseWheel(wxMouseEvent& event)
{
    // FIXME this should be fixed in GTK soon and can be removed in wx > 2.4.2
    //       harmless otherwise.
    if (event.m_linesPerAction == 0)
        event.m_linesPerAction = 3;

    event.Skip();
}

void wxSTEditor::OnScroll( wxScrollEvent& event )
{
    // this event is from user set wxScrollBars
    event.Skip();
    if (event.GetOrientation() == wxVERTICAL) return;

    wxScrollBar* sb = wxStaticCast(event.GetEventObject(), wxScrollBar);
    int pos   = event.GetPosition();
    int thumb = sb->GetThumbSize();
    //int range = sb->GetRange();
    int scroll_width = GetScrollWidth();

    // only check if at scroll end, wxEVT_SCROLL(WIN)_BOTTOM is not reliable
    if (pos + thumb >= scroll_width)
    {
        int longest_len = GetLongestLinePixelWidth();
        if (longest_len > scroll_width)
            SetScrollWidth(longest_len);
            //sb->SetScrollbar(pos, thumb, text_width, true);

        // You have to release the scrollbar before the change takes effect.
        sb->Refresh();
    }
}
void wxSTEditor::OnScrollWin( wxScrollWinEvent& event )
{
    // this event is from this window's builtin scrollbars
    event.Skip();
    if (event.GetOrientation() == wxVERTICAL) return;

    int pos   = event.GetPosition(); //GetScrollPos(wxHORIZONTAL);
    int thumb = GetScrollThumb(wxHORIZONTAL);
    //int range = GetScrollRange(wxHORIZONTAL);
    int scroll_width = GetScrollWidth();

    // only check if at scroll end, wxEVT_SCROLL(WIN)_BOTTOM is not reliable
    if (pos + thumb >= scroll_width)
    {
        int longest_len = GetLongestLinePixelWidth();
        if (longest_len > scroll_width)
            SetScrollWidth(longest_len);
            //SetScrollbar(wxHORIZONTAL, pos, thumb, text_width, true);
    }
}

void wxSTEditor::OnContextMenu(wxContextMenuEvent& event)
{
    wxMenu* popupMenu = GetOptions().GetEditorPopupMenu();
    if (popupMenu)
    {
        UpdateItems(popupMenu);
        if (!SendEvent(wxEVT_STEDITOR_POPUPMENU, 0, GetState(), GetFileName().GetFullPath()))
        {
            PopupMenu(popupMenu);
        }
    }
    else
        event.Skip();
}

void wxSTEditor::OnMenu(wxCommandEvent& event)
{
    wxSTERecursionGuard guard(m_rGuard_OnMenu);
    if (guard.IsInside()) return;

    if (!HandleMenuEvent(event))
        event.Skip();
}

void wxSTEditor::OnSTCMarginClick(wxStyledTextEvent &event)
{
    const int double_click_millis = 600; // time between clicks for double click
    const int line   = LineFromPosition(event.GetPosition());
    const int margin = event.GetMargin();

    wxLongLong t = wxGetLocalTimeMillis();

    wxLongLong last_time   = m_marginDClickTime;
    int        last_line   = m_marginDClickLine;
    int        last_margin = m_marginDClickMargin;

    m_marginDClickTime   = t;
    m_marginDClickLine   = line;
    m_marginDClickMargin = margin;

    if ((t < last_time + double_click_millis) &&
        (line == last_line) && (margin == last_margin))
    {
        wxStyledTextEvent dClickEvent(event);
        dClickEvent.SetEventType(wxEVT_STEDITOR_MARGINDCLICK);
        dClickEvent.SetEventObject(this);
        dClickEvent.SetLine(line);
        dClickEvent.SetMargin(margin);
        dClickEvent.SetPosition(event.GetPosition());
        // let dclick override this event if not skipped
        if (GetEventHandler()->ProcessEvent( dClickEvent ))
            return;
    }

    // let others process this first
    if ( GetParent()->GetEventHandler()->ProcessEvent( event ) )
        return;

    if (margin == STE_MARGIN_FOLD)
    {
        const int level = GetFoldLevel(line);
        if (STE_HASBIT(level, wxSTC_FOLDLEVELHEADERFLAG))
            ToggleFold(line);
    }
    else
        event.Skip();
}

void wxSTEditor::OnSTCMarginDClick(wxStyledTextEvent &event)
{
    // let others process this first
    if ( GetParent()->GetEventHandler()->ProcessEvent( event ) )
        return;

    const int line = event.GetLine();

    if ((event.GetMargin() == STE_MARGIN_MARKER) &&
        GetEditorPrefs().IsOk() &&
        GetEditorPrefs().GetPrefBoolByID(ID_STE_PREF_BOOKMARK_DCLICK))
    {
        // See similiar code for ID_STE_BOOKMARK_TOGGLE
        if ((MarkerGet(line) & (1<<STE_MARKER_BOOKMARK)) != 0)
            MarkerDelete(line, STE_MARKER_BOOKMARK);
        else
            MarkerAdd(line, STE_MARKER_BOOKMARK);
    }
    else
        event.Skip();
}

void wxSTEditor::OnSetFocus(wxFocusEvent &event)
{
    event.Skip();
    STE_INITRETURN;
    // in GTK2 you can get a focus event when not shown
    if (!IsShown())
        return;

    // check to make sure that the parent is not being deleted
    for (wxWindow* parent = GetParent(); parent; parent = parent->GetParent())
    {
        if (parent->IsBeingDeleted())
        {
            SetSendSTEEvents(false);
            return;
        }
    }

    SendEvent(wxEVT_STEDITOR_SET_FOCUS, 0, GetState(), GetFileName().GetFullPath());
}
void wxSTEditor::OnSTEFocus(wxSTEditorEvent &event)
{
    STE_INITRETURN;
    if (m_activating)
        return;

    event.Skip();

    UpdateCanDo(false); // no events since nothing happened
    UpdateAllItems();

    // block recursive isaltered, show/close dialog, focus event, isaltered...
    m_activating = true;
    IsAlteredOnDisk(true);
    m_activating = false;
}

// --------------------------------------------------------------------------

wxSTETreeItemData* wxSTEditor::GetTreeItemData() const
{
    return GetSTERefData()->m_treeItemData;
}

void wxSTEditor::SetTreeItemData(wxSTETreeItemData* data)
{
    GetSTERefData()->m_treeItemData = data;
}

// --------------------------------------------------------------------------

wxClientData *wxSTEditor::GetClientObject() const
{
    return GetSTERefData()->GetClientObject();
}

void wxSTEditor::SetClientObject( wxClientData *data )
{
    GetSTERefData()->SetClientObject(data);
}

// --------------------------------------------------------------------------


// This is in scintilla/src/Editor.cxx
// const char *ControlCharacterString(unsigned char ch);

// "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
// "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",
// "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
// "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US"

static const int ste_ctrlCharLenghts_size = 32;
static const int ste_ctrlCharLengths[ste_ctrlCharLenghts_size] =
                                           { 3, 3, 3, 3, 3, 3, 3, 3,
                                             2, 2, 2, 2, 2, 2, 2, 2,
                                             3, 3, 3, 3, 3, 3, 3, 3,
                                             3, 2, 3, 3, 2, 2, 2, 2 };

int wxSTEditor::GetLongestLinePixelWidth(int top_line, int bottom_line)
{
    int longest_len   = 0;
    int first_line    = top_line < 0 ? GetFirstVisibleLine() : top_line;
    int line_count    = GetLineCount();
    int lines_visible = LinesOnScreen();
    int last_line     = bottom_line < 0 ? wxMin(line_count, first_line + lines_visible) : bottom_line;
    int tab_width     = GetTabWidth();
    int ctrl_char_symbol = GetControlCharSymbol();

    if (last_line < first_line)
    {
        int tmp = first_line; first_line = last_line; last_line = tmp;
    }

    // FIXME this is not the best solution, but with some luck it'll work
    //       Scintilla should provide this info from its LayoutCache
    for (int n = first_line; n <= last_line; n++)
    {
        int len = LineLength(n);

        int tabs = 0;
        if ((tab_width > 1) && (len*tab_width > longest_len))
        {
            // need to sum up only how much of the tab is used
            wxCharBuffer buf = GetLineRaw(n);
            const char* c = buf.data();
            for (int i = 0; i < len; i++, c++)
            {
                if (*c == '\t')
                    tabs += tab_width - ((i + tabs) % tab_width);
                else if ((ctrl_char_symbol >= ste_ctrlCharLenghts_size) &&
                         (int(*c) < ste_ctrlCharLenghts_size))
                {
                    // scintilla writes name of char
                    tabs += ste_ctrlCharLengths[size_t(*c)] - 1;
                }
            }

            //wxPrintf(wxT("line %d len %d pos %d\n"), n, len, len+tabs); fflush(stdout);
        }
        len += tabs + 3; // add a little extra if showing line endings
        if (longest_len < len) longest_len = len;
    }
    return TextWidth(wxSTC_STYLE_DEFAULT, wxString(longest_len, wxT('D')));
}

bool wxSTEditor::ResetLastAutoIndentLine()
{
    int last_autoindent_line = GetSTERefData()->m_last_autoindent_line;
    int last_autoindent_len  = GetSTERefData()->m_last_autoindent_len;

    if (last_autoindent_line < 0)
        return false;
    if (last_autoindent_line > GetLineCount())
    {
        GetSTERefData()->m_last_autoindent_line = -1;
        return false;
    }

    // we're still on the same line
    if (last_autoindent_line == LineFromPosition(GetCurrentPos()))
        return false;

    const int line_len = GetLineLength(last_autoindent_line);
    if (line_len < last_autoindent_len)
    {
        GetSTERefData()->m_last_autoindent_line = -1;
        return false;
    }

    wxString lineString(GetLine(last_autoindent_line));
    if (lineString.Mid(last_autoindent_len).Strip(wxString::both).IsEmpty())
    {
        int line_start = PositionFromLine(last_autoindent_line);
        SetTargetStart(line_start + last_autoindent_len);
        SetTargetEnd(line_start + line_len);
        ReplaceTarget(wxEmptyString);

        GetSTERefData()->m_last_autoindent_line = -1;
        return true;
    }

    return false;
}
