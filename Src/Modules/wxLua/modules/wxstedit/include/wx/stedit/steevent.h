///////////////////////////////////////////////////////////////////////////////
// Name:        steevent.h
// Purpose:     wxSTEditorEvent
// Author:      John Labenski
// Modified by:
// Created:     11/05/2002
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file steevent.h
/// @brief wxSTEditorEvent, a wxCommandEvent derived class for wxSTEditor events and wxEVT_ST* declarations.

#ifndef _STEEVENT_H_
#define _STEEVENT_H_

#include <wx/defs.h>
#include <wx/event.h>

#include "wx/stedit/stedefs.h"

class WXDLLIMPEXP_FWD_STEDIT wxSTEditor;

//-----------------------------------------------------------------------------
/// @class wxSTEditorEvent
/// @brief A specialized wxCommandEvent for use with the wxSTEditor.
//-----------------------------------------------------------------------------
class WXDLLIMPEXP_STEDIT wxSTEditorEvent : public wxCommandEvent
{
public:
    wxSTEditorEvent() : wxCommandEvent() {}
    wxSTEditorEvent(const wxSTEditorEvent& event) : wxCommandEvent(event) {}
    wxSTEditorEvent( int id, wxEventType type, wxObject* obj,
                     int stateChange, int stateValues,
                     const wxString& fileName );

    virtual ~wxSTEditorEvent() {}

    /// Has the state of the editor changed see STE_StateType for different states.
    /// Can OR states together to see if any of them have changed.
    bool HasStateChange(int stateChange) const         { return (GetStateChange() & stateChange) != 0; }
    bool GetStateValue(STE_StateType stateValue) const { return (GetStateValues() & stateValue)  != 0; }

    /// Get the changes of the wxSTEditor::GetState() for the wxEVT_STEDITOR_STATE_CHANGED.
    int  GetStateChange() const          { return GetInt(); }
    /// Get the wxSTEditor::GetState() for any wxEVT_STEDITOR_* events.
    int  GetStateValues() const          { return int(GetExtraLong()); }
    void SetStateChange(int stateChange) { SetInt(stateChange); }
    void SetStateValues(int stateValues) { SetExtraLong(stateValues); }

    ///. Get the filename of the wxStEditor for wxEVT_STEDITOR_* events.
    wxFileName GetFileName() const                 { return wxFileName(GetString()); }
    void SetFileName( const wxFileName& fileName ) { SetString( fileName.GetFullPath() ); }

    wxSTEditor* GetEditor() const;

    // implementation
    virtual wxEvent *Clone() const { return new wxSTEditorEvent(*this); }

private:
    DECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxSTEditorEvent)
};

// --------------------------------------------------------------------------
/// @name wxSTEditor wxEvent types

BEGIN_DECLARE_EVENT_TYPES()
/// @{
    /// wxSTEditor created, event.GetEventObject() is the editor, use to setup after constructor
    /// and at the end of the call to wxSTEditor::CreateOptions(...).
    ///   (this is a wxCommandEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STEDITOR_CREATED, 0)
    /// wxSTEditorSplitter created, event.GetEventObject() is the splitter, use to setup after constructor.
    ///   (this is a wxCommandEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STSPLITTER_CREATED, 0)
    /// wxSTEditorNotebook created, event.GetEventObject() is the notebook, use to setup after constructor.
    ///   (this is a wxCommandEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STNOTEBOOK_CREATED, 0)

    /// The state of the editor has changed see STE_StateType for the types of changes.
    /// An event to update the gui only when items change to avoid UpdateUI overkill.
    ///   (this is a wxSTEditorEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STEDITOR_STATE_CHANGED, 0)
    /// This wxSTEditor has the focus now, (serves to pass EVT_SET_FOCUS to parents).
    ///   (this is a wxSTEditorEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STEDITOR_SET_FOCUS, 0)
    /// The popup menu for the wxSTEditor is about to be shown, maybe you want to update it?
    ///   (this is a wxSTEditorEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STEDITOR_POPUPMENU, 0)

    /// The margin has been double clicked in the same line.
    ///   (this is a wxStyledTextEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STEDITOR_MARGINDCLICK, 0)

    /// A wxSTEditor is about to be created for the wxSTEditorSplitter.
    /// event.GetEventObject() is the parent wxSTEditorSplitter.
    /// You can set the event.SetEventObject() to a "new wxSTEditor" or a
    /// subclassed one of your own and this editor will be used instead.
    /// Make sure that the parent of your editor is the splitter
    ///    (ie. the original event.GetEventObject())
    /// event.GetInt() is the preferred id (probably wxID_ANY)
    ///   (this is a wxCommandEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STSPLITTER_CREATE_EDITOR, 0)

    /// A wxSTEditorSplitter is about to be created for the wxSTEditorNotebook.
    /// event.GetEventObject() is the parent wxSTEditorNotebook.
    /// You can set the event.SetEventObject() to a "new wxSTEditorSplitter" or a
    /// subclassed one of your own and this splitter will be used instead.
    /// Make sure that the parent of your splitter is the notebook
    ///    (ie. the original event.GetEventObject())
    /// event.GetInt() is the preferred id (probably wxID_ANY)
    ///   (this is a wxCommandEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STNOTEBOOK_CREATE_SPLITTER, 0)

    /// The user has clicked on one of the splitter buttons in the
    ///   wxSTEditor. This event is received by the splitter and then
    ///   the splitting occurs. The event.GetInt() is enum wxSPLIT_VERTICAL
    ///   or wxSPLIT_HORIZONTAL.
    ///   (this is a wxCommandEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STSPLITTER_SPLIT_BEGIN, 0)

    /// The wxNotebook doesn't always send enough events to follow it's state.
    ///  This event is sent whenever the selection or page count changes
    ///  eg. When all the pages are deleted, gtk doesn't notify you that the
    ///  selection is now -1
    ///   (this is a wxNotebookEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STNOTEBOOK_PAGE_CHANGED, 0)

    /// Enter has been pressed on the last line of the wxSTEditorShell.
    ///   event.GetString() contains the contents of the line.
    ///   (this is a wxSTEditorEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STESHELL_ENTER, 0)

    /// New find-all results are in a wxSTEditorFindResultsEditor.
    ///   event.GetEventObject() is the wxSTEditorFindResultsEditor with the results.
    /// This event is sent since the parents of the wxSTEditorFindResultsEditor,
    ///    be they a notebook, splitter, separate dialog, know how to show it.
    ///   (this is a wxCommandEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STEFIND_RESULTS_NEED_SHOWN, 0)

    /// Go to the file and line in an editor, probably from a previous find all.
    ///   event.GetString() can be parsed using wxSTEditorFindReplaceData::ParseFindAllString()
    ///   event.GetExtraLong() is the index into the wxSTEditorFindReplaceData::GetFindAllStrings().
    ///   event.GetEventObject() is the wxSTEditorFindResultsEditor who's results was selected.
    ///   (this is a wxFindDialogEvent)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_STEDIT, wxEVT_STEFIND_GOTO, 0)

