////////////////////////////////////////////////////////////////////////////////
// Name:        stedoc.h
// Purpose:     MDI wxSTEditor app
// Author:      Troels K
// Modified by:
// Created:     2012-01-19
// Copyright:   (c) Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _STEDOC_H_
#define _STEDOC_H_

//-----------------------------------------------------------------------------
// wxSTEditorDoc/View are not really good wxWidgets doc/view citizens because
// - creation and destruction are not handled by wxDocManager
//     (see workarounds in wxSTEditorDoc ctor and dtor)
// - has only a dummy view instance
//     (not in sync with wxSTEditorRefData.m_editors)
// - wxView::GetFrame() returns NULL
//
// Still this module may be useful for
// - demonstating how to roll your own wxSTEditorRefData class type in user space
// - demonstating how to wire wxSTEditor/wxSTEditorRefData into the wxWidgets
//   doc/view framework (albeit in a somewhat limited way)
// - if your user app is built on the doc/view framework and has its own doc/views,
//   then wxSTEditor fits in better when it can produce a wxDocument instance
//   for you, eg you can implement "universal" File Open code that returns a non-NULL
//   wxDocument pointer always, if succesful,
//      wxDocument* OpenSomeFile(filename); // Returns a wxSTEditorDoc instance
//                                          // *or* some other wxDocument instance
// 
//-----------------------------------------------------------------------------

class wxSTEditorDoc : public wxDocument, public wxSTEditorRefData
{
    DECLARE_DYNAMIC_CLASS(wxSTEditorDoc)
public:
    wxSTEditorDoc();
    wxSTEditorDoc(bool living_in_wxSTEditorFrame);

    virtual ~wxSTEditorDoc();

    wxFileName GetFilename() const
    { 
        return m_fileName;
    }

    virtual void SetFilename(const wxFileName& fileName, bool notifyViews = false)
    {
        m_fileName = fileName;
        wxDocument::SetFilename(fileName.GetFullPath(), notifyViews);
    }
    virtual void Modify(bool);
private:
    void Init();
    bool m_living_in_wxSTEditorFrame;
};

class wxSTEditorView : public wxView
{
    DECLARE_DYNAMIC_CLASS(wxSTEditorView)
public:
    wxSTEditorView() : wxView(), m_text(NULL) {}

    wxSTEditorDoc* GetDocument()
    {
        return wxStaticCast(wxView::GetDocument(), wxSTEditorDoc);
    }

    virtual void OnDraw(wxDC*)
    {
        // nothing to do here, wxSTEditor draws itself
    }
    virtual wxPrintout* OnCreatePrintout();
    virtual wxWindow* GetWindow() const { return m_text; }

#if (wxVERSION_NUMBER < 2900)
    virtual void OnChangeFilename()
    {
       //base::OnChangeFilename(); // skip
       wxWindow *win = GetFrame();
       if (!win) return;

       wxDocument *doc = GetDocument();
       if (!doc) return;

       wxString label = doc->GetUserReadableName();
       if (doc->IsModified())
       {
          label += wxT('*');
       }
       win->SetLabel(label);
    }
#endif

    wxSTEditor* GetEditor() const { return m_text; }
protected:
    wxSTEditor* m_text;
};

class wxSTEditorDocTemplate : public wxDocTemplate
{
   wxSTEditorDocTemplate(wxDocManager*);
   static wxSTEditorDocTemplate* ms_instance;
public:
   static wxSTEditorDocTemplate* Create(wxDocManager*);
   static wxSTEditorDocTemplate* GetInstance() { return ms_instance; }
};

#endif  // _STEDOC_H_
