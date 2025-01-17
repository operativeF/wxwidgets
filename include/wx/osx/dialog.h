/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/dialog.h
// Purpose:     wxDialog class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIALOG_H_
#define _WX_DIALOG_H_

#include "wx/panel.h"

class WXDLLIMPEXP_FWD_CORE wxMacToolTip ;
class WXDLLIMPEXP_FWD_CORE wxModalEventLoop ;

// Dialog boxes
class wxDialog : public wxDialogBase
{
    wxDECLARE_DYNAMIC_CLASS(wxDialog);

public:
    wxDialog() { Init(); }

    // Constructor with no modal flag - the new convention.
    wxDialog(wxWindow *parent, wxWindowID id,
             const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_DIALOG_STYLE,
             const wxString& name = wxASCII_STR(wxDialogNameStr))
    {
        Init();
        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_DIALOG_STYLE,
                const wxString& name = wxASCII_STR(wxDialogNameStr));

    virtual ~wxDialog();

//    virtual bool Destroy();
    bool Show(bool show = true) override;

    // return true if we're showing the dialog modally
    bool IsModal() const override;

    // show the dialog modally and return the value passed to EndModal()
    int ShowModal() override;

    void ShowWindowModal() override;

    // may be called to terminate the dialog with the given return code
    void EndModal(int retCode) override;

    static bool OSXHasModalDialogsOpen();
    void OSXBeginModalDialog();
    void OSXEndModalDialog();

#if wxOSX_USE_COCOA
    bool OSXGetWorksWhenModal();
    void OSXSetWorksWhenModal(bool worksWhenModal);
#endif

    // implementation
    // --------------

    wxDialogModality GetModality() const override;

#if wxOSX_USE_COCOA
    virtual void ModalFinishedCallback(void* WXUNUSED(panel), int WXUNUSED(returnCode)) {}
#endif

protected:
    // show window modal dialog
    void DoShowWindowModal();

    // end window modal dialog.
    void EndWindowModal();

    // mac also takes command-period as cancel
    bool IsEscapeKey(const wxKeyEvent& event) override;


    wxDialogModality m_modality;

    wxModalEventLoop* m_eventLoop;

private:
    void Init();

    static std::vector<wxDialog*> s_modalStack;
#if wxOSX_USE_COCOA
    static std::vector<bool> s_modalWorksStack;
#endif
};

#endif
    // _WX_DIALOG_H_
