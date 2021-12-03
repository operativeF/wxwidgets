///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/datectrl.h
// Purpose:     wxDatePickerCtrl for Windows
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2005-01-09
// Copyright:   (c) 2005 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_DATECTRL_H_
#define _WX_MSW_DATECTRL_H_

import WX.WinDef;

import <string>;

class wxDatePickerCtrl : public wxDatePickerCtrlBase
{
public:
    wxDatePickerCtrl() = default;

    wxDatePickerCtrl(wxWindow *parent,
                     wxWindowID id,
                     const wxDateTime& dt = wxDefaultDateTime,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxDP_DEFAULT | wxDP_SHOWCENTURY,
                     const wxValidator& validator = wxDefaultValidator,
                     const std::string& name = wxDatePickerCtrlNameStr)
    {
        Create(parent, id, dt, pos, size, style, validator, name);
    }

    wxDatePickerCtrl& operator=(wxDatePickerCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& dt = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDP_DEFAULT | wxDP_SHOWCENTURY,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxDatePickerCtrlNameStr);

    // Override this one to add date-specific (and time-ignoring) checks.
    void SetValue(const wxDateTime& dt) override;
    wxDateTime GetValue() const override;

    // Implement the base class pure virtuals.
    void SetRange(const wxDateTime& dt1, const wxDateTime& dt2) override;
    bool GetRange(wxDateTime *dt1, wxDateTime *dt2) const override;

    // Override MSW-specific functions used during control creation.
    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

protected:
#if wxUSE_INTL
    wxLocaleInfo MSWGetFormat() const override;
#endif // wxUSE_INTL
    bool MSWAllowsNone() const override { return HasFlag(wxDP_ALLOWNONE); }
    bool MSWOnDateTimeChange(const tagNMDATETIMECHANGE& dtch) override;

private:
    wxDateTime MSWGetControlValue() const;
};

#endif // _WX_MSW_DATECTRL_H_
