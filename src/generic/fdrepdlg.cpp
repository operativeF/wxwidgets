/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/fdrepdlg.cpp
// Purpose:     Find/Replace dialogs
// Author:      Markus Greither and Vadim Zeitlin
// Modified by:
// Created:     05/25/01
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FINDREPLDLG

#include "wx/intl.h"
#include "wx/log.h"

#include "wx/sizer.h"

#include "wx/button.h"
#include "wx/checkbox.h"
#include "wx/radiobox.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"

#include "wx/fdrepdlg.h"

import WX.Utils.Settings;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// ============================================================================
// implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(wxGenericFindReplaceDialog, wxDialog)
    EVT_BUTTON(wxID_FIND, wxGenericFindReplaceDialog::OnFind)
    EVT_BUTTON(wxID_REPLACE, wxGenericFindReplaceDialog::OnReplace)
    EVT_BUTTON(wxID_REPLACE_ALL, wxGenericFindReplaceDialog::OnReplaceAll)
    EVT_BUTTON(wxID_CANCEL, wxGenericFindReplaceDialog::OnCancel)

    EVT_UPDATE_UI(wxID_FIND, wxGenericFindReplaceDialog::OnUpdateFindUI)
    EVT_UPDATE_UI(wxID_REPLACE, wxGenericFindReplaceDialog::OnUpdateFindUI)
    EVT_UPDATE_UI(wxID_REPLACE_ALL, wxGenericFindReplaceDialog::OnUpdateFindUI)

    EVT_CLOSE(wxGenericFindReplaceDialog::OnCloseWindow)
wxEND_EVENT_TABLE()

// ----------------------------------------------------------------------------
// wxGenericFindReplaceDialog
// ----------------------------------------------------------------------------

