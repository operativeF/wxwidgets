/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/gauge.h
// Purpose:     wxGauge implementation for MSW
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_GAUGE_H_
#define _WX_MSW_GAUGE_H_

#if wxUSE_GAUGE

#include <string>

// Group box
class wxGauge : public wxGaugeBase
{
public:
    wxGauge() = default;

    wxGauge(wxWindow *parent,
            wxWindowID id,
            int range,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxGA_HORIZONTAL,
            const wxValidator& validator = wxDefaultValidator,
            const std::string& name = wxGaugeNameStr)
    {
        Create(parent, id, range, pos, size, style, validator, name);
    }

    wxGauge& operator=(wxGauge&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                int range,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxGA_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxGaugeNameStr);

    // set gauge range/value
    void SetRange(int range) override;
    void SetValue(int pos) override;

    // overridden base class virtuals
    bool SetForegroundColour(const wxColour& col) override;
    bool SetBackgroundColour(const wxColour& col) override;

    void Pulse() override;

    DWORD MSWGetStyle(unsigned int style, DWORD *exstyle) const override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

protected:
    wxSize DoGetBestSize() const override;

private:
    // returns true if the control is currently in indeterminate (a.k.a.
    // "marquee") mode
    bool IsInIndeterminateMode() const;

    // switch to/from indeterminate mode
    void SetIndeterminateMode();
    void SetDeterminateMode();

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // wxUSE_GAUGE

#endif // _WX_MSW_GAUGE_H_
