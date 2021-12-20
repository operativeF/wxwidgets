/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/textdlgg.cpp
// Purpose:     wxTextEntryDialog
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_TEXTDLG

#include "wx/generic/textdlgg.h"

#include "wx/utils.h"
#include "wx/dialog.h"
#include "wx/button.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/intl.h"

#if wxUSE_STATLINE
    #include "wx/statline.h"
#endif

import WX.Core.Sizer;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

constexpr int wxID_TEXT = 3000;

// ----------------------------------------------------------------------------
// wxTextEntryDialog
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxTextEntryDialog, wxDialog)
    EVT_BUTTON(wxID_OK, wxTextEntryDialog::OnOK)
wxEND_EVENT_TABLE()

bool wxTextEntryDialog::Create(wxWindow *parent,
                                     std::string_view message,
                                     std::string_view caption,
                                     std::string_view value,
                                     unsigned int style,
                                     const wxPoint& pos)
{
    // Do not pass style to GetParentForModalDialog() as wxDIALOG_NO_PARENT
    // has the same numeric value as wxTE_MULTILINE and so attempting to create
    // a dialog for editing multiline text would also prevent it from having a
    // parent which is undesirable. As it is, we can't create a text entry
    // dialog without a parent which is not ideal neither but is a much less
    // important problem.
    if ( !wxDialog::Create(GetParentForModalDialog(parent, 0),
                           wxID_ANY, caption,
                           pos, wxDefaultSize,
                           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) )
    {
        return false;
    }

    m_dialogStyle = style;
    m_value = value;

    wxBeginBusyCursor();

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxSizerFlags flagsBorder2;
    flagsBorder2.DoubleBorder();

#if wxUSE_STATTEXT
    // 1) text message
    topsizer->Add(CreateTextSizer(message), flagsBorder2);
#endif

    // 2) text ctrl
    m_textctrl = new wxTextCtrl(this, wxID_TEXT, value,
                                wxDefaultPosition, wxSize(300, wxDefaultCoord),
                                style & ~wxTextEntryDialogStyle);

    topsizer->Add(m_textctrl,
                  wxSizerFlags(style & wxTE_MULTILINE ? 1 : 0).
                    Expand().
                    TripleBorder(wxLEFT | wxRIGHT));

    // 3) buttons if any
    wxSizer *buttonSizer = CreateSeparatedButtonSizer(style & (wxOK | wxCANCEL));
    if ( buttonSizer )
    {
        topsizer->Add(buttonSizer, wxSizerFlags(flagsBorder2).Expand());
    }

    SetAutoLayout( true );
    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    if ( style & wxCENTRE )
        Centre( wxBOTH );

    wxEndBusyCursor();

    return true;
}

bool wxTextEntryDialog::TransferDataToWindow()
{
    if ( m_textctrl )
    {
        m_textctrl->SetValue(m_value);
        m_textctrl->SetFocus();
    }

    return wxDialog::TransferDataToWindow();
}

bool wxTextEntryDialog::TransferDataFromWindow()
{
    if ( m_textctrl )
    {
        m_value = m_textctrl->GetValue();
    }

    return wxDialog::TransferDataFromWindow();
}

void wxTextEntryDialog::OnOK([[maybe_unused]] wxCommandEvent& event )
{
    if ( Validate() && TransferDataFromWindow() )
    {
        EndModal( wxID_OK );
    }
}

void wxTextEntryDialog::SetMaxLength(unsigned long len)
{
    if ( m_textctrl )
    {
        m_textctrl->SetMaxLength(len);
    }
}

void wxTextEntryDialog::SetValue(const std::string& val)
{
    m_value = val;

    if ( m_textctrl )
    {
        m_textctrl->SetValue(val);
    }
}

void wxTextEntryDialog::ForceUpper()
{
    if ( m_textctrl )
    {
        m_textctrl->ForceUpper();
    }
}

#if wxUSE_VALIDATORS

void wxTextEntryDialog::SetTextValidator( wxTextValidatorStyle style )
{
    SetTextValidator(wxTextValidator(style));
}

void wxTextEntryDialog::SetTextValidator( const wxTextValidator& validator )
{
    if ( m_textctrl )
    {
        m_textctrl->SetValidator( validator );
    }
}

#endif // wxUSE_VALIDATORS

// ----------------------------------------------------------------------------
// wxPasswordEntryDialog
// ----------------------------------------------------------------------------

bool wxPasswordEntryDialog::Create(wxWindow *parent,
                                   std::string_view message,
                                   std::string_view caption,
                                   std::string_view value,
                                   unsigned int style,
                                   const wxPoint& pos)
{
    // Only change from wxTextEntryDialog is the password style
    return wxTextEntryDialog::Create(parent, message, caption, value,
                                     style | wxTE_PASSWORD, pos);
}

#endif // wxUSE_TEXTDLG
