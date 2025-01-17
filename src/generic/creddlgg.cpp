///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/creddlgg.h
// Purpose:     wxGenericCredentialEntryDialog implementation
// Author:      Tobias Taschner
// Created:     2018-10-23
// Copyright:   (c) 2018 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_CREDENTIALDLG

#include "wx/dialog.h"
#include "wx/button.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/intl.h"

#include "wx/generic/creddlgg.h"

import WX.Core.Sizer;

wxGenericCredentialEntryDialog::wxGenericCredentialEntryDialog(
    wxWindow* parent, const std::string& message, const std::string& title,
    const wxWebCredentials& cred) :
    wxDialog(parent, wxID_ANY, title)
{
    Init(message, cred);
}

bool wxGenericCredentialEntryDialog::Create(wxWindow* parent,
    const std::string& message, const std::string& title,
    const wxWebCredentials& cred)
{
    if ( !wxDialog::Create(parent, wxID_ANY, title) )
        return false;

    Init(message, cred);
    return true;
}

void wxGenericCredentialEntryDialog::Init(const std::string& message,
    const wxWebCredentials& cred)
{
    wxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    topsizer->Add(CreateTextSizer(message), wxSizerFlags().Border());

    topsizer->Add(new wxStaticText(this, wxID_ANY, _("Username:")),
        wxSizerFlags().HorzBorder());
    m_userTextCtrl = new wxTextCtrl(this, wxID_ANY, cred.GetUser().ToStdString(),
                                    wxDefaultPosition,
                                    wxSize(FromDIP(300), wxDefaultCoord));
    topsizer->Add(m_userTextCtrl, wxSizerFlags().Expand().Border());

    topsizer->Add(new wxStaticText(this, wxID_ANY, _("Password:")),
        wxSizerFlags().HorzBorder());
    m_passwordTextCtrl = new wxTextCtrl(this, wxID_ANY,
                                        wxSecretString(cred.GetPassword()).ToStdString(),
                                        wxDefaultPosition, wxDefaultSize,
                                        wxTE_PASSWORD);
    topsizer->Add(m_passwordTextCtrl, wxSizerFlags().Expand().Border());

    topsizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Expand().Border());
    SetSizerAndFit(topsizer);

    m_userTextCtrl->SetFocus();
}

wxWebCredentials wxGenericCredentialEntryDialog::GetCredentials() const
{
    return {m_userTextCtrl->GetValue(), wxSecretValue(m_passwordTextCtrl->GetValue())};
}

#endif // wxUSE_CREDENTIALDLG
