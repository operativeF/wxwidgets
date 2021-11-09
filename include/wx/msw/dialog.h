/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dialog.h
// Purpose:     wxDialog class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIALOG_H_
#define _WX_DIALOG_H_

import <string>;

class wxDialogModalData;

class wxDialog : public wxDialogBase
{
public:
    wxDialog() = default;

    wxDialog(wxWindow *parent, wxWindowID id,
             const std::string& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             unsigned int style = wxDEFAULT_DIALOG_STYLE,
             const std::string& name = wxDialogNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }

    ~wxDialog();

    wxDialog& operator=(wxDialog&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_DIALOG_STYLE,
                const std::string& name = wxDialogNameStr);

    // return true if we're showing the dialog modally
    bool IsModal() const override { return m_modalData != nullptr; }

    // show the dialog modally and return the value passed to EndModal()
    int ShowModal() override;

    // may be called to terminate the dialog with the given return code
    void EndModal(int retCode) override;

    bool Show(bool show = true) override;
    void SetWindowStyleFlag(unsigned int style) override;

    // Windows callbacks
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;

private:
    // these functions deal with the gripper window shown in the corner of
    // resizable dialogs
    void CreateGripper();
    void DestroyGripper();
    void ShowGripper(bool show);
    void ResizeGripper();

    // this function is used to adjust Z-order of new children relative to the
    // gripper if we have one
    void OnWindowCreate(wxWindowCreateEvent& event);

    // gripper window for a resizable dialog, NULL if we're not resizable
    WXHWND m_hGripper{nullptr};

    // this pointer is non-NULL only while the modal event loop is running
    wxDialogModalData *m_modalData{nullptr};

    wxDECLARE_DYNAMIC_CLASS(wxDialog);
};

#endif
    // _WX_DIALOG_H_
