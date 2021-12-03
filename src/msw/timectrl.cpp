/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/timectrl.cpp
// Purpose:     wxTimePickerCtrl implementation
// Author:      Vadim Zeitlin
// Created:     2005-01-09
// Copyright:   (c) 2005 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_TIMEPICKCTRL

#include "wx/msw/wrapcctl.h"

#include "wx/timectrl.h"
#include "wx/dateevt.h"

import WX.WinDef;

// ============================================================================
// wxTimePickerCtrl implementation
// ============================================================================

WXDWORD wxTimePickerCtrl::MSWGetStyle(unsigned int style, WXDWORD *exstyle) const
{
    WXDWORD styleMSW = wxTimePickerCtrlBase::MSWGetStyle(style, exstyle);

    styleMSW |= DTS_TIMEFORMAT;

    return styleMSW;
}

#if wxUSE_INTL

wxLocaleInfo wxTimePickerCtrl::MSWGetFormat() const
{
    return wxLOCALE_TIME_FMT;
}

#endif // wxUSE_INTL

bool wxTimePickerCtrl::MSWOnDateTimeChange(const NMDATETIMECHANGE& dtch)
{
    m_date.SetFromMSWSysTime(dtch.st);

    wxDateEvent event(this, m_date, wxEVT_TIME_CHANGED);
    return HandleWindowEvent(event);
}
#endif // wxUSE_TIMEPICKCTRL
