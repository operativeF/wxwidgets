///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/timectrl.h
// Purpose:     wxTimePickerCtrl for Windows.
// Author:      Vadim Zeitlin
// Created:     2011-09-22
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_TIMECTRL_H_
#define _WX_MSW_TIMECTRL_H_

import Utils.Geometry;
import WX.WinDef;

import <string>;

// ----------------------------------------------------------------------------
// wxTimePickerCtrl
// ----------------------------------------------------------------------------

class wxTimePickerCtrl : public wxTimePickerCtrlBase
{
public:
    // ctors
    wxTimePickerCtrl() = default;

    wxTimePickerCtrl(wxWindow *parent,
                     wxWindowID id,
                     const wxDateTime& dt = wxDefaultDateTime,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxTP_DEFAULT,
                     const wxValidator& validator = wxDefaultValidator,
                     std::string_view name = wxTimePickerCtrlNameStr)
    {
        Create(parent, id, dt, pos, size, style, validator, name);
    }

    wxTimePickerCtrl& operator=(wxTimePickerCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& dt = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxTP_DEFAULT,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxTimePickerCtrlNameStr)
    {
        return MSWCreateDateTimePicker(parent, id, dt,
                                       pos, size, style,
                                       validator, name);
    }

    // Override MSW-specific functions used during control creation.
    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

protected:
#if wxUSE_INTL
    wxLocaleInfo MSWGetFormat() const override;
#endif // wxUSE_INTL
    bool MSWAllowsNone() const override { return false; }
    bool MSWOnDateTimeChange(const tagNMDATETIMECHANGE& dtch) override;
};

#endif // _WX_MSW_TIMECTRL_H_
