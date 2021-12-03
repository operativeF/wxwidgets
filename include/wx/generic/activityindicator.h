///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/activityindicator.h
// Purpose:     Declaration of wxActivityIndicatorGeneric.
// Author:      Vadim Zeitlin
// Created:     2015-03-06
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_ACTIVITYINDICATOR_H_
#define _WX_GENERIC_ACTIVITYINDICATOR_H_

#ifndef wxHAS_NATIVE_ACTIVITYINDICATOR
    // This is the only implementation we have, so call it accordingly.
    #define wxActivityIndicatorGeneric wxActivityIndicator
#endif

// ----------------------------------------------------------------------------
// wxActivityIndicatorGeneric: built-in generic implementation.
// ----------------------------------------------------------------------------

class wxActivityIndicatorGeneric : public wxActivityIndicatorBase
{
public:
    wxActivityIndicatorGeneric() = default;

    explicit
    wxActivityIndicatorGeneric(wxWindow* parent,
                               wxWindowID winid = wxID_ANY,
                               const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxDefaultSize,
                               unsigned int style = 0,
                               std::string_view name = wxActivityIndicatorNameStr)
    {
        Create(parent, winid, pos, size, style, name);
    }

    bool Create(wxWindow* parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxActivityIndicatorNameStr);

    ~wxActivityIndicatorGeneric();

    void Start() override;
    void Stop() override;
    bool IsRunning() const override;

protected:
    wxSize DoGetBestClientSize() const override;

private:
    class wxActivityIndicatorImpl *m_impl{nullptr};
};

#endif // _WX_GENERIC_ACTIVITYINDICATOR_H_
