/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/msgdlg.h
// Purpose:     wxMessageDialog class. Use generic version if no
//              platform-specific implementation.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSGBOXDLG_H_
#define _WX_MSGBOXDLG_H_

class wxMessageDialog : public wxMessageDialogBase
{
public:
    wxMessageDialog(wxWindow *parent,
                    const wxString& message,
                    const wxString& caption = wxASCII_STR(wxMessageBoxCaptionStr),
                    long style = wxOK|wxCENTRE,
                    const wxPoint& pos = wxDefaultPosition);

    int ShowModal() override;

#if wxOSX_USE_COCOA
    void ShowWindowModal() override;
    void ModalFinishedCallback(void* panel, int resultCode) override;
#endif

protected:
    // not supported for message dialog
    virtual void DoSetSize(int WXUNUSED(x), int WXUNUSED(y),
                           int WXUNUSED(width), int WXUNUSED(height),
                           int WXUNUSED(sizeFlags) = wxSIZE_AUTO) override {}

#if wxOSX_USE_COCOA
    void* ConstructNSAlert();
#endif

    int m_buttonId[4];
    int m_buttonCount;

    wxDECLARE_DYNAMIC_CLASS(wxMessageDialog);
};

#endif // _WX_MSGBOXDLG_H_
