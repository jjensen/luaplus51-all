///////////////////////////////////////////////////////////////////////////////
// Name:        steexprt.h
// Purpose:     wxSTEditorExporter
// Author:      John Labenski, mostly others see src for copyright
// Modified by:
// Created:     11/05/2002
// Copyright:   (c) John Labenski, Neil Hodgson & others see steexprt.cpp
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file steexprt.h
/// @brief wxSTEditorExporter and wxSTEditorExportDialog to save to html, pdf, rtf, tex, and xml.

#ifndef _STEEXPORT_H_
#define _STEEXPORT_H_

#include "wx/stedit/stedefs.h"

//-----------------------------------------------------------------------------
/// @class wxSTEditorExporter
/// @brief A class to export the contents of a wxSTEditor to a file or create
///        a html string representation for use in wxHtmlEasyPrinting.
///
///  Create this class on demand, there is no use keeping it around.
//-----------------------------------------------------------------------------

/// What type of file to export to.
enum STE_Export_Type
{
    STE_EXPORT_HTML,
    STE_EXPORT_HTMLCSS,
    STE_EXPORT_PDF,
    STE_EXPORT_RTF,
    STE_EXPORT_TEX,
    STE_EXPORT_XML
};

class WXDLLIMPEXP_STEDIT wxSTEditorExporter
{
public:
    wxSTEditorExporter(wxSTEditor* editor);

    // these are taken from SciTE src/Exporters.cxx
    bool SaveToRTF(const wxFileName& fileName, int start = 0, int end = -1);
    bool SaveToHTMLCSS(const wxFileName& fileName);
    bool SaveToPDF(const wxFileName& fileName);
    bool SaveToTEX(const wxFileName& fileName);
    bool SaveToXML(const wxFileName& fileName);

    /// Get a HTML representation of the text, w/ styles.
    ///  code originally from wxHatch by Chris Elliott
    wxString RenderAsHTML(int from, int to) const;
    /// Save to HTML using RenderAsHTML(), returns true if the file was written.
    bool SaveToHTML(const wxFileName& fileName);

    /// Export to the enum STE_Export_Type file_format to the given fileName.
    /// If !overwrite_prompt don't ask to overwrite.
    /// If !msg_on_error don't show an error message on failure (write error).
    bool ExportToFile(int file_format, const wxFileName& fileName,
                      bool overwrite_prompt, bool msg_on_error);

    /// Returns extensions for enum STE_Export_Type, else empty string.
    static wxString GetExtension(int file_format);
    /// Returns wildcard strings ("HTML (html,htm)|*.html;*.htm") for enum STE_Export_Type,
    /// else wxFileSelectorDefaultWildcardStr.
    static wxString GetWildcards(int file_format);

    // -----------------------------------------------------------------------
    // implementation

    /// Maps the stc style # to the appropriate ste style using the langs.
    /// Returns STE_STYLE_DEFAULT if the style isn't set.
    int SciToSTEStyle(int sci_style) const;

protected:
    wxSTEditor*      m_editor;
    wxSTEditorPrefs  m_stePrefs;
    wxSTEditorStyles m_steStyles;
    wxSTEditorLangs  m_steLangs;
};

//-----------------------------------------------------------------------------
/// @class wxSTEditorExportDialog
/// @brief Choose format and filename to export text.
///
/// This does not save the file, see wxSTEditor::ShowExportDialog().
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_STEDIT wxSTEditorExportDialog : public wxDialog
{
public:
    wxSTEditorExportDialog();
    wxSTEditorExportDialog(wxWindow* parent,
                           long style = wxDEFAULT_DIALOG_STYLE_RESIZE);

    bool Create(wxWindow* parent,
                long style = wxDEFAULT_DIALOG_STYLE_RESIZE);

    wxFileName GetFileName() const;
    void SetFileName(const wxFileName& fileName);

    STE_Export_Type  GetFileFormat() const;
    void SetFileFormat(STE_Export_Type file_format);

    // -----------------------------------------------------------------------
    // implementation

    void OnChoice(wxCommandEvent& event);
    void OnButton(wxCommandEvent& event);

    wxFileName FileNameExtChange(const wxFileName&, int file_format) const;

    wxChoice   *m_fileFormatChoice;
    wxComboBox *m_fileNameCombo;

    static wxArrayString sm_fileNames;     // remember previous settings
    static int           sm_file_format;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_ABSTRACT_CLASS(wxSTEditorExportDialog);
};

#endif  // _STEEXPORT_H_
