///////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/activityindicator.h
// Purpose:     Declaration of wxActivityIndicator for wxGTK.
// Author:      Vadim Zeitlin
// Created:     2015-03-05
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_ACTIVITYINDICATOR_H_
#define _WX_GTK_ACTIVITYINDICATOR_H_

// With GTK+ 3 we can always be certain that this control is available, so use
// the normal base class. With GTK+ 2 however, we may determine during run-time
// that we need to fall back to the generic implementation because the GTK+
// version is earlier than 2.20, so we need to inherit from the generic class.
#ifdef __WXGTK3__
    #define wxActivityIndicatorGtkBase wxActivityIndicatorBase
#else
    #include "wx/generic/activityindicator.h"

    #define wxActivityIndicatorGtkBase wxActivityIndicatorGeneric
#endif

// ----------------------------------------------------------------------------
// wxActivityIndicator: implementation using GtkSpinner.
// ----------------------------------------------------------------------------

class wxActivityIndicator : public wxActivityIndicatorGtkBase
{
public:
    wxActivityIndicator()
    {
    }

    explicit
    wxActivityIndicator(wxWindow* parent,
                        wxWindowID winid = wxID_ANY,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = 0,
                        const wxString& name = wxActivityIndicatorNameStr)
    {
        Create(parent, winid, pos, size, style, name);
    }

    bool Create(wxWindow* parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxActivityIndicatorNameStr);

    void Start() override;
    void Stop() override;
    bool IsRunning() const override;

protected:
    wxSize DoGetBestClientSize() const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxActivityIndicator);
    wxActivityIndicator(const wxActivityIndicator&) = delete;
	wxActivityIndicator& operator=(const wxActivityIndicator&) = delete;
};

#endif // _WX_GTK_ACTIVITYINDICATOR_H_
