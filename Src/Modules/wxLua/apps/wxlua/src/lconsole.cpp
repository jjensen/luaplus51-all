/////////////////////////////////////////////////////////////////////////////
// Purpose:     A console to help debug/use wxLua
// Author:      J Winwood
// Created:     14/11/2001
// Modifications: Enhanced console window functionality
// RCS-ID:      $Id: lconsole.cpp,v 1.19 2008/12/05 05:57:21 jrl1 Exp $
// Copyright:   (c) 2001-2002 Lomtick Software. All rights reserved.
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
    #include "art/wxlua.xpm"
//#endif

#include "wx/splitter.h"

#include "wxlua/include/wxlua.h"
#include "lconsole.h"

// ----------------------------------------------------------------------------
// wxLuaConsole
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxLuaConsole, wxFrame)
    EVT_CLOSE(wxLuaConsole::OnCloseWindow)
END_EVENT_TABLE()

wxLuaConsole::wxLuaConsole(wxLuaConsoleWrapper* consoleWrapper,
                           wxWindow* parent, wxWindowID id, const wxString& title,
                           const wxPoint& pos, const wxSize& size,
                           long style, const wxString& name)
             :wxFrame(parent, id, title, pos, size, style, name),
              m_wrapper(consoleWrapper), m_exit_on_error(false)
{
    SetIcon(wxICON(LUA));

    m_splitter = new wxSplitterWindow(this, wxID_ANY,
                                      wxDefaultPosition, wxDefaultSize,
                                      wxSP_3DSASH);
    m_textCtrl = new wxTextCtrl(m_splitter, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    m_debugListBox = new wxListBox(m_splitter, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   0, NULL, wxLB_SINGLE);
    m_debugListBox->Show(false);

    // listbox is shown only when used
    m_splitter->Initialize(m_textCtrl);
}

void wxLuaConsole::OnCloseWindow(wxCloseEvent&)
{
    // Must NULL the console so nobody will try to still use it.
    // Using EVT_DESTROY in the app causes a segfault if this is deleted
    // in wxApp::OnExit() and though this is ugly, it works.
    if (m_wrapper)
        m_wrapper->SetConsole(NULL);

    Destroy();
    if (m_exit_on_error)
        wxExit();
}

void wxLuaConsole::DisplayText(const wxString& msg)
{
    m_textCtrl->AppendText(msg + wxT("\n"));
}

void wxLuaConsole::DisplayStack(const wxLuaState& wxlState)
{
    wxCHECK_RET(wxlState.Ok(), wxT("Invalid wxLuaState"));
    int       nIndex    = 0;
    lua_Debug luaDebug;
    wxString  buffer;

    m_debugListBox->Clear();
    lua_State* L = wxlState.GetLuaState();

    while (lua_getstack(L, nIndex, &luaDebug) != 0)
    {
        buffer.Empty();
        if (lua_getinfo(L, "Sln", &luaDebug))
        {
            int lineNumber = luaDebug.currentline;
            if (lineNumber == -1)
            {
                if (luaDebug.name != NULL)
                    buffer.Printf(wxT("function %s"),
                                  lua2wx(luaDebug.name).c_str());
                else
                    buffer.Printf(wxT("{global}"));
            }
            else
            {
                if (luaDebug.name != NULL)
                    buffer.Printf(wxT("function %s line %d"),
                                  lua2wx(luaDebug.name).c_str(),
                                  lineNumber);
                else
                    buffer.Printf(wxT("{global} line %d"),
                                  lineNumber);
            }

            // skip over ourselves on the stack
            if (nIndex > 0)
                m_debugListBox->Append(buffer, (void *) nIndex);
        }
        nIndex++;
    }

    // only show the listbox if it has anything in it
    if (m_debugListBox->GetCount() && !m_splitter->IsSplit())
    {
        m_splitter->SplitHorizontally( m_textCtrl, m_debugListBox, 150);
        m_splitter->SetMinimumPaneSize(50);
    }
}

// ---------------------------------------------------------------------------
// wxLuaConsoleWrapper
// ---------------------------------------------------------------------------

wxLuaConsole* wxLuaConsoleWrapper::GetConsole()
{
    wxCHECK_MSG(m_luaConsole != NULL, NULL, wxT("Member wxLuaConsole is NULL!"));
    return m_luaConsole;
}


