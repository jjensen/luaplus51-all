///////////////////////////////////////////////////////////////////////////////
// Name:        stedit.h
// Purpose:     wxSTEditor
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file stedit.h
/// @brief wxSTEditor, a wxStyledTextCtrl derived class and the wxSTEditorEvent class.

#ifndef _STEDIT_H_
#define _STEDIT_H_

#include <wx/defs.h>
#include <wx/datetime.h>
#include <wx/fdrepdlg.h>
#include <wx/filename.h>
#include <wx/treectrl.h>
#include <wx/textbuf.h> // for wxTextFileType

class WXDLLIMPEXP_FWD_CORE wxMenu;
class WXDLLIMPEXP_FWD_CORE wxKeyEvent;
class WXDLLIMPEXP_FWD_CORE wxFindDialogEvent;
class WXDLLIMPEXP_FWD_CORE wxToolBar;

#include "wx/stedit/stedefs.h"
#include "wx/stedit/steprefs.h"
#include "wx/stedit/stestyls.h"
#include "wx/stedit/stelangs.h"
#include "wx/stedit/steopts.h"

class WXDLLIMPEXP_FWD_STEDIT wxSTEditorFindReplaceDialog;
class WXDLLIMPEXP_FWD_STEDIT wxSTETreeItemData;

//-----------------------------------------------------------------------------
/// @class wxSTEPointerLocker
/// @brief A simple pointer locker for objects to share pointers to data and
///        be able to clear the pointer for all copies.
/// Create a wxSTEPointerLocker as a member of a class (or whereever) and pass
/// it a pointer to keep track of. Other objects can create wxSTEPointerLockers
/// and ref the locker (as you would a wxPen/wxBitmap) and can access the
/// shared pointer, BUT only after checking if it's valid by calling IsOk().
/// This can be used as an alternative to tracking wxWindows by connecting to
///   wxEVT_DESTROY for each of them.
/// This class is NOT threadsafe for simplicity.
/// In wxWidgets 2.9 there are MUCH better classes, wxTrackable and wxWeakRef,
/// and this exists in a basic form for compatibility with wx2.8.
//-----------------------------------------------------------------------------

template <class T>
class WXDLLIMPEXP_STEDIT wxSTEPointerLockerRefData : public wxObjectRefData
{
public:
    wxSTEPointerLockerRefData(T* ptr = NULL, bool is_static = false) : m_pointer(ptr), m_is_static(is_static) {}

    virtual ~wxSTEPointerLockerRefData()
    {
        if (!m_is_static)
            delete m_pointer;
    }

    T*   m_pointer;
    bool m_is_static;
};

template <class T>
class WXDLLIMPEXP_STEDIT wxSTEPointerLocker : public wxObject
{
public:
    /// Point to an object for objects to share.
    /// Note that any one of the wxSTEPointerLocker refs may clear the pointer.
    /// If is_static is false then take ownership of the data and call delete
    /// on it when there are no longer any references to it.
    wxSTEPointerLocker(T* ptr, bool is_static)
    {
        m_refData = new wxSTEPointerLockerRefData<T>(ptr, is_static);
    }

    wxSTEPointerLocker(const wxSTEPointerLocker& locker) { Ref(locker); }

    virtual ~wxSTEPointerLocker() { }

    /// Returns true if the pointer is valid.
    bool IsOk() const { return (get() != NULL); }

    /// Get the pointer, which may be NULL.
    T*       get()       { return m_refData ? dynamic_cast<wxSTEPointerLockerRefData<T> >(m_refData)->m_pointer : NULL; }
    /// Get the pointer, which may be NULL.
    const T* get() const { return m_refData ? dynamic_cast<wxSTEPointerLockerRefData<T> >(m_refData)->m_pointer : NULL; }

    /// Set the pointer, clearing the reference to the previous one.
    void     set(T* ptr, bool is_static) { UnRef(); m_refData = new wxSTEPointerLockerRefData<T>(ptr, is_static); }

    // ------------------------------------------------------------------------
    /// @name std/boost::shared_ptr compatibility functions.
    /// @{

    void   reset()           { set(NULL, false); }
    size_t use_count() const { return m_refData ? m_refData->GetRefCount() : 0; }
    bool   unique()    const { return use_count() == 1; }
    bool   expired()   const { return !IsOk(); }

    /// @}
    // ------------------------------------------------------------------------
    /// @name Operators
    /// @{
    wxSTEPointerLocker& operator = (const wxSTEPointerLocker& locker)
    {
        if ( (*this) != locker )
            Ref(locker);
        return *this;
    }

    bool operator == (const wxSTEPointerLocker& locker) const
        { return m_refData == locker.m_refData; }
    bool operator != (const wxSTEPointerLocker& locker) const
        { return m_refData != locker.m_refData; }

    operator bool() const { return IsOk(); }
    /// @}
};

//-----------------------------------------------------------------------------
/// @class wxSTERecursionGuard
/// @brief A simple recursion guard to block reentrant functions.
/// Create a wxSTERecursionGuardFlag as a member of a class (or whereever) and
/// in functions that may recurse, create a wxSTERecursionGuard and handle
/// recursion appropriately by calling IsInside().
/// wxStEdit has its own version since it's used as a class member and wxWidgets
/// may change their implementation.
//-----------------------------------------------------------------------------

/// @class wxSTERecursionGuardFlag
/// @brief Create one of these classes to maintain the recursion flag state.
class WXDLLIMPEXP_STEDIT wxSTERecursionGuardFlag
{
public:
    wxSTERecursionGuardFlag() : m_flag(0) {}
    int m_flag;
};

class WXDLLIMPEXP_STEDIT wxSTERecursionGuard
{
public:
    /// Attach to a wxSTERecursionGuardFlag whose lifetime MUST be greater
    /// than the life of this class instance.
    wxSTERecursionGuard(wxSTERecursionGuardFlag& flag) : m_flag(flag)
    {
        m_isInside = (flag.m_flag++ != 0);
    }

    ~wxSTERecursionGuard()
    {
        wxASSERT_MSG(m_flag.m_flag > 0, wxT("unbalanced wxSTERecursionGuards!?"));
        m_flag.m_flag--;
    }

    bool IsInside() const { return m_isInside; }

private:
    wxSTERecursionGuardFlag& m_flag;
    bool m_isInside;     // true if m_flag had been already set when created
};

//-----------------------------------------------------------------------------
/// @enum STE_FindStringType options of what to do when finding strings.
/// @see wxSTEditor::FindString()
//-----------------------------------------------------------------------------

enum STE_FindStringType
{
    STE_FINDSTRING_NOTHING = 0,     ///< Do nothing when finding a string.

    STE_FINDSTRING_SELECT  = 0x01,  ///< Select the string if found.
    STE_FINDSTRING_GOTO    = 0x02   ///< Goto the start pos of string.
};