/// @}
END_DECLARE_EVENT_TYPES()


#if !defined(wxStyledTextEventHandler) // not in < wx29
typedef void (wxEvtHandler::*wxStyledTextEventFunction)(wxStyledTextEvent&);

#define wxStyledTextEventHandler( func ) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxStyledTextEventFunction, &func)
    //wxEVENT_HANDLER_CAST( wxStyledTextEventFunction, func )
#endif // !defined(wxStyledTextEventHandler)

typedef void (wxEvtHandler::*wxSTEditorEventFunction)(wxSTEditorEvent&);

#define wxSTEditorEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxSTEditorEventFunction, &func)
#define StyledTextEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxStyledTextEventFunction, &func)

#define wx__DECLARE_STEEVT(evt, id, fn)  wx__DECLARE_EVT1( evt, id, wxSTEditorEventHandler(fn))
#define wx__DECLARE_STEVT(evt, id, fn)   wx__DECLARE_EVT1( evt, id, StyledTextEventHandler(fn))

// --------------------------------------------------------------------------
/// @name wxSTEditor static EVT_ST*() wxEvent handlers.
// --------------------------------------------------------------------------
/// @{

#define EVT_STEDITOR_CREATED(id, fn)            EVT_COMMAND(id, wxEVT_STEDITOR_CREATED,   fn)
#define EVT_STSPLITTER_CREATED(id, fn)          EVT_COMMAND(id, wxEVT_STSPLITTER_CREATED, fn)
#define EVT_STNOTEBOOK_CREATED(id, fn)          EVT_COMMAND(id, wxEVT_STNOTEBOOK_CREATED, fn)

#define EVT_STEDITOR_STATE_CHANGED(id, fn)      wx__DECLARE_STEEVT(wxEVT_STEDITOR_STATE_CHANGED, id, fn)
#define EVT_STEDITOR_SET_FOCUS(id, fn)          wx__DECLARE_STEEVT(wxEVT_STEDITOR_SET_FOCUS,     id, fn)
#define EVT_STEDITOR_POPUPMENU(id, fn)          wx__DECLARE_STEEVT(wxEVT_STEDITOR_POPUPMENU,     id, fn)

#define EVT_STEDITOR_MARGINDCLICK(id, fn)       wx__DECLARE_STEVT(wxEVT_STEDITOR_MARGINDCLICK,   id, fn)

#define EVT_STSPLITTER_CREATE_EDITOR(id, fn)    EVT_COMMAND(id, wxEVT_STSPLITTER_CREATE_EDITOR,   fn)
#define EVT_STNOTEBOOK_CREATE_SPLITTER(id, fn)  EVT_COMMAND(id, wxEVT_STNOTEBOOK_CREATE_SPLITTER, fn)

#define EVT_STNOTEBOOK_PAGE_CHANGED(id, fn)     wx__DECLARE_EVT1(wxEVT_STNOTEBOOK_PAGE_CHANGED, id, wxNotebookEventHandler(fn))

#define EVT_STSPLITTER_SPLIT_BEGIN(id, fn)      wx__DECLARE_EVT1(wxEVT_STSPLITTER_SPLIT_BEGIN,  id, wxCommandEventHandler(fn))

#define EVT_STESHELL_ENTER(id, fn)              wx__DECLARE_STEEVT(wxEVT_STESHELL_ENTER, id, fn)

#define EVT_STEFIND_RESULTS_NEED_SHOWN(id, fn)  wx__DECLARE_EVT1(wxEVT_STEFIND_RESULTS_NEED_SHOWN, id, wxCommandEventHandler(fn))
#define EVT_STEFIND_GOTO(id, fn)                wx__DECLARE_EVT1(wxEVT_STEFIND_GOTO,               id, wxFindDialogEventHandler(fn))

/// @}

#endif  // _STEEVENT_H_
