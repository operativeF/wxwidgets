/////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/dialog.h
// Purpose:     wxDialog class
// Author:      Vaclav Slavik
// Created:     2001/09/16
// Copyright:   (c) 2001 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_DIALOG_H_
#define _WX_UNIV_DIALOG_H_

class WXDLLIMPEXP_FWD_CORE wxWindowDisabler;
class WXDLLIMPEXP_FWD_CORE wxEventLoop;

// Dialog boxes
class wxDialog : public wxDialogBase
{
public:
    wxDialog() { Init(); }

    // Constructor with no modal flag - the new convention.
    wxDialog(wxWindow *parent, wxWindowID id,
             const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             unsigned int style = wxDEFAULT_DIALOG_STYLE,
             const wxString& name = wxASCII_STR(wxDialogNameStr))
    {
        Init();
        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_DIALOG_STYLE,
                const wxString& name = wxASCII_STR(wxDialogNameStr));

    ~wxDialog();

    // is the dialog in modal state right now?
    bool IsModal() const override;

    // For now, same as Show(true) but returns return code
    int ShowModal() override;

    // may be called to terminate the dialog with the given return code
    void EndModal(int retCode) override;

    // returns true if we're in a modal loop
    bool IsModalShowing() const;

    bool Show(bool show = true) override;

    // implementation only from now on
    // -------------------------------

    // event handlers
    void OnCloseWindow(wxCloseEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnApply(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

protected:
    // common part of all ctors
    void Init();

private:
    // while we are showing a modal dialog we disable the other windows using
    // this object
    wxWindowDisabler *m_windowDisabler;

    // modal dialog runs its own event loop
    wxEventLoop *m_eventLoop;

    // is modal right now?
    bool m_isShowingModal;

    wxDECLARE_DYNAMIC_CLASS(wxDialog);
    wxDECLARE_EVENT_TABLE();
};

#endif
    // _WX_UNIV_DIALOG_H_
