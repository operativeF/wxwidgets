/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/progdlg.h
// Purpose:     wxProgressDialog
// Author:      Rickard Westerlund
// Created:     2010-07-22
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROGDLG_H_
#define _WX_PROGDLG_H_

#include "wx/geometry/rect.h"

#include <string>

class wxProgressDialogTaskRunner;
struct wxProgressDialogSharedData;

class wxProgressDialog : public wxGenericProgressDialog
{
public:
    wxProgressDialog(const std::string& title,
                     const std::string& message,
                     int maximum = 100,
                     wxWindow *parent = nullptr,
                     unsigned int style = wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    ~wxProgressDialog();

    wxProgressDialog& operator=(wxProgressDialog&&) = delete;

    bool Update(int value, const std::string& newmsg = {}, bool *skip = nullptr) override;
    bool Pulse(const std::string& newmsg = {}, bool *skip = nullptr) override;

    void Resume() override;

    int GetValue() const override;
    std::string wxGetMessage() const override;

    void SetRange(int maximum) override;

    // Return whether "Cancel" or "Skip" button was pressed, always return
    // false if the corresponding button is not shown.
    bool WasSkipped() const override;
    bool WasCancelled() const override;

    void SetTitle(const std::string& title) override;
    std::string GetTitle() const override;

    void SetIcons(const wxIconBundle& icons) override;
    void DoMoveWindow(wxRect boundary) override;
    wxPoint DoGetPosition() const override;
    wxSize DoGetSize() const override;
    void Fit() override;

    bool Show( bool show = true ) override;

    // Must provide overload to avoid hiding it (and warnings about it)
    void Update() override { wxGenericProgressDialog::Update(); }

    WXWidget GetHandle() const override;

private:
    // Common part of Update() and Pulse().
    //
    // Returns false if the user requested cancelling the dialog.
    bool DoNativeBeforeUpdate(bool *skip);

    // Dispatch the pending events to let the windows to update, just as the
    // generic version does. This is done as part of DoNativeBeforeUpdate().
    void DispatchEvents();

    // Updates the various timing information for both determinate
    // and indeterminate modes. Requires the shared object to have
    // been entered.
    void UpdateExpandedInformation(int value);

    // Get the task dialog geometry when using the native dialog.
    wxRect GetTaskDialogRect() const;

    // Store the message and title we currently use to be able to return it
    // from Get{Message,Title}()
    std::string m_message;
    std::string m_title;

    wxProgressDialogTaskRunner *m_taskDialogRunner{nullptr};

    wxProgressDialogSharedData *m_sharedData{nullptr};

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_PROGDLG_H_
