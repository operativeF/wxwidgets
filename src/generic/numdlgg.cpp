/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/numdlgg.cpp
// Purpose:     wxGetNumberFromUser implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     23.07.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_NUMBERDLG

#include "wx/utils.h"
#include "wx/dialog.h"
#include "wx/button.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/intl.h"
#include "wx/wxcrtvararg.h"

#if wxUSE_STATLINE
  #include "wx/statline.h"
#endif

#if wxUSE_SPINCTRL
#include "wx/spinctrl.h"
#endif

// this is where wxGetNumberFromUser() is declared
#include "wx/numdlg.h"

import WX.Core.Sizer;

import <string>;

#if !wxUSE_SPINCTRL
    // wxTextCtrl will do instead of wxSpinCtrl if we don't have it
    using wxSpinCtrl = wxTextCtrl;
#endif

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxNumberEntryDialog
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxNumberEntryDialog, wxDialog)
    EVT_BUTTON(wxID_OK, wxNumberEntryDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, wxNumberEntryDialog::OnCancel)
wxEND_EVENT_TABLE()

bool wxNumberEntryDialog::Create(wxWindow *parent,
                                         const std::string& message,
                                         const std::string& prompt,
                                         const std::string& caption,
                                         long value,
                                         long min,
                                         long max,
                                         const wxPoint& pos)
{
    if ( !wxDialog::Create(GetParentForModalDialog(parent, 0),
                           wxID_ANY, caption,
                           pos, wxDefaultSize) )
    {
        return false;
    }

    m_value = value;
    m_max = max;
    m_min = min;

    wxBeginBusyCursor();

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
#if wxUSE_STATTEXT
    // 1) text message
    topsizer->Add( CreateTextSizer( message ), 0, wxALL, 10 );
#endif

    // 2) prompt and text ctrl
    wxBoxSizer *inputsizer = new wxBoxSizer( wxHORIZONTAL );

#if wxUSE_STATTEXT
    // prompt if any
    if (!prompt.empty())
        inputsizer->Add( new wxStaticText( this, wxID_ANY, prompt ), 0, wxCENTER | wxDirection::wxLEFT, 10 );
#endif

    // spin ctrl
    wxString valStr;
    valStr.Printf("%ld", m_value);
#if wxUSE_SPINCTRL
    m_spinctrl = new wxSpinCtrl(this, wxID_ANY, valStr, wxDefaultPosition, wxSize( 140, wxDefaultCoord ), wxSP_ARROW_KEYS, (int)m_min, (int)m_max, (int)m_value);
#else
    m_spinctrl = new wxTextCtrl(this, wxID_ANY, valStr, wxDefaultPosition, wxSize( 140, wxDefaultCoord ));
#endif
    inputsizer->Add( m_spinctrl, 1, wxCENTER | wxDirection::wxLEFT | wxDirection::wxRIGHT, 10 );
    // add both
    topsizer->Add( inputsizer, 0, wxEXPAND | wxDirection::wxLEFT | wxDirection::wxRIGHT, 5 );

    // 3) buttons if any
    wxSizer *buttonSizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
    if ( buttonSizer )
    {
        topsizer->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder());
    }

    SetSizer( topsizer );
    SetAutoLayout( true );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    Centre( wxBOTH );

    m_spinctrl->SetSelection(-1, -1);
    m_spinctrl->SetFocus();

    wxEndBusyCursor();

    return true;
}

void wxNumberEntryDialog::OnOK([[maybe_unused]] wxCommandEvent& event)
{
#if !wxUSE_SPINCTRL
    wxString tmp = m_spinctrl->GetValue();
    if ( wxSscanf(tmp, "%ld", &m_value) != 1 )
        EndModal(wxID_CANCEL);
    else
#else
    m_value = m_spinctrl->GetValue();
#endif
    if ( m_value < m_min || m_value > m_max )
    {
        // not a number or out of range
        m_value = -1;
        EndModal(wxID_CANCEL);
    }

    EndModal(wxID_OK);
}

void wxNumberEntryDialog::OnCancel([[maybe_unused]] wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

// wxGetTextFromUser is in utilscmn.cpp

long wxGetNumberFromUser(const std::string& msg,
                         const std::string& prompt,
                         const std::string& title,
                         long value,
                         long min,
                         long max,
                         wxWindow *parent,
                         const wxPoint& pos)
{
    wxNumberEntryDialog dialog(parent, msg, prompt, title,
                               value, min, max, pos);
    if (dialog.ShowModal() == wxID_OK)
        return dialog.GetValue();

    return -1;
}

#endif // wxUSE_NUMBERDLG
