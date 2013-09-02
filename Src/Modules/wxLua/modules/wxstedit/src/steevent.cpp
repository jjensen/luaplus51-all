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

#include "precomp.h"

#include "wx/stedit/steevent.h"
#include "wx/stedit/stedit.h"

//-----------------------------------------------------------------------------
// wxSTEditorEvent
//-----------------------------------------------------------------------------

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STEDITOR_CREATED)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_STSPLITTER_CREATED)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_STNOTEBOOK_CREATED)

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STEDITOR_STATE_CHANGED)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_STEDITOR_SET_FOCUS)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_STEDITOR_POPUPMENU)

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STEDITOR_MARGINDCLICK)

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STSPLITTER_CREATE_EDITOR)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_STNOTEBOOK_CREATE_SPLITTER)

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STSPLITTER_SPLIT_BEGIN)

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STNOTEBOOK_PAGE_CHANGED)

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STESHELL_ENTER)

DEFINE_LOCAL_EVENT_TYPE(wxEVT_STEFIND_RESULTS_NEED_SHOWN)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_STEFIND_GOTO)

//-----------------------------------------------------------------------------
// wxSTEditorEvent
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSTEditorEvent, wxCommandEvent)

wxSTEditorEvent::wxSTEditorEvent( int id, wxEventType type, wxObject* obj,
                                  int stateChange, int stateValues,
                                  const wxString& fileName )
                :wxCommandEvent(type, id)
{
    SetEventObject(obj);
    SetInt(stateChange);
    SetExtraLong(stateValues);
    SetString(fileName);
}

wxSTEditor* wxSTEditorEvent::GetEditor() const
{
    return wxDynamicCast(GetEventObject(), wxSTEditor);
}
