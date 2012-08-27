///////////////////////////////////////////////////////////////////////////////
// Name:        appdoc.cpp
// Purpose:     MDI wxSTEditor app
// Author:      Troels K
// Modified by:
// Created:     2012-01-19
// Copyright:   (c) Troels K
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "wx/stedit/steopts.h"
#include "stedocview.h"
#include "../../src/wxext.h"
#include "app.h"
#include "wx/stedit/steart.h"

class ExampleDocTemplate1 : public wxDocTemplate
{
    ExampleDocTemplate1(wxDocManager*);
protected:
    wxFrame* CreateViewFrame(wxView*);
public:
    static wxDocTemplate* Create(wxDocManager*);

    friend class ImageView;
};

// ----------------------------------------------------------------------------
// Image and image details document classes (both are read-only for simplicity)
// ----------------------------------------------------------------------------

// This is a normal document containing an image, just like wxSTEditorDoc
// above contains some text. It can be created from an image file on disk as
// usual.
class ImageDocument : public wxDocument
{
public:
    ImageDocument() : wxDocument() { }

    virtual bool OnNewDocument();
    virtual bool OnOpenDocument(const wxString& file);

    wxImage GetImage() const { return m_image; }

protected:
    virtual bool DoOpenDocument(const wxString& file);

private:
    wxImage m_image;

    DECLARE_DYNAMIC_CLASS(ImageDocument)
};

// ----------------------------------------------------------------------------
// ImageDocument and ImageDetailsDocument implementation
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(ImageDocument, wxDocument)

bool ImageDocument::DoOpenDocument(const wxString& file)
{
    return m_image.LoadFile(file);
}

bool ImageDocument::OnNewDocument()
{
    bool ok = wxDocument::OnNewDocument();

    if (ok)
    {
        wxArtID id = wxART_STEDIT_APP;

        m_image = wxArtProvider::GetBitmap(id).ConvertToImage();

        wxSize size(m_image.GetWidth(), m_image.GetHeight());
        size *= 10;
        m_image.Rescale(size.x, size.y);

        SetTitle(id);
        SetFilename(wxEmptyString, true);
    }
    return ok;
}

bool ImageDocument::OnOpenDocument(const wxString& filename)
{
    if ( !wxDocument::OnOpenDocument(filename) )
        return false;

    return true;
}

// ----------------------------------------------------------------------------
// ImageCanvas
// ----------------------------------------------------------------------------

class ImageCanvas : public wxScrolledWindow
{
public:
    ImageCanvas(wxView*);

    virtual void OnDraw(wxDC&);
private:
    wxView* m_view;
};

// ----------------------------------------------------------------------------
// ImageView
// ----------------------------------------------------------------------------

class ImageView : public wxView
{
public:
    ImageView() : wxView(), m_canvas(NULL) {}

    virtual bool OnCreate(wxDocument*, long flags);
    virtual void OnDraw(wxDC*);
    virtual bool OnClose(bool deleteWindow = true);
    virtual void OnUpdate(wxView* sender, wxObject* hint = NULL);

    ImageDocument* GetDocument();

private:
    ImageCanvas* m_canvas;

    DECLARE_DYNAMIC_CLASS(ImageView)
};

// ----------------------------------------------------------------------------
// ImageCanvas implementation
// ----------------------------------------------------------------------------

// Define a constructor for my canvas
ImageCanvas::ImageCanvas(wxView* view) : wxScrolledWindow(view->GetFrame()), m_view(view)
{
    SetScrollRate( 10, 10 );
}

// Define the repainting behaviour
void ImageCanvas::OnDraw(wxDC& dc)
{
    if ( m_view )
    {
        m_view->OnDraw(&dc);
    }
}

// ----------------------------------------------------------------------------
// ImageView implementation
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(ImageView, wxView)

ImageDocument* ImageView::GetDocument()
{
    return wxStaticCast(wxView::GetDocument(), ImageDocument);
}

bool ImageView::OnCreate(wxDocument* doc, long flags)
{
    if ( !wxView::OnCreate(doc, flags) )
        return false;

    wxFrame* frame = wxStaticCast(doc->GetDocumentTemplate(), ExampleDocTemplate1)->CreateViewFrame(this);
    wxASSERT(frame == GetFrame());
    m_canvas = new ImageCanvas(this);
    frame->SetIcon(wxICON(image));
    frame->Show();

    return true;
}

void ImageView::OnUpdate(wxView* sender, wxObject* hint)
{
    wxImage image = GetDocument()->GetImage();

    wxView::OnUpdate(sender, hint);
    if ( image.IsOk() )
    {
        m_canvas->SetVirtualSize(image.GetWidth(), image.GetHeight());
    }
}

void ImageView::OnDraw(wxDC* dc)
{
    wxImage image = GetDocument()->GetImage();
    if ( image.IsOk() )
    {
        dc->DrawBitmap(wxBitmap(image), 0, 0, true);
    }
}

