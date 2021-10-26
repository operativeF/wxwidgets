///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/creddlgg.h
// Purpose:     wxGenericCredentialEntryDialog interface
// Author:      Tobias Taschner
// Created:     2018-10-23
// Copyright:   (c) 2018 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CREDDLGG_H_BASE
#define _WX_CREDDLGG_H_BASE

#include "wx/defs.h"

#if wxUSE_CREDENTIALDLG

#include "wx/dialog.h"
#include "wx/webrequest.h"

#include <string>

class WXDLLIMPEXP_CORE wxGenericCredentialEntryDialog : public wxDialog
{
public:
    wxGenericCredentialEntryDialog() = default;

    wxGenericCredentialEntryDialog(wxWindow* parent,
                                   const std::string& message,
                                   const std::string& title,
                                   const wxWebCredentials& cred = wxWebCredentials());

    wxGenericCredentialEntryDialog(const wxGenericCredentialEntryDialog&) = delete;
	wxGenericCredentialEntryDialog& operator=(const wxGenericCredentialEntryDialog&) = delete;

    bool Create(wxWindow* parent,
                const std::string& message,
                const std::string& title,
                const wxWebCredentials& cred = wxWebCredentials());

    void SetUser(const std::string& user) { m_userTextCtrl->SetValue(user); }
    void SetPassword(const std::string& password)
        { m_passwordTextCtrl->SetValue(password); }

    wxWebCredentials GetCredentials() const;

private:
    wxTextCtrl* m_userTextCtrl{nullptr};
    wxTextCtrl* m_passwordTextCtrl{nullptr};

    void Init(const std::string& message, const wxWebCredentials& cred);
};

// Add this typedef as long as the generic version is the only one available
using wxCredentialEntryDialog = wxGenericCredentialEntryDialog;

#endif // wxUSE_CREDENTIALDLG

#endif // _WX_CREDDLGG_H_BASE
