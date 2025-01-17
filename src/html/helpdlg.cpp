/////////////////////////////////////////////////////////////////////////////
// Name:        src/html/helpdlg.cpp
// Purpose:     wxHtmlHelpDialog
// Notes:       Based on htmlhelp.cpp, implementing a monolithic
//              HTML Help controller class,  by Vaclav Slavik
// Author:      Harm van der Heijden, Vaclav Slavik and Julian Smart
// Copyright:   (c) Harm van der Heijden, Vaclav Slavik and Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_WXHTML_HELP

#include "wx/intl.h"
#include "wx/log.h"

#include "wx/bmpbuttn.h"
#include "wx/statbox.h"
#include "wx/radiobox.h"
#include "wx/menu.h"
#include "wx/msgdlg.h"

#include "wx/html/htmlwin.h"
#include "wx/html/helpdlg.h"
#include "wx/html/helpctrl.h"
#include "wx/artprov.h"

import WX.Core.Sizer;

wxIMPLEMENT_DYNAMIC_CLASS(wxHtmlHelpDialog, wxDialog);

wxBEGIN_EVENT_TABLE(wxHtmlHelpDialog, wxDialog)
    EVT_CLOSE(wxHtmlHelpDialog::OnCloseWindow)
wxEND_EVENT_TABLE()

wxHtmlHelpDialog::wxHtmlHelpDialog(wxWindow* parent, wxWindowID id, const wxString& title,
                                   unsigned int style, wxHtmlHelpData* data)
{
    Init(data);
    Create(parent, id, title, style);
}

void wxHtmlHelpDialog::Init(wxHtmlHelpData* data)
{
    // Simply pass the pointer on to the help window
    m_Data = data;
    m_HtmlHelpWin = nullptr;
    m_helpController = nullptr;
}

// Create: builds the GUI components.
bool wxHtmlHelpDialog::Create(wxWindow* parent, wxWindowID id,
                             [[maybe_unused]] const wxString& title, unsigned int style)
{
    m_HtmlHelpWin = new wxHtmlHelpWindow(m_Data);

    wxDialog::Create(parent, id, "Help",
                    wxPoint(m_HtmlHelpWin->GetCfgData().x, m_HtmlHelpWin->GetCfgData().y),
                    wxSize(m_HtmlHelpWin->GetCfgData().w, m_HtmlHelpWin->GetCfgData().h),
                    wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER, "wxHtmlHelp");
    m_HtmlHelpWin->Create(this, wxID_ANY, wxDefaultPosition, GetClientSize(),
        wxTAB_TRAVERSAL|wxNO_BORDER, style);

    m_HtmlHelpWin->GetCfgData().x = GetPosition().x;
    m_HtmlHelpWin->GetCfgData().y = GetPosition().y;

    SetIcon(wxArtProvider::GetIcon(wxART_HELP, wxART_HELP_BROWSER));

    wxWindow* item1 = this;
    wxBoxSizer* item2 = new wxBoxSizer(wxVERTICAL);
    item1->SetSizer(item2);
    item1->SetAutoLayout(true);

    wxWindow* item3 = m_HtmlHelpWin;
    item2->Add(item3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* item4 = new wxBoxSizer(wxHORIZONTAL);
    item2->Add(item4, 0, wxGROW, 5);

    item4->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* item6 = new wxButton(item1, wxID_OK, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
    item4->Add(item6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10);
#ifdef __WXMAC__
    // Add some space for the resize handle
    item4->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL, 0);
#endif

    Layout();
    Centre();

    return true;
}

void wxHtmlHelpDialog::SetTitleFormat(const wxString& format)
{
    m_TitleFormat = format;
}

void wxHtmlHelpDialog::OnCloseWindow(wxCloseEvent& evt)
{
    if (!IsIconized())
    {
        m_HtmlHelpWin->GetCfgData().w = GetSize().x;
        m_HtmlHelpWin->GetCfgData().h = GetSize().y;
        m_HtmlHelpWin->GetCfgData().x = GetPosition().x;
        m_HtmlHelpWin->GetCfgData().y = GetPosition().y;
    }

    if (m_HtmlHelpWin->GetSplitterWindow() && m_HtmlHelpWin->GetCfgData().navig_on)
        m_HtmlHelpWin->GetCfgData().sashpos = m_HtmlHelpWin->GetSplitterWindow()->GetSashPosition();

    if (m_helpController)
    {
        m_helpController->OnCloseFrame(evt);
    }

    evt.Skip();
}

#endif // wxUSE_WXHTML_HELP
