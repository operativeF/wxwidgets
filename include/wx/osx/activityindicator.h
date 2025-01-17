///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/activityindicator.h
// Purpose:     Declaration of wxActivityIndicator for wxOSX (Cocoa only).
// Author:      Vadim Zeitlin
// Created:     2015-03-08
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_ACTIVITYINDICATOR_H_
#define _WX_OSX_ACTIVITYINDICATOR_H_

// ----------------------------------------------------------------------------
// wxActivityIndicator: implementation using GtkSpinner.
// ----------------------------------------------------------------------------

class wxActivityIndicator : public wxActivityIndicatorBase
{
public:
    wxActivityIndicator()
    {
        Init();
    }

    explicit
    wxActivityIndicator(wxWindow* parent,
                        wxWindowID winid = wxID_ANY,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = 0,
                        const wxString& name = wxActivityIndicatorNameStr)
    {
        Init();

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

private:
    // Common part of all ctors.
    void Init() { m_isRunning = false; }

    bool m_isRunning;

    wxDECLARE_DYNAMIC_CLASS(wxActivityIndicator);
    wxActivityIndicator(const wxActivityIndicator&) = delete;
	wxActivityIndicator& operator=(const wxActivityIndicator&) = delete;
};

#endif // _WX_OSX_ACTIVITYINDICATOR_H_
