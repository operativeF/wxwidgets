///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/progdlgg.h
// Purpose:     wxGenericProgressDialog class
// Author:      Karsten Ballueder
// Modified by: Francesco Montorsi
// Created:     09.05.1999
// Copyright:   (c) Karsten Ballueder
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __PROGDLGH_G__
#define __PROGDLGH_G__

#include "wx/dialog.h"
#include "wx/weakref.h"

#include <limits>

class WXDLLIMPEXP_FWD_CORE wxButton;
class WXDLLIMPEXP_FWD_CORE wxEventLoop;
class WXDLLIMPEXP_FWD_CORE wxGauge;
class WXDLLIMPEXP_FWD_CORE wxStaticText;
class WXDLLIMPEXP_FWD_CORE wxWindowDisabler;

/*
    Progress dialog which shows a moving progress bar.
    Taken from the Mahogany project.
*/
class wxGenericProgressDialog : public wxDialog
{
public:
    wxGenericProgressDialog();
    wxGenericProgressDialog(const std::string& title, const std::string& message,
                            int maximum = 100,
                            wxWindow *parent = nullptr,
                            unsigned int style = wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    wxGenericProgressDialog(const wxGenericProgressDialog&) = delete;
    wxGenericProgressDialog& operator=(const wxGenericProgressDialog&) = delete;

    ~wxGenericProgressDialog();

    bool Create(const std::string& title,
                const std::string& message,
                int maximum = 100,
                wxWindow *parent = nullptr,
                unsigned int style = wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    virtual bool Update(int value, const std::string& newmsg = {}, bool *skip = nullptr);
    virtual bool Pulse(const std::string& newmsg = {}, bool *skip = nullptr);

    virtual void Resume();

    virtual int GetValue() const;
    virtual int GetRange() const;
    virtual std::string wxGetMessage() const;

    virtual void SetRange(int maximum);

    // Return whether "Cancel" or "Skip" button was pressed, always return
    // false if the corresponding button is not shown.
    virtual bool WasCancelled() const;
    virtual bool WasSkipped() const;

    // Must provide overload to avoid hiding it (and warnings about it)
    void Update() override { wxDialog::Update(); }

    bool Show( bool show = true ) override;

    // This enum is an implementation detail and should not be used
    // by user code.
    enum class State
    {
        Uncancelable,   // dialog can't be canceled
        Canceled,       // can be cancelled and, in fact, was
        Continue,       // can be cancelled but wasn't
        Finished,            // finished, waiting to be removed from screen
        Dismissed            // was closed by user after finishing
    };

protected:
    // Update just the m_maximum field, this is used by public SetRange() but,
    // unlike it, doesn't update the controls state. This makes it useful for
    // both this class and its derived classes that don't use m_gauge to
    // display progress.
    void SetMaximum(int maximum);

    // Return the labels to use for showing the elapsed/estimated/remaining
    // times respectively.
    static std::string GetElapsedLabel() { return wxGetTranslation("Elapsed time:"); }
    static std::string GetEstimatedLabel() { return wxGetTranslation("Estimated time:"); }
    static std::string GetRemainingLabel() { return wxGetTranslation("Remaining time:"); }


    // Similar to wxWindow::HasFlag() but tests for a presence of a wxPD_XXX
    // flag in our (separate) flags instead of using m_windowStyle.
    bool HasPDFlag(int flag) const { return (m_pdStyle & flag) != 0; }

    // Return the progress dialog style. Prefer to use HasPDFlag() if possible.
    unsigned int GetPDStyle() const { return m_pdStyle; }
    void SetPDStyle(unsigned int pdStyle) { m_pdStyle = pdStyle; }

    // Updates estimated times from a given progress bar value and stores the
    // results in provided arguments.
    void UpdateTimeEstimates(int value,
                             unsigned long &elapsedTime,
                             unsigned long &estimatedTime,
                             unsigned long &remainingTime);

    // Converts seconds to HH:mm:ss format.
    static std::string GetFormattedTime(unsigned long timeInSec);

    // Create a new event loop if there is no currently running one.
    void EnsureActiveEventLoopExists();

    // callback for optional abort button
    void OnCancel(wxCommandEvent&);

    // callback for optional skip button
    void OnSkip(wxCommandEvent&);

    // callback to disable "hard" window closing
    void OnClose(wxCloseEvent&);

    // called to disable the other windows while this dialog is shown
    void DisableOtherWindows();

    // must be called to re-enable the other windows temporarily disabled while
    // the dialog was shown
    void ReenableOtherWindows();

    // Store the parent window as wxWindow::m_parent and also set the top level
    // parent reference we store in this class itself.
    void SetTopParent(wxWindow* parent);

    // return the top level parent window of this dialog (may be NULL)
    wxWindow *GetTopParent() const { return m_parentTop; }

private:
    // Reference to the parent top level window, automatically becomes NULL if
    // it it is destroyed and could be always NULL if it's not given at all.
    wxWindowRef m_parentTop{nullptr};

    // the widget displaying current status (may be NULL)
    wxGauge *m_gauge{nullptr};
    // the message displayed
    wxStaticText *m_msg{nullptr};
    // displayed elapsed, estimated, remaining time
    wxStaticText *m_elapsed{nullptr};
    wxStaticText *m_estimated{nullptr};
    wxStaticText *m_remaining{nullptr};

    // the abort and skip buttons (or NULL if none)
    wxButton *m_btnAbort{nullptr};
    wxButton *m_btnSkip{nullptr};

    // for wxPD_APP_MODAL case
    wxWindowDisabler *m_winDisabler{nullptr};

    // Temporary event loop created by the dialog itself if there is no
    // currently active loop when it is created.
    wxEventLoop *m_tempEventLoop{nullptr};

    // saves the time when elapsed time was updated so there is only one
    // update per second
    unsigned long m_last_timeupdate{0};

    // tells how often a change of the estimated time has to be confirmed
    // before it is actually displayed - this reduces the frequency of updates
    // of estimated and remaining time
    int m_delay{3};

    // counts the confirmations
    int m_ctdelay{0};
    unsigned long m_display_estimated{0};

    // Progress dialog styles: this is not the same as m_windowStyle because
    // wxPD_XXX constants clash with the existing TLW styles so to be sure we
    // don't have any conflicts we just use a separate variable for storing
    // them.
    unsigned int m_pdStyle{};

protected:
    // the maximum value
    int m_maximum{0};

#if defined(__WXMSW__)
    // the factor we use to always keep the value in 16 bit range as the native
    // control only supports ranges from 0 to 65,535
    size_t m_factor;
#endif // __WXMSW__

    // time when the dialog was created
    unsigned long m_timeStart;
    // time when the dialog was closed or cancelled
    // FIXME: Max
    unsigned long m_timeStop{std::numeric_limits<unsigned long>::max()};
    // time between the moment the dialog was closed/cancelled and resume
    unsigned long m_break{0};

    // continue processing or not (return value for Update())
    State m_state{State::Uncancelable};

private:
    // skip some portion
    bool m_skip{false};

    // update the label to show the given time (in seconds)
    static void SetTimeLabel(unsigned long val, wxStaticText *label);    

    // create the label with given text and another one to show the time nearby
    // as the next windows in the sizer, returns the created control
    wxStaticText *CreateLabel(const std::string& text, wxSizer *sizer);

    // updates the label message
    void UpdateMessage(const std::string& newmsg);

    // common part of Update() and Pulse(), returns true if not cancelled
    bool DoBeforeUpdate(bool *skip);

    // common part of Update() and Pulse()
    void DoAfterUpdate();

    // shortcuts for enabling buttons
    void EnableClose();
    void EnableSkip(bool enable = true);
    void EnableAbort(bool enable = true);
    void DisableSkip() { EnableSkip(false); }
    void DisableAbort() { EnableAbort(false); }

    wxDECLARE_EVENT_TABLE();
};

#endif // __PROGDLGH_G__
