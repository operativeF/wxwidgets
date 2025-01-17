/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/fdrepdlg.h
// Purpose:     wxGenericFindReplaceDialog class
// Author:      Markus Greither
// Modified by:
// Created:     25/05/2001
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_FDREPDLG_H_
#define _WX_GENERIC_FDREPDLG_H_

class wxCheckBox;
class wxRadioBox;
class wxTextCtrl;

import <string>;
import <tuple>;

// ----------------------------------------------------------------------------
// wxGenericFindReplaceDialog: dialog for searching / replacing text
// ----------------------------------------------------------------------------

class wxGenericFindReplaceDialog : public wxFindReplaceDialogBase
{
public:
    wxGenericFindReplaceDialog() = default;

    wxGenericFindReplaceDialog(wxWindow *parent,
                               wxFindReplaceData *data,
                               const std::string& title,
                               int style = 0)
    {
        std::ignore = Create(parent, data, title, style);
    }

    bool Create(wxWindow *parent,
                wxFindReplaceData *data,
                const std::string& title,
                int style = 0);

protected:
    void SendEvent(const wxEventType& evtType);

    void OnFind(wxCommandEvent& event);
    void OnReplace(wxCommandEvent& event);
    void OnReplaceAll(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void OnUpdateFindUI(wxUpdateUIEvent& event);

    void OnCloseWindow(wxCloseEvent& event);

    wxCheckBox *m_chkCase{nullptr};
    wxCheckBox *m_chkWord{nullptr};

    wxRadioBox *m_radioDir{nullptr};

    wxTextCtrl *m_textFind{nullptr};
    wxTextCtrl *m_textRepl{nullptr};

private:
    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_GENERIC_FDREPDLG_H_