bool wxGenericFindReplaceDialog::Create(wxWindow *parent,
                                        wxFindReplaceData *data,
                                        const std::string& title,
                                        int style)
{
    parent = GetParentForModalDialog(parent, style);

    if ( !wxDialog::Create(parent, wxID_ANY, title,
                           wxDefaultPosition, wxDefaultSize,
                           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
                           | style) )
    {
        return false;
    }

    SetData(data);

    wxCHECK_MSG( m_FindReplaceData, false,
                 "can't create dialog without data" );

    bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

    wxBoxSizer *leftsizer = new wxBoxSizer( wxVERTICAL );

    // 3 columns because there is a spacer in the middle
    wxFlexGridSizer *sizer2Col = new wxFlexGridSizer(3);
    sizer2Col->AddGrowableCol(2);

    sizer2Col->Add(new wxStaticText(this, wxID_ANY, _("Search for:"),
                                    wxDefaultPosition, wxSize(80, wxDefaultCoord)),
                                    0,
                                    wxALIGN_CENTRE_VERTICAL | wxALIGN_RIGHT);

    sizer2Col->Add(10, 0);

    m_textFind = new wxTextCtrl(this, wxID_ANY, m_FindReplaceData->GetFindString());
    sizer2Col->Add(m_textFind, 1, wxALIGN_CENTRE_VERTICAL | wxEXPAND);

    if ( style & wxFR_REPLACEDIALOG )
    {
        sizer2Col->Add(new wxStaticText(this, wxID_ANY, _("Replace with:"),
                                        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
                                        0,
                                        wxALIGN_CENTRE_VERTICAL |
                                        wxALIGN_RIGHT | wxTOP, 5);

        sizer2Col->Add(isPda ? 2 : 10, 0);

        m_textRepl = new wxTextCtrl(this, wxID_ANY,
                                    m_FindReplaceData->GetReplaceString());
        sizer2Col->Add(m_textRepl, 1,
                       wxALIGN_CENTRE_VERTICAL | wxEXPAND | wxTOP, 5);
    }

    leftsizer->Add(sizer2Col, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer *optsizer = new wxBoxSizer( isPda ? wxVERTICAL : wxHORIZONTAL );

    wxBoxSizer *chksizer = new wxBoxSizer( wxVERTICAL);

    m_chkWord = new wxCheckBox(this, wxID_ANY, _("Whole word"));
    chksizer->Add(m_chkWord, 0, wxALL, 3);

    m_chkCase = new wxCheckBox(this, wxID_ANY, _("Match case"));
    chksizer->Add(m_chkCase, 0, wxALL, 3);

    optsizer->Add(chksizer, 0, wxALL, 10);

    static const wxString searchDirections[] = {_("Up"), _("Down")};
    int majorDimension = 0;
    int rbStyle ;
    if (isPda)
        rbStyle = wxRA_SPECIFY_ROWS;
    else
        rbStyle = wxRA_SPECIFY_COLS;

    m_radioDir = new wxRadioBox(this, wxID_ANY, _("Search direction"),
                                wxDefaultPosition, wxDefaultSize,
                                WXSIZEOF(searchDirections), searchDirections,
                                majorDimension, rbStyle);

    optsizer->Add(m_radioDir, 0, wxALL, isPda ? 5 : 10);

    leftsizer->Add(optsizer);

    wxBoxSizer *bttnsizer = new wxBoxSizer(wxVERTICAL);

    wxButton* btn = new wxButton(this, wxID_FIND);
    btn->SetDefault();
    bttnsizer->Add(btn, 0, wxALL, 3);

    bttnsizer->Add(new wxButton(this, wxID_CANCEL), 0, wxALL, 3);

    if ( style & wxFR_REPLACEDIALOG )
    {
        bttnsizer->Add(new wxButton(this, wxID_REPLACE, _("&Replace")),
                                    0, wxALL, 3);

        bttnsizer->Add(new wxButton(this, wxID_REPLACE_ALL, _("Replace &all")),
                                    0, wxALL, 3);
    }

    wxBoxSizer *topsizer = new wxBoxSizer( wxHORIZONTAL );

    topsizer->Add(leftsizer, 1, wxALL, isPda ? 0 : 5);
    topsizer->Add(bttnsizer, 0, wxALL, isPda ? 0 : 5);

    auto flags = m_FindReplaceData->GetFlags();

    if ( flags & wxFindReplaceFlags::MatchCase )
        m_chkCase->SetValue(true);

    if ( flags & wxFindReplaceFlags::WholeWord )
        m_chkWord->SetValue(true);

    m_radioDir->SetSelection( flags & wxFindReplaceFlags::Down );

    if ( style & wxFR_NOMATCHCASE )
        m_chkCase->Enable(false);

    if ( style & wxFR_NOWHOLEWORD )
        m_chkWord->Enable(false);

    if ( style & wxFR_NOUPDOWN)
        m_radioDir->Enable(false);

    SetAutoLayout( true );
    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    Centre( wxBOTH );

    m_textFind->SetFocus();

    return true;
}

// ----------------------------------------------------------------------------
// send the notification event
// ----------------------------------------------------------------------------

void wxGenericFindReplaceDialog::SendEvent(const wxEventType& evtType)
{
    wxFindDialogEvent event(evtType, GetId());
    event.SetEventObject(this);
    event.SetFindString(m_textFind->GetValue());
    if ( HasFlag(wxFR_REPLACEDIALOG) )
    {
        event.SetReplaceString(m_textRepl->GetValue());
    }

    FindReplaceFlags flags{};

    if ( m_chkCase->GetValue() )
        flags |= wxFindReplaceFlags::MatchCase;

    if ( m_chkWord->GetValue() )
        flags |= wxFindReplaceFlags::WholeWord;

    if ( !m_radioDir || m_radioDir->GetSelection() == 1 )
    {
        flags |= wxFindReplaceFlags::Down;
    }

    event.SetFlags(flags);

    wxFindReplaceDialogBase::Send(event);
}

// ----------------------------------------------------------------------------
// event handlers
// ----------------------------------------------------------------------------

void wxGenericFindReplaceDialog::OnFind([[maybe_unused]] wxCommandEvent& event)
{
    SendEvent(wxEVT_FIND_NEXT);
}

void wxGenericFindReplaceDialog::OnReplace([[maybe_unused]] wxCommandEvent& event)
{
    SendEvent(wxEVT_FIND_REPLACE);
}

void wxGenericFindReplaceDialog::OnReplaceAll([[maybe_unused]] wxCommandEvent& event)
{
    SendEvent(wxEVT_FIND_REPLACE_ALL);
}

void wxGenericFindReplaceDialog::OnCancel([[maybe_unused]] wxCommandEvent& event)
{
    SendEvent(wxEVT_FIND_CLOSE);

    Show(false);
}

void wxGenericFindReplaceDialog::OnUpdateFindUI(wxUpdateUIEvent &event)
{
    // we can't search for empty strings
    event.Enable( !m_textFind->GetValue().empty() );
}

void wxGenericFindReplaceDialog::OnCloseWindow(wxCloseEvent &)
{
    SendEvent(wxEVT_FIND_CLOSE);
}

#endif // wxUSE_FINDREPLDLG