bool ImageView::OnClose(bool deleteWindow)
{
    if ( !wxView::OnClose(deleteWindow) )
        return false;

    Activate(false);

    if ( deleteWindow )
    {
        GetFrame()->Destroy();
        SetFrame(NULL);
    }
    return true;
}

ExampleDocTemplate1::ExampleDocTemplate1(wxDocManager* docManager) : wxDocTemplate(docManager,
                      _("Image"), wxT("*.png;*.jpg"), wxT(""), wxT("png;jpg"),
                      wxT("Image Doc"), wxT("Image View"),
                      CLASSINFO(ImageDocument), CLASSINFO(ImageView))
{
}

/*static*/ wxDocTemplate* ExampleDocTemplate1::Create(wxDocManager* docManager)
{
   return new ExampleDocTemplate1(docManager);
}

wxFrame* ExampleDocTemplate1::CreateViewFrame(wxView* view)
{
    wxDocMDIChildFrame* subframe = new wxDocMDIChildFrame();

    if (subframe->Create(view->GetDocument(), view, wxStaticCast(wxTheApp->GetTopWindow(), wxMDIParentFrame), wxID_ANY, wxEmptyString))
    {
        wxMenuBar* menubar = new wxMenuBar();
        wxMenu* menu;

        menu = new wxMenu();
        menu->Append(wxID_NEW);
        menu->Append(wxID_OPEN);
        menu->Append(wxID_CLOSE, wxGetStockLabel(wxID_CLOSE) + wxT("\t") + _("Ctrl+W"));
        menu->AppendSeparator();
        menu->Append(ID_STE_PROPERTIES, wxGetStockLabelEx(wxID_PROPERTIES) + wxT("\t") + _("Alt+Enter"));
        menu->AppendSeparator();
        menu->Append(wxID_PRINT);
        menu->Append(wxID_PRINT_SETUP, _("Print Set&up..."));
        menu->Append(wxID_PREVIEW, wxGetStockLabelEx(wxID_PREVIEW) + wxT("\t") + _("Ctrl+Shift+P"));
        menu->AppendSeparator();
        menu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT) + wxT("\t") + _("Ctrl+Q"));
        menubar->Append(menu, wxGetStockLabel(wxID_FILE));

        menu = new wxMenu();
        menu->Append(ID_STE_SHOW_FULLSCREEN, wxString(_("&Fullscreen")) + wxT("\t") + _("F11"), wxEmptyString, wxITEM_CHECK);
        menubar->Append(menu, _("&View"));

        menu = new wxMenu();
        menu->Append(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT) + wxT("\t") + _("Shift+F1"));
        menubar->Append(menu, wxGetStockLabel(wxID_HELP));

        subframe->SetMenuBar(menubar);
    }
    else
    {
        wxDELETE(subframe);
    }
    return subframe;
}

class DocManager : public wxDocManager
{
    DECLARE_CLASS(DocManager)
public:
    DocManager();

    virtual wxDocument* CreateDocument(const wxString& path, long flags);

    wxDocument* CreateNewDefaultDocument();
protected:
    void PositionSubFrame(wxMDIChildFrame*);
};

//wxFileHistory*    m_fileHistory

IMPLEMENT_CLASS(DocManager, wxDocManager)

DocManager::DocManager() : wxDocManager()
{
}

wxDocument* DocManager::CreateDocument(const wxString& path, long flags)
{
    wxDocument* doc = wxDocManager::CreateDocument(path, flags);

    if (doc)
    {
        PositionSubFrame(wxStaticCast(doc->GetFirstView()->GetFrame(), wxMDIChildFrame));
    }
    return doc;
}

wxDocument* DocManager::CreateNewDefaultDocument()
{
    wxDocTemplate* docTemplate = EditorDocTemplate::GetInstance();
    wxDocument* doc = docTemplate->CreateDocument(wxEmptyString, wxDOC_NEW);

    if (doc->OnNewDocument())
    {
        PositionSubFrame(wxStaticCast(doc->GetFirstView()->GetFrame(), wxMDIChildFrame));
    }
    else
    {
        doc->GetDocumentManager()->CloseDocument(doc);
    }
    return doc;
}

void DocManager::PositionSubFrame(wxMDIChildFrame* subframe)
{
#if (wxVERSION_NUMBER >= 2900)
    wxMDIParentFrame* frame = subframe->GetMDIParent();
#else
    wxMDIParentFrame* frame = wxStaticCast(subframe->GetParent(), wxMDIParentFrame);
#endif

    subframe->Lower(); // -> last in z order instead of first
    frame->Tile(wxVERTICAL);
}

wxDocManager* App::CreateDocManager()
{
    wxDocManager* docManager = new DocManager();

    EditorDocTemplate::Create(docManager);
    ExampleDocTemplate1::Create(docManager);

    return docManager;
}

wxDocument* App::CreateNewDefaultDocument()
{
    return wxStaticCast(wxDocManager::GetDocumentManager(), DocManager)->CreateNewDefaultDocument();
}
