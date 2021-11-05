/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/numdlgg.h
// Purpose:     wxNumberEntryDialog class
// Author:      John Labenski
// Modified by:
// Created:     07.02.04 (extracted from textdlgg.cpp)
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __NUMDLGH_G__
#define __NUMDLGH_G__

#include "wx/defs.h"

#if wxUSE_NUMBERDLG

#include "wx/dialog.h"

#include <string>

#if wxUSE_SPINCTRL
    class wxSpinCtrl;
#else
    class wxTextCtrl;
#endif // wxUSE_SPINCTRL

// ----------------------------------------------------------------------------
// wxNumberEntryDialog: a dialog with spin control, [ok] and [cancel] buttons
// ----------------------------------------------------------------------------

class wxNumberEntryDialog : public wxDialog
{
public:
    wxNumberEntryDialog() = default;

    wxNumberEntryDialog(wxWindow *parent,
                        const std::string& message,
                        const std::string& prompt,
                        const std::string& caption,
                        long value, long min, long max,
                        const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, prompt, caption, value, min, max, pos);
    }

    wxNumberEntryDialog(const wxNumberEntryDialog&) = delete;
    wxNumberEntryDialog& operator=(const wxNumberEntryDialog&) = delete;

    bool Create(wxWindow *parent,
                const std::string& message,
                const std::string& prompt,
                const std::string& caption,
                long value, long min, long max,
                const wxPoint& pos = wxDefaultPosition);

    long GetValue() const { return m_value; }

    // implementation only
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

protected:

#if wxUSE_SPINCTRL
    wxSpinCtrl *m_spinctrl;
#else
    wxTextCtrl *m_spinctrl;
#endif // wxUSE_SPINCTRL

    long m_value{0};
    long m_min{0};
    long m_max{0};

private:
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxNumberEntryDialog);
};

// ----------------------------------------------------------------------------
// function to get a number from user
// ----------------------------------------------------------------------------

long
    wxGetNumberFromUser(const std::string& message,
                        const std::string& prompt,
                        const std::string& caption,
                        long value = 0,
                        long min = 0,
                        long max = 100,
                        wxWindow *parent = nullptr,
                        const wxPoint& pos = wxDefaultPosition);

#endif // wxUSE_NUMBERDLG

#endif // __NUMDLGH_G__
