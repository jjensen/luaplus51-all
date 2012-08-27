///////////////////////////////////////////////////////////////////////////////
// Name:        stedocview.h
// Purpose:     MDI wxSTEditor app
// Author:      Troels K
// Modified by:
// Created:     2012-01-19
// Copyright:   (c) Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __STEDOCVIEW_H__
#define __STEDOCVIEW_H__

#ifdef _STEOPTS_H_
class EditorDocTemplate : public wxDocTemplate
{
    DECLARE_CLASS(EditorDocTemplate)
    static EditorDocTemplate* ms_instance;
protected:
    wxClassInfo* m_frameClassInfo;

    EditorDocTemplate(wxDocManager*, wxClassInfo* frameClassInfo);

    virtual wxFrame* CreateViewFrame(wxView*);
public:
    static EditorDocTemplate* Create(wxDocManager*);
    static EditorDocTemplate* GetInstance() { return ms_instance; }

    wxSTEditorOptions m_steOptions;

    friend class EditorView;
};
#endif

#ifdef _STEDIT_H_

// ----------------------------------------------------------------------------
// A very simple text document class
// ----------------------------------------------------------------------------

class EditorDoc : public wxSTEditorDoc
{
public:
    EditorDoc();

    wxSTEditor* GetTextCtrl() const;
    wxSTEditorOptions& GetOptions() { return m_options; }
    
    EditorDocTemplate* GetDocumentTemplate() const
    {
        return wxStaticCast(wxSTEditorDoc::GetDocumentTemplate(), EditorDocTemplate);
    }

    virtual ~EditorDoc();
    virtual bool OnCreate(const wxString& path, long flags);
    virtual bool OnNewDocument();

#if (wxVERSION_NUMBER >= 2900)
    virtual wxString GetUserReadableName() const
    {
        // full path in mdi child caption
        return GetFilename().IsOk() ? GetFilename().GetFullPath() : wxSTEditorDoc::GetUserReadableName();
    }
#else
    virtual bool GetPrintableName(wxString& buf) const
    {
        if (GetFilename().IsOk())
        {
            // full path in mdi child caption
            buf = GetFilename().GetFullPath();
            return true;
        }
        else
        {
            return wxSTEditorDoc::GetPrintableName(buf);
        }
    }
#endif
protected:
    virtual bool DoSaveDocument(const wxString& filename);
    virtual bool DoOpenDocument(const wxString& filename);

    DECLARE_DYNAMIC_CLASS(EditorDoc)
};

// ----------------------------------------------------------------------------
// Text view classes
// ----------------------------------------------------------------------------

class EditorView : public wxSTEditorView
{
    DECLARE_DYNAMIC_CLASS(EditorView)
public:
    EditorView();

    virtual ~EditorView();
    virtual bool OnCreate(wxDocument*, long flags);
    virtual bool OnClose(bool deleteWindow = true);
    virtual void OnChangeFilename();

    EditorDoc* GetDocument()
    {
        return wxStaticCast(wxSTEditorView::GetDocument(), EditorDoc);
    }
private:
    void OnMenu(wxCommandEvent&);
    DECLARE_EVENT_TABLE()
};

class EditorChildFrame : public wxDocMDIChildFrame
{
    DECLARE_DYNAMIC_CLASS(EditorChildFrame)
protected:
    wxSTEditorSplitter *m_steSplitter;
    wxSplitterWindow   *m_mainSplitter;  // splitter for notebook/editor and bottom notebook
    wxSplitterWindow   *m_sideSplitter;  // splitter for editor and left hand panels
    wxWindow           *m_sideSplitterWin1; // these are the two pages of the side splitter
    wxWindow           *m_sideSplitterWin2;
public:
    EditorChildFrame();

    bool Create(wxView*, wxMDIParentFrame*);

    EditorDoc* GetDocument() const
    { 
        return wxStaticCast(wxDocMDIChildFrame::GetDocument(), EditorDoc);
    }
    wxSTEditor* GetEditor()
    {
        return wxStaticCast(wxDocMDIChildFrame::GetView(), EditorView)->GetEditor();
    }

#if (wxVERSION_NUMBER < 2900)
    wxMDIParentFrame* GetMDIParent() const { return wxStaticCast(GetParent(), wxMDIParentFrame); }

    virtual void DoGiveHelp(const wxString& text, bool show)
    {
       wxMDIParentFrame* frame = GetMDIParent();
       frame->DoGiveHelp(text, show);
       //base::DoGiveHelp(text, show);
    }
#endif

    virtual ~EditorChildFrame();
    // Get the splitter between editor (notebook) and some user set window
    virtual wxSplitterWindow* GetMainSplitter() const { return m_mainSplitter; }
    // Get the splitter between sidebar notebook and editors, NULL if not style STF_SIDEBAR
    virtual wxSplitterWindow* GetSideSplitter() const { return m_sideSplitter; }
};
#endif // _STEDIT_H_

#endif // __STEDOCVIEW_H__