//-----------------------------------------------------------------------------
/// @enum STE_TranslatePosType options of what to do for wxSTEditor::TranslateXXX().
//-----------------------------------------------------------------------------

enum STE_TranslatePosType
{
    STE_TRANSLATE_NOTHING   = 0, ///< Take input literally.
    STE_TRANSLATE_SELECTION = 1, ///< Use selection to translate.
    STE_TRANSLATE_TARGET    = 2  ///< Use target to translate.
};

//-----------------------------------------------------------------------------
/// @enum STE_ClipboardType options of how to get/set data from/to the clipboard.
//-----------------------------------------------------------------------------

enum STE_ClipboardType
{
    STE_CLIPBOARD_DEFAULT = 1, ///< Use the normal clipboard.
    STE_CLIPBOARD_PRIMARY = 2, ///< Use the primary clipboard (Unix systems only).
    STE_CLIPBOARD_BOTH    = 3  ///< Use both clipboards (only valid for Set functions).
};

//-----------------------------------------------------------------------------
/// @class wxSTEditorRefData
/// @brief A wxObject ref counted data to share with refed (split) wxSTEditor instances.
///
/// You normally should not need to access any of this, use the member functions
/// in the wxSTEditor. Triple check that you cannot do what you want through
/// the wxSTEditor before accessing members of this class.
///
/// Instead of trying to derive from this class to add members, please create
/// your own wxClientData object with the extra data you need and use the
/// wxSTEditor::Get/SetClientData() functions. The wxClientData is deleted by
/// this class, so if you need it to be persistent make your data a shared pointer
/// member of the wxClientData container you set.
//-----------------------------------------------------------------------------
class WXDLLIMPEXP_STEDIT wxSTEditorRefData : public wxObjectRefData, public wxClientDataContainer
{
public:
    wxSTEditorRefData();
    virtual ~wxSTEditorRefData();

    // Find/Add/Remove editors that share this data
    size_t GetEditorCount() const            { return m_editors.GetCount(); }
    bool HasEditor(wxSTEditor* editor) const { return FindEditor(editor) != wxNOT_FOUND; }
    int FindEditor(wxSTEditor* editor) const { return m_editors.Index(editor); }
    wxSTEditor *GetEditor(size_t n) const    { return (wxSTEditor*)m_editors.Item(n); }
    void AddEditor(wxSTEditor* editor)       { if (!HasEditor(editor)) m_editors.Add(editor); }
    void RemoveEditor(wxSTEditor* editor)    { int n = FindEditor(editor); if (n != wxNOT_FOUND) m_editors.RemoveAt(n); }

    bool SetLanguage(int lang)
    {
        wxCHECK_MSG(lang >= 0, false, wxT("Invalid language ID"));
        m_steLang_id = lang;
        return true;
    }
    bool SetLanguage(const wxFileName& fileName);

protected:
    wxFileName   m_fileName;        // current filename for the editor
    wxDateTime   m_modifiedTime;    // file modification time, else invalid
    wxString     m_encoding;        // encoding specified by LoadFile parameter, or else, found inside file
    bool         m_file_bom;        // bom found inside file

    int          m_steLang_id;      // index into the wxSTEditorLangs used

    wxSTETreeItemData* m_treeItemData; // the data if tracked in a wxTreeCtrl

    int     m_last_autoindent_line; // last line that was auto indented
    int     m_last_autoindent_len;  // the length of the line before auto indenting

    long    m_state;                // what state does this editor have, enum STE_StateType
    bool    m_dirty_flag;           // set if file format is changed by the user, in the properties dialog
                                    // There is no opposite of SCI_SETSAVEPOINT
    wxString   m_hilighted_word;    // The last selected word that has been indicated.
    wxArrayInt m_hilightedArray;    // Start pos of each hilighted word

    wxSTEditorOptions m_options;    // options, always created

    // we have our own copy of prefs/styles/langs in addition to those in
    // the options so we can detach an editor, but use the rest of the options
    // they're ref counted so they're small
    wxSTEditorPrefs  m_stePrefs;
    wxSTEditorStyles m_steStyles;
    wxSTEditorLangs  m_steLangs;

private:
    wxArrayPtrVoid m_editors;       // editors that share this data

    friend class wxSTEditor;
    friend class wxSTETreeItemData;
};

