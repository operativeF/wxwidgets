///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/datetimectrl.h
// Purpose:     wxDateTimePickerCtrl for Windows.
// Author:      Vadim Zeitlin
// Created:     2011-09-22 (extracted from wx/msw/datectrl.h).
// Copyright:   (c) 2005-2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_DATETIMECTRL_H_
#define _WX_MSW_DATETIMECTRL_H_

#include "wx/intl.h"

import WX.Cfg.Flags;

import <string>;

// Forward declare a struct from Platform SDK.
struct tagNMDATETIMECHANGE;

// ----------------------------------------------------------------------------
// wxDateTimePickerCtrl
// ----------------------------------------------------------------------------

class wxDateTimePickerCtrl : public wxDateTimePickerCtrlBase
{
public:
    // set/get the date
    void SetValue(const wxDateTime& dt) override;
    wxDateTime GetValue() const override;

    void SetNullText(const std::string& text) override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;

protected:
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }
    wxSize DoGetBestSize() const override;

    // Helper for the derived classes Create(): creates a native control with
    // the specified attributes.
    bool MSWCreateDateTimePicker(wxWindow *parent,
                                 wxWindowID id,
                                 const wxDateTime& dt,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 unsigned int style,
                                 const wxValidator& validator,
                                 std::string_view name);

#if wxUSE_INTL
    // Override to return the date/time format used by this control.
    virtual wxLocaleInfo MSWGetFormat() const = 0;
#endif // wxUSE_INTL

    // Override to indicate whether we can have no date at all.
    virtual bool MSWAllowsNone() const = 0;

    // Override to update m_date and send the event when the control contents
    // changes, return true if the event was handled.
    virtual bool MSWOnDateTimeChange(const tagNMDATETIMECHANGE& dtch) = 0;


    // the date currently shown by the control, may be invalid
    wxDateTime m_date;

private:
    // Helper setting the appropriate format depending on the passed in state.
    void MSWUpdateFormat(bool valid);

    // Same thing, but only doing if the validity differs from the date
    // validity, i.e. avoiding useless work if nothing needs to be done.
    void MSWUpdateFormatIfNeeded(bool valid);


    // shown when there is no valid value (so only used with wxDP_ALLOWNONE),
    // always non-empty if SetNullText() was called, see the comments there
    std::string m_nullText;
};

#endif // _WX_MSW_DATETIMECTRL_H_
