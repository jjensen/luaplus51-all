///////////////////////////////////////////////////////////////////////////////
// Name:        stedoc.cpp
// Purpose:     MDI wxSTEditor app
// Author:      Troels K
// Modified by:
// Created:     2012-01-19
// Copyright:   (c) Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#include <wx/docview.h>
#include "wx/stedit/stedit.h"

#include "stedoc.h"

IMPLEMENT_DYNAMIC_CLASS(wxSTEditorDoc, wxDocument)
IMPLEMENT_DYNAMIC_CLASS(wxSTEditorView, wxView)

wxSTEditorDoc::wxSTEditorDoc() : wxDocument(), wxSTEditorRefData(), m_living_in_wxSTEditorFrame(true)
{
    Init();
}

wxSTEditorDoc::wxSTEditorDoc(bool living_in_wxSTEditorFrame) : wxDocument(), wxSTEditorRefData(), m_living_in_wxSTEditorFrame(living_in_wxSTEditorFrame)
{
    Init();
}

void wxSTEditorDoc::Init()
{
    if (m_living_in_wxSTEditorFrame)
    {
        wxSTEditorDocTemplate::GetInstance()->InitDocument(this, wxEmptyString);
    }
}

wxSTEditorDoc::~wxSTEditorDoc()
{
    if (m_living_in_wxSTEditorFrame)
    {
        // Normally you destroy a document by calling wxDocManager::CloseDocument()
        // but wxSTEditorDoc is destroyed from elsewhere (from wxObject::UnRef).
        // Hence this roundabout method:
        GetFirstView()->SetDocument(NULL); // decouple, so the view can be destroyed without this document being deleting once more
        DeleteAllViews(); // destroy view
    }
}

void wxSTEditorDoc::Modify(bool mod)
{
#if (wxVERSION_NUMBER < 2900)
    bool asterisk = mod != m_documentModified;
#endif
    wxDocument::Modify(mod);
#if (wxVERSION_NUMBER < 2900)
    if (asterisk)
    {
        // Allow views to append asterix to the title
        wxView* view = GetFirstView();
        if (view) view->OnChangeFilename();
    }
#endif
}

wxPrintout* wxSTEditorView::OnCreatePrintout()
{
    return new wxSTEditorPrintout(m_text);
}

/*static*/ wxSTEditorDocTemplate* wxSTEditorDocTemplate::ms_instance = NULL;

wxSTEditorDocTemplate::wxSTEditorDocTemplate(wxDocManager* docManager) : 
    wxDocTemplate(docManager, _("Text"), wxT("*.txt;*.text;*.h;*.c;*.cpp"),
      wxT(""), wxT("txt"), wxT("Editor doc"), wxT("Editor view"),
          CLASSINFO(wxSTEditorDoc), CLASSINFO(wxSTEditorView))
{
    ms_instance = this;
    STE_GlobalRefDataClassInfo = m_docClassInfo;
}

/*static*/ wxSTEditorDocTemplate* wxSTEditorDocTemplate::Create(wxDocManager* docManager)
{
   return new wxSTEditorDocTemplate(docManager);
}