//-----------------------------------------------------------------------------
/// @class wxSTEditor
/// @brief A wxStyledTextCtrl derived editor control with many added features.
///
/// This control can be created and used as a wxStyledTextCtrl editor, but it
/// is best created by the wxSTEditorSplitter. Be sure to call CreateOptions(...)
/// if you create the editor by itself, however this is not necessary, but
/// quite a bit of the added functionally will be missing.
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_STEDIT wxSTEditor : public wxStyledTextCtrl
{
public :

    wxSTEditor() : wxStyledTextCtrl() { Init(); }

    wxSTEditor(wxWindow *parent, wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0, // wxStyledTextCtrl ors this with defaults
               const wxString& name = wxT("wxSTEditor"));

    virtual ~wxSTEditor();
    virtual bool Destroy();

    bool Create( wxWindow *parent, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = 0, // wxStyledTextCtrl ors this with defaults
                 const wxString& name = wxT("wxSTEditor"));

    /// Clone this editor, uses wxClassInfo so derived classes should work without
    /// having to override this.
    /// Override if you need to create your own type for the wxSTEditorSplitter to use in
    ///   wxSTEditorSplitter::CreateEditor().
    /// Be sure to use DECLARE_DYNAMIC_CLASS() and IMPLEMENT_DYNAMIC_CLASS()
    /// in your derived editor.
    virtual wxSTEditor* Clone(wxWindow *parent, wxWindowID id = wxID_ANY,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = 0, // wxStyledTextCtrl ors this with defaults
                              const wxString& name = wxT("wxSTEditor")) const;

    wxWindow* GetModalParent() { return this; } // TODO : remove this

    /// If this editor is going to use a Refed document, run this after construction.
    /// A refed editor will mirror the original wxSTEditor, the input origEditor isn't modified.
    ///  @see wxSTEditorSplitter for usage, be careful to get it right.
    virtual void RefEditor(wxSTEditor *origEditor);

    /// Create all the elements set in the wxSTEditorOptions.
    /// This function is always called after creation by the parent wxSTEditorSplitter
    /// with its options and would be a good function to override to setup
    /// the editor your own way.
    /// This registers the prefs, styles, langs in the options,
    /// and creates any enum STE_EditorOptionsType items set in the options.
    virtual void CreateOptions(const wxSTEditorOptions& options);
    /// GetOptions, use this to get editor option values.
    const wxSTEditorOptions& GetOptions() const;
    /// GetOptions, use this to get/change editor option values.
    wxSTEditorOptions& GetOptions();
    /// Set the options, the options will now be refed copies of the ones you send in.
    /// This can be used to detach the options for a particular editor from
    /// the rest of them. This function doesn't do anything other than
    /// set the new options, nothing will be created or destroyed.
    /// @see CreateOptions(...) to set and create items.
    void SetOptions(const wxSTEditorOptions& options);

    // ************************************************************************
    /// Enable or disable sending or handling events, for use during initialization or destruction.
    /// NOTE: This might not be necessary anymore since overriding Destroy()
    ///
    /// IMPORTANT! In your wxApp::OnExit or wxFrame's EVT_CLOSE handler
    /// make sure you call this to ensure that any extraneous focus events
    /// are blocked. GTK 2.0 for example sends them, MSW has been known to do it
    /// as well (but that was for a different control).
    ///
    /// The problem occurs because focus events may be sent to the window if the
    /// user closes it and immediately clicks on it before it's destroyed. This
    /// is not typical, but can happen suprisingly easily.
    /// The sequence of events is as follows, the focus event is created
    /// from wxWidgets and the ste editor tries to update the menus and toolbars.
    /// In GTK2, for example, the event loop is run when updating a
    /// toolbar tool and so the ste editor can be destroyed before the toolbar
    /// finishes updating. When the function returns the program crashes.
    void SetSendSTEEvents(bool send) { m_sendEvents = send; }
    // ************************************************************************

    // ------------------------------------------------------------------------
    /// @name wxTextCtrl methods and a few simple convenience functions.
    /// This editor can be used as a replacement wxTextCtrl with very few, if any, code changes.
    /// Many functions are simply const versions that were not in wxWidgets 2.8.
    /// @{

#if (wxVERSION_NUMBER < 2900) // in wx2.9 wxSTC derived from wxTextCtrlIface
    bool CanCopy() const                    { return HasSelection(); }
    bool CanCut()  const                    { return CanCopy() && IsEditable(); }
    void SetInsertionPoint(STE_TextPos pos) { GotoPos(pos); }
    void SetInsertionPointEnd()             { GotoPos(GetLength()); }
    void WriteText(const wxString &text)    { InsertText(GetCurrentPos(), text); SetCurrentPos(GetCurrentPos() + (STE_TextPos)text.Len()); }
    STE_TextPos XYToPosition(long x, long y) const { return x + wxConstCast(this, wxSTEditor)->PositionFromLine(y); }
    // Remove this section of text between markers
    void Remove(int iStart, int iEnd)       { SetTargetStart(iStart); SetTargetEnd(iEnd); ReplaceTarget(wxEmptyString); }
    // Get the row/col representation of the position
    bool PositionToXY(STE_TextPos, long *x, long *y) const;
    bool HasSelection() const               { return (GetSelectionStart() != GetSelectionEnd()); } // some wxTextCtrl implementations have this
    void RemoveSelection()                  { SetSelection(GetCurrentPos() , GetCurrentPos()); }   // some wxTextCtrl implementations have this
    void ShowPosition(STE_TextPos pos)      { GotoPos(pos); }
    void SetValue(const wxString& text)     { SetText(text); }
    void ChangeValue(const wxString& text)  { SetText(text); }
    wxString GetValue() const               { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetText(); }
    wxString GetText() const                { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetText(); }
    void ClearSelections()                  { SetSelection(GetInsertionPoint() , GetInsertionPoint()); }


    // verbatim copy of wx trunk wxTextAreaBase::SetModified()
    void SetModified(bool modified) { if ( modified ) MarkDirty(); else DiscardEdits(); }
#endif // (wxVERSION_NUMBER < 2900)

    wxString GetLineText(int line) const; ///< excluding any cr/lf at end.
    int GetLineLength(int iLine) const;   ///< excluding any cr/lf at end.

    virtual void SetEditable(bool editable);

    void SetReadOnly(bool readOnly) { SetEditable(!readOnly); } // overload to use our overridden implementation
    bool GetReadOnly() const        { return !IsEditable();   } // overload to use overridden implementation in a derived class

    // ------------------------------------------------------------------------
    // wxWidgets 2.8 <--> 2.9 compatibility functions (most functions are const in wx2.9)

    wxString GetTextRange(int startPos, int endPos) const { return  wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetTextRange(startPos, endPos); }

#if (wxVERSION_NUMBER >= 2900)

    /// changed from bool to int in wx trunk, returns enum IndentView.ivNone = 0.
    bool GetIndentationGuides() const { return 0 != wxStyledTextCtrl::GetIndentationGuides(); }

#else // (wxVERSION_NUMBER < 2900)

    int GetEOLMode() const                { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetEOLMode(); }
    STE_TextPos GetInsertionPoint() const { return wxConstCast(this, wxSTEditor)->GetCurrentPos(); } // not in wx2.8

    wxString GetLine(int line) const { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetLine(line); }
    int GetLength() const            { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetLength(); }
    int GetTextLength() const        { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetTextLength(); }
    long GetNumberOfLines() const    { return wxConstCast(this, wxSTEditor)->GetLineCount(); } // not in wx2.8
    wxString GetRange(int startPos, int endPos) const { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetTextRange(startPos, endPos); }

    void GetSelection(STE_TextPos* iStart, STE_TextPos* iEnd) const // forwards compatibility, wx2.8 uses int*
        { int s=0,e=0; wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetSelection(&s, &e); if (iStart) *iStart=s; if (iEnd) *iEnd=e; }
    void GetSelection(int *iStart, int *iEnd) const
        { int s=0,e=0; wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetSelection(&s, &e); if (iStart) *iStart=s; if (iEnd) *iEnd=e; }

    STE_TextPos GetSelectionStart() const { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetSelectionStart(); }
    STE_TextPos GetSelectionEnd() const   { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetSelectionEnd(); }

    STE_TextPos GetTargetStart() const    { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetTargetStart(); }
    STE_TextPos GetTargetEnd() const      { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetTargetEnd(); }

    virtual bool IsEditable() const       { return !wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::GetReadOnly(); } // not in wx2.8; virtual in wx trunk so make it virtual here too

    int LineFromPosition(STE_TextPos pos) const  { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::LineFromPosition(pos); }
    STE_TextPos PositionFromLine(int line) const { return wxConstCast(this, wxSTEditor)->wxStyledTextCtrl::PositionFromLine(line); }

    STE_TextPos GetLastPosition() const { return GetTextLength(); }
    bool IsEmpty() const                { return GetLastPosition() <= 0; }
#endif // (wxVERSION_NUMBER >= 2900)

#ifdef __WXGTK__
    /// Override CanPaste() to return !readonly in GTK.
    /// NOTE: GTK runs the evt loop during the wxSTC::CanPaste() check which can cause a crash in scintilla's
    /// drawing code in certain circumstances, for now let's just assume you can always paste if not readonly.
    /// The worst that can happen is that empty text is retrieved from the clipboard and
    /// nothing is pasted which is harmless. In any case, we want this check to be fast.
    bool CanPaste()       { return IsEditable(); }
    bool CanPaste() const { return IsEditable(); }
#endif // __WXGTK__

    virtual bool IsModified() const; // not in wx2.8; virtual in wx trunk so make it virtual here too
    virtual void DiscardEdits();     // not in wx2.8; virtual in wx trunk so make it virtual here too
    virtual void MarkDirty();        // not in wx2.8; virtual in wx trunk so make it virtual here too

    // ----------------------------------------------------------------------

    /// Colourize the whole document.
    void ColouriseDocument() { Colourise(0, -1); }

    /// Returns true if both whitespace and EOL are shown, else false.
    bool GetViewNonPrint() const;
    /// Show or hide both whitespace and EOL markers.
    void SetViewNonPrint(bool show_non_print);

    /// @}
    // ------------------------------------------------------------------------
    /// @name Convenience functions and other useful functions.
    /// @{

    /// Translate the start and end positions in the document, returns true if !empty.
    /// If start_pos == end_pos == -1 use current selection/target,
    ///   if empty selection/target use cursor line based on the STE_TranslatePosType.
    /// To get the whole document use (0, GetLength()-1) or (0, -1).
    bool TranslatePos(STE_TextPos start_pos, STE_TextPos end_pos,
                      STE_TextPos* trans_start_pos, STE_TextPos* trans_end_pos,
                      STE_TranslatePosType type = STE_TRANSLATE_SELECTION);
    /// Translate the top and bottom lines in the document, returns true if they differ.
    /// If top_line == bottom_line == -1 then use top/bottom line of selection/target,
    ///   if empty selection/target use the cursor line based on STE_TranslatePosType.
    /// If top_line = 0 and bottom_line = -1 use (0, GetLineCount()-1).
    bool TranslateLines(int  top_line,       int  bottom_line,
                        int* trans_top_line, int* trans_bottom_line,
                        STE_TranslatePosType type = STE_TRANSLATE_SELECTION);

    /// Returns true if the text range is a complete word.
    bool TextRangeIsWord(STE_TextPos start_pos, STE_TextPos end_pos) const;

    // ------------------------------------------------------------------------

    /// Get the text between the GetTargetStart() and GetTargetEnd().
    wxString GetTargetText() const;

    // ------------------------------------------------------------------------

    /// Is text available in the single (both is not allowed) specified clipboard
    ///   in any usable text format.
    /// Formats tested are wxDF_TEXT and if avilable wxDF_UNICODETEXT and wxDF_HTML.
    static bool IsClipboardTextAvailable(STE_ClipboardType clip_type = STE_CLIPBOARD_DEFAULT);
    /// Returns true if there is data in the single (both is not allowed) specified
    ///   clipboard with the given formats.
    /// This function takes an array since the clipboard has to be opened to test formats.
    static bool IsClipboardFormatAvailable(const enum wxDataFormatId* array, size_t array_count,
                                           STE_ClipboardType clip_type = STE_CLIPBOARD_DEFAULT);
    /// Get the current text in the single specified clipboard into the buf.
    /// Returns true if the clipboard was opened and the buf is not empty.
    static bool GetClipboardText(wxString* buf, STE_ClipboardType clip_type = STE_CLIPBOARD_DEFAULT);
    /// Set the text to the specified clipboard(s).
    static bool SetClipboardText(const wxString& str, STE_ClipboardType clip_type = STE_CLIPBOARD_DEFAULT);
    /// Set the HTML text to the clipboard. In MSW the clipboard will contain
    /// a valid HTML data object and a text object, on other systems the
    /// clipboard will only contain a text object.
    static bool SetClipboardHtml(const wxString& htmldata);

    /// Paste the text from the clipboard into the text at the current cursor
    /// position preserving the linefeeds in the text using PasteRectangular().
    bool PasteRectangular();
    /// Paste the text into the document using the column of pos, -1 means
    /// current cursor position, as the leftmost side for linefeeds.
    void PasteRectangular(const wxString& str, STE_TextPos pos = -1);

    // ------------------------------------------------------------------------

    /// Convert the wxSTC_EOL_CRLF, wxSTC_EOL_CR, wxSTC_EOL_LF type to the
    /// wxTextFileType wxTextFileType_Dos, wxTextFileType_Mac, wxTextFileType_Unix.
    /// Returns wxTextBuffer::typeDefault for unknown input.
    static wxTextFileType ConvertEOLModeType(int stc_eol_mode);

    /// Convert the input string to have the appropriate line endings for
    /// the wxSTC_EOL_XXX type.
    static wxString ConvertEOLMode(const wxString& str, int stc_eol_mode);

    /// Get the wxSTC_EOL_XXX string "\r", "\n", "\r\n" as appropriate for
    /// Mac, Unix, DOS. Default input (-1) gets EOL string for current doc settings.
    wxString GetEOLString(int stc_eol_mode = -1) const;

    /// AppendText to the document and if the cursor was already at the end of
    /// the document keep the cursor at the end.
    /// This is useful for scrolling logs so the user can click above the end and
    /// read the message without it scrolling off the screen as new text is added below.
    /// If goto_end then always put the cursor at the end.
    void AppendTextGotoEnd(const wxString &text, bool goto_end = false);

    /// Write the text to the line, adding lines if necessary.
    /// If inc_newline then also overwrite the newline char at end of line.
    /// @see GetLineText() in wxTextCtrl compatibility functions
    /// which excludes any crlf at the end of the line.
    void SetLineText(int line, const wxString& text, bool inc_newline = false);

    /// Go to the start of the current line.
    void GotoStartOfCurrentLine() { GotoLine(LineFromPosition(GetInsertionPoint())); }

    /// Get the number of words in the string, counts words as contiguous isalnum().
    size_t GetWordCount(const wxString& text) const;
    /// Get the number of words, counts words as contiguous isalnum().
    /// @see TranslatePos(start_pos, end_pos) for start/end interpretation.
    size_t GetWordCount(STE_TextPos start_pos = 0, STE_TextPos end_pos = -1,
                        STE_TranslatePosType type = STE_TRANSLATE_SELECTION);
    /// Get the count of the "words", they may be single characters and
    /// they may also be parts of other words. Returns total count.
    /// The output int array contains the count in the same order as the words array.
    size_t GetWordArrayCount(const wxString& text,
                             const wxArrayString& words, wxArrayInt& count,
                             bool ignoreCase = false);
    /// Get the EOL count for each EOL type and tabs, each and all inputs can be NULL.
    void GetEOLCount(int *crlf, int *cr, int *lf, int *tabs = NULL);

    /// Set the indentation of a line or set of lines, width is usually GetIndent().
    /// @see TranslateLines(top_line, bottom_line) for top/bottom interpretation.
    void SetIndentation(int width, int top_line = -1, int bottom_line = -1,
                        STE_TranslatePosType type = STE_TRANSLATE_SELECTION);

    /// Convert tab characters to spaces uses GetTabWidth() for # spaces to use,
    /// returns the number of replacements.
    /// @see TranslatePos(start_pos, end_pos) for start/end interpretation.
    size_t ConvertTabsToSpaces(bool to_spaces, STE_TextPos start_pos = -1, STE_TextPos end_pos = -1,
                               STE_TranslatePosType type = STE_TRANSLATE_SELECTION);
    /// Remove all trailing spaces and tabs from the document.
    /// @see TranslateLines(top_line, bottom_line) for top/bottom interpretation.
    bool RemoveTrailingWhitespace(int top_line = -1, int bottom_line = -1);
    /// Remove chars before and after the position until char not in remove found.
    /// Only works on a single line, if pos == -1 then use GetCurrentPos().
    bool RemoveCharsAroundPos(STE_TextPos pos = -1, const wxString& remove = wxT(" \t"));
    /// Inserts specified text at the column (adding spaces as necessary).
    /// If col == 0, prepend text, < 0 then append text, else insert at column.
    /// @see TranslateLines(top_line, bottom_line) for top/bottom interpretation.
    bool InsertTextAtCol(int col, const wxString& text,
                         int top_line = -1, int bottom_line = -1);

    /// Put all the text in the input lines in equally spaced columns using the chars
    /// to split in cols before, after, and what chars will bound regions that
    /// you want to preserve (like strings).
    bool Columnize(int top_line = -1, int bottom_line = -1,
                   const wxString& splitBefore   = wxT(")]{}"),
                   const wxString& splitAfter    = wxT(",;"),
                   const wxString& preserveChars = wxT("\""),
                   const wxString& ignoreAfterChars = wxEmptyString);

    // ------------------------------------------------------------------------

    /// Show a dialog that allows the user to append, prepend, or insert text
    ///  in the selected lines or the current line.
    bool ShowInsertTextDialog();
    /// Show a dialog to allow the user to turn the selected text into columns.
    bool ShowColumnizeDialog();
    /// Show a convert EOL dialog to allow the user to select one.
    bool ShowConvertEOLModeDialog();
    /// Show a dialog to allow the user to select a text size zoom to use.
    bool ShowSetZoomDialog();
    /// Simple dialog to goto a particular line in the text.
    bool ShowGotoLineDialog();

    // ------------------------------------------------------------------------

    /// Calls ToggleFold on the parent fold of the line (if any).
    /// If line = -1 then use the current line.
    void ToggleFoldAtLine(int line = -1);
    /// Expand all folds at and above or below the level.
    void ExpandFoldsToLevel(int level, bool expand = true);
    /// Collapse all folds at and above or below the level.
    void CollapseFoldsToLevel(int level) { ExpandFoldsToLevel(level, false); }
    /// Expand all the folds in the document.
    void ExpandAllFolds()   { ExpandFoldsToLevel(wxSTC_FOLDLEVELNUMBERMASK, true); }
    /// Collapse all the folds in the document.
    void CollapseAllFolds() { CollapseFoldsToLevel(0); }

    // ------------------------------------------------------------------------

#if (wxVERSION_NUMBER >= 2902)
    /// Get a wxVersionInfo for the wxStEdit library.
    /// See also wxStyledTextCtrl::GetLibraryVersionInfo().
    static wxVersionInfo GetStEditorVersionInfo();
#else
    /// Get a wxString of the version information for the wxStEdit library.
    /// See also wxStyledTextCtrl::GetLibraryVersionInfo().
    static wxString GetStEditorVersionString();
#endif

    /// @}
    // ------------------------------------------------------------------------
    /// @name Load/Save methods
    /// @{

    /// Get the current filename, including path.
    wxFileName GetFileName() const;
    /// Set the current filename, including path.
    void SetFileName(const wxFileName& filename, bool send_event = false);

    /// Copy the current filename and path to the clipboard.
    bool CopyFilePathToClipboard();

    /// Returns true if the document was ever loaded from or saved to disk.
    bool IsFileFromDisk() const { return GetFileModificationTime().IsValid(); }

    /// Set the last modification time of the file on the disk (internal use).
    /// Doesn't read/write to/from disk and the time should be invalid if it wasn't
    /// loaded from a file in the first place.
    void SetFileModificationTime(const wxDateTime &dt);
    /// Get the last modification time of the file on the disk.
    wxDateTime GetFileModificationTime() const;

    /// Can/Should this document be saved.
    /// Returns false if the document hasn't been modified since the last time it
    /// was saved. Returns true if it was never saved, even if the document is
    /// not modified which is used to show that "new" files should be saved.
    bool CanSave() const { return IsModified() || !IsFileFromDisk(); }

#if wxUSE_STREAMS
    /// Load a file from the wxInputStream (probably a wxFileInputStream).
    /// Flags are enum STE_LoadFileType.
    bool LoadFile( wxInputStream& stream,
                   const wxFileName& filename,
                   int flags = STE_LOAD_QUERY_UNICODE,
                   wxWindow* parent = NULL,
                   const wxString& encoding = wxEmptyString);

    /// Load a file from the wxInputStream (probably a wxFileInputStream).
    /// Flags are enum STE_LoadFileType.
    /// If successful: sets filename, modified time, encoding, and bom.
    bool LoadFileToString( wxString* filedata,
                           wxInputStream& stream,
                           const wxFileName& filename,
                           int flags = STE_LOAD_QUERY_UNICODE,
                           wxWindow* parent = NULL,
                           const wxString& encoding = wxEmptyString);

    /// Save the text to wxOutputStream only, does not update editor.
    bool SaveFile( wxOutputStream& stream,
                   const wxString& encoding = wxEmptyString,
                   bool file_bom = false);
#endif

    /// Load a file, if filename is wxEmptyString then use wxFileSelector.
    /// If using wxFileSelector then if extensions is wxEmptyString use
    /// GetOptions().GetDefaultFileExtensions() else the ones supplied.
    virtual bool LoadFile( const wxFileName& fileName = wxFileName(),
                           const wxString &extensions = wxEmptyString,
                           bool query_if_changed = true,
                           const wxString& encoding = wxEmptyString);
    /// Save current file, if use_dialog or GetFileName() is empty use wxFileSelector.
    virtual bool SaveFile( bool use_dialog = true,
                           const wxString &extensions = wxEmptyString );
    /// Save the file and update the editor settings.
    virtual bool SaveFile( const wxFileName& fileName,
                           const wxString& fileEncoding,
                           bool write_file_bom );

    /// Helper function to create the dialog to ask the user if they want to save.
    /// The selected* variables are only changed if the user presses Ok
    /// and the the function returns true.
    virtual bool SaveFileDialog( bool use_dialog,
                                 const wxString &extensions,
                                 wxFileName* selectedFileName,
                                 wxString*   selectedFileEncoding,
                                 bool*       selected_file_bom);
    /// Clear everything to a blank page.
    /// If title is empty then pop up a dialog to ask the user what name to use.
    virtual bool NewFile(const wxString &title = wxEmptyString);

    /// Revert the document to the version on disk if it was ever loaded.
    /// Returns success on reloading the file, false if it was never saved or
    /// couldn't be reread for some reason.
    bool Revert();

    /// Replace the entire text, and do wxSTC setup, don't forget to set filename
    /// and file modification time.
    void SetTextAndInitialize(const wxString& text);

    /// If IsModified() show a message box asking if the user wants to save the file.
    /// Returns wxYES, wxNO, wxCANCEL.
    /// If wxYES then the file is automatically saved if save_file is true.
    ///
    /// wxCANCEL should be used when the program is closing to allow the user to
    ///   cancel the exit and continue editing.
    /// Use EVT_CLOSE in frame before hiding the frame for this dialog.
    /// Check for wxCloseEvent::CanVeto and if it can't be vetoed use the
    ///   style wxYES_NO only since it can't be canceled.
    virtual int QuerySaveIfModified(bool save_file, int style = wxYES_NO|wxCANCEL);

    /// If there's a valid filename, return false if its modification time is
    ///   before the file on disk time or if it is invalid (not loaded from disk).
    /// If show_reload_dialog then ask user if they want to reload.
    /// If yes then the file is reloaded else modified time is set to an
    ///   invalid time so that the user won't be asked again.
    bool IsAlteredOnDisk(bool show_reload_dialog);

    /// Show a dialog to allow users to export the document.
    ///  @see wxSTEditorExporter
    bool ShowExportDialog();

    /// Show a modal dialog that displays the properties of this editor.
    /// @see wxSTEditorPropertiesDialog
    void ShowPropertiesDialog();

    /// @}
    // ------------------------------------------------------------------------
    /// @name Find/Replace methods
    ///
    /// A note about the find dialog system.
    ///
    /// When you create a find/replace dialog this checks if the grandparent is
    /// a wxSTEditorNotebook and uses that as a parent. Else, if the parent is
    /// a wxSTEditorSplitter then use that as a parent to avoid this being 2nd
    /// window and user unsplits. Finally this is used as a parent.
    ///
    /// Find/Replace events from the dialog are sent to the parent (see above).
    /// If in a notebook and STE_FR_ALLDOCS is set the notebook handles the event
    /// and switches pages automatically to find the next occurance, else the
    /// current editor handles the event. If the splitter is the parent, it does
    /// nothing and passes the event to the current editor.
    ///
    /// ShowFindReplaceDialog(true...) will Destroy() a previously made dialog by
    /// checking if a window exists with the name
    /// wxSTEditorFindReplaceDialogNameStr and creates a new one so there will only
    /// ever be one created.
    /// @{

    /// Find the string using the STEFindReplaceFlags flags.
    /// @param findString The string to search for.
    /// @param start_pos The starting position, -1 uses GetCursorPos().
    /// @param end_pos The ending position, -1 uses GetTextLength() or
    ///   if flags doesn't have STE_FR_DOWN then 0 would be the end.
    /// @param flags If = -1 uses GetFindFlags(), else use ored values of STEFindReplaceFlags.
    /// @param action Type STE_FindStringType selects, goto, or do nothing.
    /// @param found_start_pos If !NULL are set to the start pos of the string.
    /// @param found_end_pos If !NULL are set to the end pos of string.
    /// Note: found_end_pos - found_start_pos might not be the string length for regexp.
    /// @returns Starting position of the found string.
    STE_TextPos FindString(const wxString &findString,
                           STE_TextPos start_pos = -1,
                           STE_TextPos end_pos = -1,
                           int flags = -1,
                           int action = STE_FINDSTRING_SELECT|STE_FINDSTRING_GOTO,
                           STE_TextPos* found_start_pos = NULL,
                           STE_TextPos* found_end_pos = NULL);
    /// Does the current selection match the findString using the flags.
    /// If flags = -1 uses GetFindFlags(), else use ored values of STEFindReplaceFlags.
    bool SelectionIsFindString(const wxString &findString, int flags = -1);
    /// Replace all occurances of the find string with the replace string.
    /// If flags = -1 uses GetFindFlags(), else use ored values of STEFindReplaceFlags.
    /// @returns The number of replacements.
    int ReplaceAllStrings(const wxString &findString,
                          const wxString &replaceString, int flags = -1);
    /// Finds all occurances of the string and returns their starting positions.
    /// If flags = -1 uses GetFindFlags(), else use ored values of STEFindReplaceFlags.
    /// If startPositions is !NULL then fill with the starting positions.
    /// If endPositions if !NULL then fill that with the ending positions
    /// Note: For regexp end - start might not equal the findString length.
    /// @returns The number of strings found.
    size_t FindAllStrings(const wxString &findString, int flags = -1,
                          wxArrayInt* startPositions = NULL,
                          wxArrayInt* endPositions = NULL);

    /// Show the find or replace dialog to the user.
    void ShowFindReplaceDialog(bool find);
    /// Get the currently shown find/replace dialog or NULL.
    wxSTEditorFindReplaceDialog* GetCurrentFindReplaceDialog();

    /// Get the find replace data from the options.
    wxSTEditorFindReplaceData *GetFindReplaceData() const;
    /// Get the current string to find.
    wxString GetFindString() const;
    /// Get the current replace string.
    wxString GetReplaceString() const;
    /// Set the current string to find, only sends wxEVT_STEDITOR_STATE_CHANGED
    /// with state change STE_CANFIND if value changes.
    void SetFindString(const wxString &str, bool send_evt = false);
    /// Get the STEFindReplaceFlags flags used to find a string.
    int GetFindFlags() const;
    /// Set the current find flags, only sends wxEVT_STEDITOR_STATE_CHANGED
    /// with state change STE_CANFIND if flags change.
    void SetFindFlags(long flags, bool send_evt = false);
    /// Get the direction of search.
    bool GetFindDown() const { return (GetFindFlags() & wxFR_DOWN) != 0; }
    /// Returns false if the last search failed and the flags or the find string hasn't changed.
    bool CanFind() const { return HasState(STE_CANFIND); }
    /// Reset the canfind variable in case you change something else.
    void SetCanFind(bool can_find) { SetStateSingle(STE_CANFIND, can_find); }

    /// @}
    // ------------------------------------------------------------------------
    /// @name Set/ClearIndicator methods
    /// @{

    /// Indicate a section of text starting at pos of length len with indic type wxSTC_INDIC(0,1,2)_MASK.
    void SetIndicator(STE_TextPos pos, int len, int indic);
    /// Indicates all strings using indic type wxSTC_INDIC(0,1,2)_MASK.
    /// If str = wxEmptyString use GetFindString(), if find_flags = -1 use GetFindFlags()|STE_FR_WHOLEDOC.
    /// The optional arrays will be filled with start and or end positions that were indicated.
    bool IndicateAllStrings(const wxString &str = wxEmptyString, int find_flags = -1, int indic = wxSTC_INDIC0_MASK,
                            wxArrayInt* startPositions = NULL, wxArrayInt* endPositions = NULL);
    /// Clear a single character of indicated text of indic type wxSTC_INDIC(0,1,2)_MASK or -1 for all.
    bool ClearIndicator(int pos, int indic = wxSTC_INDIC0_MASK);
    /// Clear an indicator starting at any position within the indicated text of
    ///   Indic type wxSTC_INDIC(0,1,2)_MASK or -1 for all.
    /// @return The position after last indicated text or -1 if nothing done.
    int ClearIndication(int pos, int indic = wxSTC_INDIC0_MASK);
    /// Clears all the indicators of type wxSTC_INDIC(0,1,2)_MASK or -1 for all.
    void ClearAllIndicators(int indic = -1);

    /// @}
    // ------------------------------------------------------------------------
    /// @name Preprocessor and brace matching functions.
    /// @{

    // These are not currently used
    int  IsLinePreprocessorCondition(const wxString &line);
    bool FindMatchingPreprocessorCondition(int &curLine, int direction,
                                           int condEnd1, int condEnd2);
    bool FindMatchingPreprocCondPosition(bool isForward, STE_TextPos& mppcAtCaret, STE_TextPos& mppcMatch);

    /// Internal use brace matching and highlighting other brace.
    bool DoFindMatchingBracePosition(STE_TextPos& braceAtCaret, STE_TextPos& braceOpposite, bool sloppy);
    /// Internal use brace matching and highlighting other brace.
    void DoBraceMatch();

    /// Get the column of the caret in the line it's currently in.
    STE_TextPos GetCaretInLine();

    /// @}
    // ------------------------------------------------------------------------
    /// @name Autocomplete functions
    /// @{

    /// Remove duplicates and sort a space separated list of words.
    /// Returns true if the word list was modified.
    wxString EliminateDuplicateWords(const wxString& words) const;

    /// Get keywords as a space separated list from the langs that begin with root
    virtual wxString GetAutoCompleteKeyWords(const wxString& root);
    /// Add the matching keywords from the langs to the array string.
    /// You may override this function to provide additional words if desired.
    /// @returns The number of words added.
    virtual size_t DoGetAutoCompleteKeyWords(const wxString& root, wxArrayString& words);

    /// Show the autocompletion box if any keywords from the langs match the start
    /// of the current word.
    bool StartAutoComplete();
    /// Show the autocompletion box if any other words in the document match the start
    ///   of the current word.
    /// If onlyOneWord then if more than one word then don't show box.
    /// If add_keywords then also add the keywords from the langs.
    virtual bool StartAutoCompleteWord(bool onlyOneWord, bool add_keywords);

    /// @}
    // ------------------------------------------------------------------------
    /// @name Printing/Rendering methods
    /// @{

    /// Show the wxWidgets print dialog.
    bool ShowPrintDialog();
    /// Show the wxWidgets print preview dialog.
    bool ShowPrintPreviewDialog();
    /// Show the wxWidgets printer setup dialog (papersize, orientation...).
    bool ShowPrintSetupDialog();
    /// Show the wxWidgets print page setup dialog (papersize, margins...).
    bool ShowPrintPageSetupDialog();
    /// Show a STC specific options dialog (wrapmode, magnification, colourmode).
    bool ShowPrintOptionsDialog();

    /// @}
    // ------------------------------------------------------------------------
    /// @name Menu/MenuBar/Toolbar management
    /// @{

    /// Update all the menu/tool items in the wxSTEditorOptions for this editor.
    virtual void UpdateAllItems();
    /// Update all the known IDs for a menu, menubar, toolbar which may be NULL
    ///  to not be updated.
    virtual void UpdateItems( wxMenu *menu = NULL,
                              wxMenuBar *menuBar = NULL,
                              wxToolBar *toolBar = NULL );
    /// Handle menu events of known types, returns success, false for unknown IDs.
    virtual bool HandleMenuEvent(wxCommandEvent &event);

    /// Update all the CanSave(), CanXXX() functions to see if they've changed and
    ///   send appropriate events as to what has changed for updating the UI.
    /// This is called in OnSTCUpdateUI(), but Saving doesn't generate an event
    ///   but CanSave becomes false. Make sure to call this in odd cases like this.
    /// If send_event is false don't send events just update internal values.
    /// @returns The changes in GetState() since the last call to this function.
    long UpdateCanDo(bool send_event);

    /// Get combinations enum STE_StateType.
    long GetState() const     { return GetSTERefData()->m_state; }
    /// Set combinations enum STE_StateType.
    void SetState(long state) { GetSTERefData()->m_state = state; }
    /// Get if one or a combination of enum STE_StateType is set.
    bool HasState(long ste_statetype) const   { return (GetSTERefData()->m_state & ste_statetype) != 0; }
    /// Set a single or combination of enum STE_StateType.
    void SetStateSingle(long state, bool set) { if (set) SetState(GetSTERefData()->m_state | state); else SetState(GetSTERefData()->m_state & ~state); }

    /// @}
    // ------------------------------------------------------------------------
    /// @name Lexer Language - you must have set wxSTEditorLangs
    /// @{

    /// Setup colouring and lexing based on wxSTEditorLangs type.
    bool SetLanguage(int lang);
    /// Setup colouring and lexing based on wxSTEditorLangs::GetFilePattern().
    bool SetLanguage(const wxFileName&);
    /// What language are we using, the index into wxSTEditorLangs.
    /// This may or may not match wxStyledTextCtrl::GetLexer() since
    /// different languages may use the same lexer. (Java uses CPP lexer)
    int  GetLanguageId() const;

    void SetFileEncoding(const wxString&);
    wxString GetFileEncoding() const;

    void SetFileBOM(bool);
    bool GetFileBOM() const;

    /// @}
    // ------------------------------------------------------------------------
    /// @name Editor preferences, styles, languages.
    ///
    /// There are global versions that many editors can share and they're ref
    /// counted so you don't need to keep any local versions around.
    /// You should use the globals in at least one editor to not let them go
    /// to waste. There's nothing special about them, it's just that if you're
    /// bothering to use this class you'll probably want at least one of each.
    ///
    /// The prefs/styles/langs are initially not set for an editor, but can be
    /// set by the function CreateOptions if you have set them in the options.
    /// If however you wish to use different ones you may call RegisterXXX to
    /// "detach" this editor from the others.
    ///
    /// Example usage :
    /// @code
    ///    editor->RegisterPreferences(wxSTEditorPrefs()); // no prefs
    ///
    ///    wxSTEditorPrefs myPrefs(true);                 // create
    ///    myPrefs.SetPrefBool(STE_PREF_VIEWEOL, true);   // adjust as necessary
    ///    editor->RegisterPreferences(myPrefs);          // assign to editor
    /// @endcode
    /// @{

    /// Register this editor to use these preferences.
    /// The default is to use the prefs in the options (if set) which by
    ///   default will be the static global wxSTEditorPrefs::GetGlobalEditorPrefs().
    /// RegisterPrefs(wxSTEditorPrefs(false)) to not use any preferences.
    void RegisterPrefs(const wxSTEditorPrefs& prefs);
    const wxSTEditorPrefs& GetEditorPrefs() const;
    wxSTEditorPrefs& GetEditorPrefs();

    /// Register this editor to use these styles.
    /// The default is to use the styles in the options (if set) which by
    ///   default will be the static global wxSTEditorStyles::GetGlobalEditorStyles().
    /// RegisterStyles(wxSTEditorStyles(false)) to not use any styles.
    void RegisterStyles(const wxSTEditorStyles& styles);
    const wxSTEditorStyles& GetEditorStyles() const;
    wxSTEditorStyles& GetEditorStyles();

    /// Register this editor to use these languages.
    /// The default is to use the langs in the options (if set) which by
    ///   default will be the static global wxSTEditorLangs::GetGlobalEditorLangs().
    /// RegisterLangs(wxSTEditorLangs(false)) to not use any languages.
    void RegisterLangs(const wxSTEditorLangs& langs);
    const wxSTEditorLangs& GetEditorLangs() const;
    wxSTEditorLangs& GetEditorLangs();

    /// @}
    // -----------------------------------------------------------------------
    /// @name Implementation
    /// @{

    /// Setup and send an event.
    /// Note for a wxEVT_STEDITOR_STATE_CHANGED event evt_int is the changed state and
    ///   extra_long is the state values.
    ///   Otherwise the int/long values are those in the wxCommandEvent
    bool SendEvent(wxEventType eventType, int evt_int = 0, long extra_long = 0,
                   const wxString &evtStr = wxEmptyString, bool do_post = false );

    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnSTCCharAdded(wxStyledTextEvent &event);
    void OnSTCUpdateUI(wxStyledTextEvent &event);
    void OnMouseWheel(wxMouseEvent& event); // FIXME - only for wxGTK 2.0
    void OnScroll(wxScrollEvent& event);
    void OnScrollWin(wxScrollWinEvent& event);

    void OnContextMenu(wxContextMenuEvent&); // popup menu if one is set in options
    void OnMenu(wxCommandEvent &event);

    void OnSTCMarginClick(wxStyledTextEvent &event);
    void OnSTCMarginDClick(wxStyledTextEvent &event); // we generate this event
    void OnSetFocus(wxFocusEvent &event);
    void OnSTEFocus(wxSTEditorEvent &event);
    void OnEraseBackground(wxEraseEvent &event) { event.Skip(false); }

    // Note that these event functions below are intentionally out of order
    // in the source code so they're easier to maintain.

    void OnFindDialog(wxFindDialogEvent& event);
    virtual void HandleFindDialogEvent(wxFindDialogEvent& event);

    void OnSTEState(wxSTEditorEvent &event);

    // ------------------------------------------------------------------------

    /// Access the scrollbar in the wxStyledTextCtrl.
    wxScrollBar* GetHScrollBar() { return m_hScrollBar; }
    /// Access the scrollbar in the wxStyledTextCtrl.
    wxScrollBar* GetVScrollBar() { return m_vScrollBar; }

    /// Get the width in pixels of the longest line between top_line and
    ///  bottom_line takeing care of ctrl chars and tabs.
    /// If top_line = bottom_line = -1 then use the visible lines.
    /// Used to adjust the length of the horizontal scrollbar.
    int GetLongestLinePixelWidth(int top_line = -1, int bottom_line = -1);

    /// Check if we autoindented but the user didn't type anything and
    ///   remove the space we added.  Resets m_last_autoindent_line/len.
    bool ResetLastAutoIndentLine();

    // ------------------------------------------------------------------------

    /// Get the wxSTETreeItemData if this editor being tracked in the wxSTEditorTreeCtrl.
    wxSTETreeItemData* GetTreeItemData() const;
    /// Set the wxSTETreeItemData if this editor being tracked in the wxSTEditorTreeCtrl.
    void SetTreeItemData(wxSTETreeItemData* treeData);

    // ------------------------------------------------------------------------

    /// Get the wxClientData object you set or NULL if none set.
    wxClientData *GetClientObject() const;
    /// Set a new wxClientData object which will be deleted when all refed
    /// copies of this editor are deleted.
    /// Note that you can use this to determine when it is deleted.
    void SetClientObject( wxClientData *data );

    // ------------------------------------------------------------------------
    /// Get the ref counted data for the editor (ref counted for splitting).
    /// The ref data is ALWAYS expected to exist, do NOT call wxObject::UnRef()
    /// unless you IMMEDIATELY replace it.
    wxSTEditorRefData* GetSTERefData() const { return (wxSTEditorRefData*)GetRefData(); }

    /// @}

protected:
    bool m_sendEvents; // block sending events if false
    bool m_activating; // are we in EVT_ACTIVATE already

    wxLongLong m_marginDClickTime;   // last time margin was clicked
    int        m_marginDClickLine;   // last line margin was clicked on
    int        m_marginDClickMargin; // last margin # clicked on

    wxSTERecursionGuardFlag m_rGuard_OnMenu;
    wxSTERecursionGuardFlag m_rGuard_HandleMenuEvent;
    wxSTERecursionGuardFlag m_rGuard_OnFindDialog;

private:
    void Init();

    // Please use the virtual method IsModified() instead
    bool GetModify() { return wxStyledTextCtrl::GetModify(); }
    // Please use one of the virtual methods instead, SetModified(false) or DiscardEdits()
    void SetSavePoint() { wxStyledTextCtrl::SetSavePoint(); }

    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(wxSTEditor)
};

// --------------------------------------------------------------------------
// include the others so that only this file needs to be included for everything
#include "wx/stedit/steevent.h"
#include "wx/stedit/stenoteb.h"
#include "wx/stedit/steframe.h"
#include "wx/stedit/stesplit.h"
#include "wx/stedit/steprint.h"
#include "wx/stedit/stemenum.h"
#include "wx/stedit/stedlgs.h"
#include "wx/stedit/stefindr.h"

#endif  // _STEDIT_H_
