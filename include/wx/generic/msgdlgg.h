/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/msgdlgg.h
// Purpose:     Generic wxMessageDialog
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_MSGDLGG_H_
#define _WX_GENERIC_MSGDLGG_H_

import <string_view>;

class wxSizer;

class wxGenericMessageDialog : public wxMessageDialogBase
{
public:
    wxGenericMessageDialog(wxWindow *parent,
                           std::string_view message,
                           std::string_view caption = wxMessageBoxCaptionStr,
                           unsigned int style = wxOK|wxCENTRE,
                           const wxPoint& pos = wxDefaultPosition);

    int ShowModal() override;

protected:
    // Creates a message dialog taking any options that have been set after
    // object creation into account such as custom labels.
    void DoCreateMsgdialog();

    void OnYes(wxCommandEvent& event);
    void OnNo(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    // can be overridden to provide more contents to the dialog
    virtual void AddMessageDialogCheckBox([[maybe_unused]] wxSizer *sizer) { }
    virtual void AddMessageDialogDetails([[maybe_unused]] wxSizer *sizer) { }

private:
    // Creates and returns a standard button sizer using the style of this
    // dialog and the custom labels, if any.
    //
    // May return NULL on smart phone platforms not using buttons at all.
    wxSizer *CreateMsgDlgButtonSizer();

    wxPoint m_pos;
    bool m_created{false};

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxGenericMessageDialog);
};

#endif // _WX_GENERIC_MSGDLGG_H_
